// STL-backed CtrlCore multithreading functionality implementation

#include "CtrlMt.h"
#include <thread>
#include <atomic>
#include <mutex>

namespace Upp {

// Static member definitions
std::mutex CtrlMtGlobal::global_mtx;
std::atomic<int> CtrlMtGlobal::ui_thread_count{0};

// CtrlMtGlobal implementation
void CtrlMtGlobal::RegisterUiThread() {
    ui_thread_count.fetch_add(1);
}

void CtrlMtGlobal::UnregisterUiThread() {
    ui_thread_count.fetch_sub(1);
}

int CtrlMtGlobal::GetActiveUiThreadCount() {
    return ui_thread_count.load();
}

bool CtrlMtGlobal::IsMainThread() {
    // In a real implementation, this would check if we're on the main/UI thread
    // For now, we'll return true if there's only one UI thread registered
    return ui_thread_count.load() == 1;
}

void CtrlMtGlobal::Initialize() {
    // Initialize any global multithreading resources
    ui_thread_count.store(0);
}

void CtrlMtGlobal::Cleanup() {
    // Cleanup any global multithreading resources
    ui_thread_count.store(0);
}

// CtrlMt implementation - some utility methods that weren't fully implemented in header
void CtrlMt::ExecuteThreadPool(std::function<void()> func) {
    // Simple implementation using std::thread
    std::thread t([func = std::move(func)]() {
        try {
            func();
        } catch (...) {
            // In a real implementation, this would log the exception
        }
    });
    t.detach(); // Let thread run independently
}

// Execute a task in background with result
template<typename Func>
std::future<typename std::result_of<Func()>::type> CtrlMt::ExecuteAsync(Func&& func) {
    return std::async(std::launch::async, std::forward<Func>(func));
}

// Execute a function after a delay
void CtrlMt::ExecuteAfter(std::chrono::milliseconds delay, std::function<void()> func) {
    std::thread([this, delay, func = std::move(func)]() {
        std::this_thread::sleep_for(delay);
        try {
            Invoke(func);
        } catch (...) {
            // In a real implementation, this would log the exception
        }
    }).detach();
}

// Execute a function periodically
void CtrlMt::ExecuteEvery(std::chrono::milliseconds interval, std::function<bool()> func) {
    std::thread([this, interval, func = std::move(func)]() {
        while (true) {
            std::this_thread::sleep_for(interval);
            try {
                bool continue_execution = Invoke([f = func]() { return f(); });
                if (!continue_execution) {
                    break; // If function returns false, stop periodic execution
                }
            } catch (...) {
                // In a real implementation, this would log the exception and possibly break
                break;
            }
        }
    }).detach();
}

// Cancel all background operations
void CtrlMt::CancelBackgroundOperations() {
    // In a full implementation, this would track and cancel async operations
    // For now, we'll just notify that cancellation was requested
}

// Wait for all pending invocations to complete
void CtrlMt::WaitPendingInvokes() {
    std::unique_lock<std::mutex> lock(invoke_mutex);
    invoke_cond.wait(lock, [this] { 
        return invoke_queue.empty() && !processing_invokes.load(); 
    });
}

// Process pending UI thread invocations
void CtrlMt::ProcessInvokes() {
    std::unique_lock<std::mutex> lock(invoke_mutex);
    processing_invokes = true;
    
    while (!invoke_queue.empty()) {
        auto func = std::move(invoke_queue.front());
        invoke_queue.pop();
        lock.unlock(); // Unlock while executing to allow concurrent queuing
        
        try {
            func();
        } catch (...) {
            // In a real implementation, this would log the exception
        }
        
        lock.lock(); // Re-lock for next iteration
    }
    
    processing_invokes = false;
    invoke_cond.notify_all(); // Notify anyone waiting
}

// Additional utility methods for thread-safe operations
void CtrlMt::Sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::thread::id CtrlMt::GetCurrentThreadId() {
    return std::this_thread::get_id();
}

// Thread-safe property access implementations
template<typename T>
void CtrlMt::SetProp(std::function<void(const T&)>& setter, const T& value) {
    if (IsUiThread()) {
        setter(value);
    } else {
        InvokeAsync([setter, value]() { 
            setter(value); 
        });
    }
}

template<typename T>
T CtrlMt::GetProp(std::function<T()>& getter) const {
    if (IsUiThread()) {
        return getter();
    } else {
        return Invoke([getter]() { 
            return getter(); 
        });
    }
}

// Critical section implementation
template<typename Func>
auto CtrlMt::CriticalSection(Func&& func) -> decltype(func()) {
    std::lock_guard<std::recursive_mutex> lock(ctrl_mutex);
    return func();
}

// Additional WaitHandle methods
void CtrlMt::WaitHandle::Signal() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        signaled = true;
    }
    cv.notify_all();
}

