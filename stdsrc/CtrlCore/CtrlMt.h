#pragma once
#ifndef _CtrlCore_CtrlMt_h_
#define _CtrlCore_CtrlMt_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <queue>
#include <memory>

// Multi-threading support for controls
class CtrlMt {
protected:
    Ctrl& ctrl;
    mutable std::recursive_mutex ctrl_mutex;
    std::queue<std::function<void()>> invoke_queue;
    std::mutex invoke_mutex;
    std::condition_variable invoke_cond;
    std::atomic<bool> processing_invokes{false};
    std::thread::id creation_thread_id;
    
public:
    explicit CtrlMt(Ctrl& c) : ctrl(c), creation_thread_id(std::this_thread::get_id()) {}
    
    // Check if we're on the UI thread
    bool IsUiThread() const {
        return std::this_thread::get_id() == creation_thread_id;
    }
    
    // Get UI thread ID
    std::thread::id GetUiThreadId() const {
        return creation_thread_id;
    }
    
    // Invoke a function on the UI thread (blocking)
    template<typename Func>
    auto Invoke(Func&& func) -> decltype(func()) {
        if (IsUiThread()) {
            return func(); // Execute directly if already on UI thread
        }
        
        std::mutex mtx;
        std::unique_lock<std::mutex> lock(mtx);
        std::condition_variable cv;
        bool finished = false;
        std::exception_ptr exception_ptr = nullptr;
        decltype(func()) result{};
        
        // Queue the function for execution on the UI thread
        {
            std::lock_guard<std::mutex> invoke_lock(invoke_mutex);
            invoke_queue.push([&]() {
                try {
                    if constexpr (std::is_void_v<decltype(func())>) {
                        func();
                        result = {};
                    } else {
                        result = func();
                    }
                } catch (...) {
                    exception_ptr = std::current_exception();
                }
                {
                    std::lock_guard<std::mutex> result_lock(mtx);
                    finished = true;
                }
                cv.notify_one();
            });
            invoke_cond.notify_one();
        }
        
        // Wait for completion
        cv.wait(lock, [&finished] { return finished; });
        
        if (exception_ptr) {
            std::rethrow_exception(exception_ptr);
        }
        
        return result;
    }
    
    // Invoke a function on the UI thread (non-blocking)
    template<typename Func>
    void InvokeAsync(Func&& func) {
        std::lock_guard<std::mutex> lock(invoke_mutex);
        invoke_queue.push([func = std::forward<Func>(func)]() { func(); });
        invoke_cond.notify_one();
    }
    
    // Process pending UI thread invocations
    void ProcessInvokes() {
        std::lock_guard<std::mutex> lock(invoke_mutex);
        processing_invokes = true;
        
        while (!invoke_queue.empty()) {
            auto func = std::move(invoke_queue.front());
            invoke_queue.pop();
            func();
        }
        
        processing_invokes = false;
    }
    
    // Check if there are pending invocations
    bool HasPendingInvokes() const {
        std::lock_guard<std::mutex> lock(invoke_mutex);
        return !invoke_queue.empty();
    }
    
    // Wait for all pending invocations to complete
    void WaitPendingInvokes() {
        std::unique_lock<std::mutex> lock(invoke_mutex);
        invoke_cond.wait(lock, [this] { return invoke_queue.empty() && !processing_invokes.load(); });
    }
    
    // Thread-safe property access macros
    #define WITH_LOCK(obj) std::lock_guard<std::recursive_mutex> lock(obj.ctrl_mutex);
    
    // Execute critical section with lock
    template<typename Func>
    auto CriticalSection(Func&& func) -> decltype(func()) {
        WITH_LOCK(*this);
        return func();
    }
    
    // Thread-safe property setters
    template<typename T>
    void SetProp(std::function<void(const T&)>& setter, const T& value) {
        if (IsUiThread()) {
            setter(value);
        } else {
            InvokeAsync([setter, value]() { setter(value); });
        }
    }
    
    // Thread-safe property getters
    template<typename T>
    T GetProp(std::function<T()>& getter) const {
        if (IsUiThread()) {
            return getter();
        } else {
            return Invoke([getter]() { return getter(); });
        }
    }
    
    // Background task execution
    template<typename Func>
    std::future<typename std::result_of<Func()>::type> ExecuteAsync(Func&& func) {
        return std::async(std::launch::async, std::forward<Func>(func));
    }
    
    // Execute a task in a thread pool
    template<typename Func>
    void ExecuteThreadPool(Func&& func) {
        // In a real implementation, this would use a thread pool
        std::thread t(std::forward<Func>(func));
        t.detach(); // Let thread run independently
    }
    
    // Wait handle for synchronization
    class WaitHandle {
    private:
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> signaled{false};
        