void CtrlMt::WaitHandle::Wait() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return signaled.load(); });
}

bool CtrlMt::WaitHandle::WaitFor(std::chrono::milliseconds ms) {
    std::unique_lock<std::mutex> lock(mtx);
    return cv.wait_for(lock, ms, [this] { return signaled.load(); });
}

void CtrlMt::WaitHandle::Reset() {
    std::lock_guard<std::mutex> lock(mtx);
    signaled = false;
}

// Additional WaitEvent methods
void CtrlMt::WaitEvent::Trigger() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        triggered = true;
    }
    cv.notify_all();
}

void CtrlMt::WaitEvent::Wait() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return triggered.load(); });
}

bool CtrlMt::WaitEvent::WaitTimeout(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mtx);
    return cv.wait_for(lock, timeout, [this] { return triggered.load(); });
}

void CtrlMt::WaitEvent::Reset() {
    std::lock_guard<std::mutex> lock(mtx);
    triggered = false;
}

bool CtrlMt::WaitEvent::IsTriggered() const {
    return triggered.load();
}

// ThreadSafeValue implementations
template<typename T>
void CtrlMt::ThreadSafeValue<T>::Set(const T& val) {
    std::lock_guard<std::mutex> lock(mtx);
    value = val;
}

template<typename T>
T CtrlMt::ThreadSafeValue<T>::Get() const {
    std::lock_guard<std::mutex> lock(mtx);
    return value;
}

// MtCtrl template implementations
template<typename BaseCtrl>
template<typename Func>
auto MtCtrl<BaseCtrl>::SyncInvoke(Func&& func) -> decltype(func()) {
    return Invoke(std::forward<Func>(func));
}

template<typename BaseCtrl>
template<typename Func>
void MtCtrl<BaseCtrl>::SyncInvokeAsync(Func&& func) {
    InvokeAsync(std::forward<Func>(func));
}

template<typename BaseCtrl>
template<typename T>
void MtCtrl<BaseCtrl>::SetProp(std::function<void(const T&)>& setter, const T& value) {
    CtrlMt::SetProp(setter, value);
}

template<typename BaseCtrl>
template<typename T>
T MtCtrl<BaseCtrl>::GetProp(std::function<T()>& getter) const {
    return CtrlMt::GetProp(getter);
}

template<typename BaseCtrl>
template<typename T>
void MtCtrl<BaseCtrl>::UpdateProp(T& prop, const T& new_value, std::function<void()> on_change) {
    bool changed = false;
    
    if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
        if (prop != new_value) {
            prop = new_value;
            changed = true;
        }
    } else {
        prop = new_value;
        changed = true;
    }
    
    if (changed && on_change) {
        if (IsUiThread()) {
            on_change();
        } else {
            Invoke(on_change);
        }
    }
}

// Explicit template instantiations for common types
template void CtrlMt::SetProp<int>(std::function<void(const int&)>&, const int&);
template void CtrlMt::SetProp<std::string>(std::function<void(const std::string&)>&, const std::string&);
template void CtrlMt::SetProp<bool>(std::function<void(const bool&)>&, const bool&);
template void CtrlMt::SetProp<double>(std::function<void(const double&)>&, const double&);

template int CtrlMt::GetProp<int>(std::function<int()>&) const;
template std::string CtrlMt::GetProp<std::string>(std::function<std::string()>&) const;
template bool CtrlMt::GetProp<bool>(std::function<bool()>&) const;
template double CtrlMt::GetProp<double>(std::function<double()>&) const;

template void MtCtrl<Ctrl>::SetProp<int>(std::function<void(const int&)>&, const int&);
template void MtCtrl<Ctrl>::SetProp<std::string>(std::function<void(const std::string&)>&, const std::string&);
template void MtCtrl<Ctrl>::SetProp<bool>(std::function<void(const bool&)>&, const bool&);
template void MtCtrl<Ctrl>::SetProp<double>(std::function<void(const double&)>&, const double&);

template int MtCtrl<Ctrl>::GetProp<int>(std::function<int()>&) const;
template std::string MtCtrl<Ctrl>::GetProp<std::string>(std::function<std::string()>&) const;
template bool MtCtrl<Ctrl>::GetProp<bool>(std::function<bool()>&) const;
template double MtCtrl<Ctrl>::GetProp<double>(std::function<double()>&) const;

}