    public:
        void Signal() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                signaled = true;
            }
            cv.notify_all();
        }
        
        void Wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return signaled.load(); });
        }
        
        bool WaitFor(std::chrono::milliseconds ms) {
            std::unique_lock<std::mutex> lock(mtx);
            return cv.wait_for(lock, ms, [this] { return signaled.load(); });
        }
        
        void Reset() {
            std::lock_guard<std::mutex> lock(mtx);
            signaled = false;
        }
    };
    
    // Event that can be waited on from multiple threads
    class WaitEvent {
    private:
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> triggered{false};
        
    public:
        void Trigger() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                triggered = true;
            }
            cv.notify_all();
        }
        
        void Wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return triggered.load(); });
        }
        
        bool WaitTimeout(std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mtx);
            return cv.wait_for(lock, timeout, [this] { return triggered.load(); });
        }
        
        void Reset() {
            std::lock_guard<std::mutex> lock(mtx);
            triggered = false;
        }
        
        bool IsTriggered() const {
            return triggered.load();
        }
    };
    
    // Thread-safe value container
    template<typename T>
    class ThreadSafeValue {
    private:
        mutable std::mutex mtx;
        T value;
        
    public:
        ThreadSafeValue() = default;
        ThreadSafeValue(const T& val) : value(val) {}
        
        void Set(const T& val) {
            std::lock_guard<std::mutex> lock(mtx);
            value = val;
        }
        
        T Get() const {
            std::lock_guard<std::mutex> lock(mtx);
            return value;
        }
        
        T operator*() const { return Get(); }
        T* operator->() { return &Get(); }
    };
    
    // Synchronization utilities
    static void Sleep(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    static std::thread::id GetCurrentThreadId() {
        return std::this_thread::get_id();
    }
    
    // Execute a function after a delay (from any thread)
    template<typename Func>
    void ExecuteAfter(std::chrono::milliseconds delay, Func&& func) {
        std::thread([this, delay, func = std::forward<Func>(func)]() {
            std::this_thread::sleep_for(delay);
            Invoke(func);
        }).detach();
    }
    
    // Execute a function periodically (from any thread)
    template<typename Func>
    void ExecuteEvery(std::chrono::milliseconds interval, Func&& func) {
        std::thread([this, interval, func = std::forward<Func>(func)]() {
            while (true) {
                std::this_thread::sleep_for(interval);
                if (!Invoke([f = func]() { return f(); })) {
                    break; // If function returns false, stop periodic execution
                }
            }
        }).detach();
    }
    
    // Cancel all background operations (not fully implemented in this basic version)
    void CancelBackgroundOperations() {
        // In a full implementation, this would track and cancel async operations
    }
};

// Helper class for creating controls with multi-threading support
template<typename BaseCtrl>
class MtCtrl : public BaseCtrl, public CtrlMt {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    MtCtrl() : BaseCtrl(), CtrlMt(*this) {}
    explicit MtCtrl(const Rect& r) : BaseCtrl(r), CtrlMt(*this) {}
    
    // Convenience methods for thread-safe operations
    template<typename Func>
    auto SyncInvoke(Func&& func) -> decltype(func()) {
        return Invoke(std::forward<Func>(func));
    }
    
    template<typename Func>
    void SyncInvokeAsync(Func&& func) {
        InvokeAsync(std::forward<Func>(func));
    }
    
    // Thread-safe property access
    template<typename T>
    void SetProp(std::function<void(const T&)>& setter, const T& value) {
        CtrlMt::SetProp(setter, value);
    }
    
    template<typename T>
    T GetProp(std::function<T()>& getter) const {
        return CtrlMt::GetProp(getter);
    }
    
    // Property update helper
    template<typename T>
    void UpdateProp(T& prop, const T& new_value, std::function<void()> on_change = nullptr) {
        if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
            if (prop != new_value) {
                prop = new_value;
                if (on_change) {
                    if (IsUiThread()) {
                        on_change();
                    } else {
                        Invoke(on_change);
                    }
                }
            }
        } else {
            prop = new_value;
            if (on_change) {
                if (IsUiThread()) {
                    on_change();
                } else {
                    Invoke(on_change);
                }
            }
        }
    }
};

// Global multi-threading utilities
class CtrlMtGlobal {
private:
    static std::mutex global_mtx;
    static std::atomic<int> ui_thread_count;
    
public:
    static void RegisterUiThread();
    static void UnregisterUiThread();
    static int GetActiveUiThreadCount();
    static bool IsMainThread();
    
    // Initialize multi-threading support
    static void Initialize();
    
    // Cleanup multi-threading support
    static void Cleanup();
};

// Macro for thread-safe operations
#define UI_THREAD_EXEC(ctrl, func) (ctrl).Invoke([&]() { func; })

#endif