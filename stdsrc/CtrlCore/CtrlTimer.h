#pragma once
#ifndef _CtrlCore_CtrlTimer_h_
#define _CtrlCore_CtrlTimer_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include <chrono>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>

// Timer implementation for controls
class CtrlTimer {
protected:
    Ctrl& ctrl;
    std::mutex timer_mutex;
    std::condition_variable timer_cv;
    std::atomic<bool> timer_running{false};
    std::thread timer_thread;
    std::list<std::pair<std::chrono::steady_clock::time_point, std::function<void()>>> timer_events;
    
public:
    explicit CtrlTimer(Ctrl& c) : ctrl(c) {}
    
    virtual ~CtrlTimer() {
        StopTimer();
    }
    
    // Start the timer system
    virtual bool StartTimer() {
        if (timer_running.exchange(true)) {
            return true; // Already running
        }
        
        timer_thread = std::thread([this]() {
            TimerLoop();
        });
        
        return true;
    }
    
    // Stop the timer system
    virtual bool StopTimer() {
        if (!timer_running.exchange(false)) {
            return true; // Already stopped
        }
        
        timer_cv.notify_all();
        
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
        
        return true;
    }
    
    // Set a single shot timer
    virtual void SetTimer(int milliseconds, std::function<void()> callback) {
        auto when = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
        
        {
            std::lock_guard<std::mutex> lock(timer_mutex);
            timer_events.push_back({when, callback});
        }
        
        timer_cv.notify_all();
        return *this;
    }
    
    // Set a repeating timer
    virtual void SetRepeat(int milliseconds, std::function<void()> callback) {
        // Create a repeating timer by rescheduling after each execution
        std::weak_ptr<Ctrl> weak_ctrl = ctrl.shared_from_this();
        
        auto repeating_callback = [this, milliseconds, callback, weak_ctrl]() {
            if (auto ctrl_ptr = weak_ctrl.lock()) {
                callback();
                // Reschedule the timer
                SetTimer(milliseconds, [this, milliseconds, callback, weak_ctrl]() {
                    if (auto ctrl_ptr = weak_ctrl.lock()) {
                        callback();
                        // Continue repeating...
                        SetTimer(milliseconds, [this, milliseconds, callback, weak_ctrl]() {
                            if (auto ctrl_ptr = weak_ctrl.lock()) {
                                callback();
                                // This pattern would continue in a full implementation
                            }
                        });
                    }
                });
            }
        };
        
        return SetTimer(milliseconds, repeating_callback);
    }
    
    // Cancel all timers
    virtual void CancelTimer() {
        std::lock_guard<std::mutex> lock(timer_mutex);
        timer_events.clear();
        return *this;
    }
    
    // Get time since timer system started
    virtual int GetTime() const {
        static auto start_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    }
    
    // Sleep for specified milliseconds
    static void Sleep(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    // Get current tick count (milliseconds since system start)
    static int64_t GetTickCount() {
        auto now = std::chrono::steady_clock::now();
        return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    
    // Delay execution by specified milliseconds
    virtual void Delay(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    // Schedule a function to run after a delay (non-blocking)
    template<typename Func>
    void RunAfter(std::chrono::milliseconds delay, Func&& func) {
        std::thread([this, delay, f = std::forward<Func>(func)]() {
            std::this_thread::sleep_for(delay);
            // In a real implementation, this would call the function on the UI thread
            f();
        }).detach();
    }
    
    // Schedule a function to run periodically
    template<typename Func>
    void RunEvery(std::chrono::milliseconds interval, Func&& func) {
        std::thread([this, interval, f = std::forward<Func>(func)]() {
            while (timer_running.load()) {
                std::this_thread::sleep_for(interval);
                if (timer_running.load()) {
                    // In a real implementation, this would call the function on the UI thread
                    f();
                }
            }
        }).detach();
    }
    
    // Internal timer loop
private:
    void TimerLoop() {
        while (timer_running.load()) {
            ProcessTimers();
            
            std::unique_lock<std::mutex> lock(timer_mutex);
            auto now = std::chrono::steady_clock::now();
            auto next_event = now + std::chrono::milliseconds(100); // Default wait time
            
            // Find the next event time
            for (const auto& event : timer_events) {
                if (event.first < next_event) {
                    next_event = event.first;
                }
            }
            
            // Wait for the next event or timeout
            auto wait_time = next_event - now;
            if (wait_time > std::chrono::milliseconds(0)) {
                timer_cv.wait_for(lock, wait_time);
            }
        }
    }
    
    void ProcessTimers() {
        std::lock_guard<std::mutex> lock(timer_mutex);
        auto now = std::chrono::steady_clock::now();
        
        // Execute any expired timers
        auto it = timer_events.begin();
        while (it != timer_events.end()) {
            if (it->first <= now) {
                // Execute the callback
                it->second();
                it = timer_events.erase(it);
            } else {
                ++it;
            }
        }
    }
    
public:
    // Time utilities
    static std::chrono::steady_clock::time_point Now() {
        return std::chrono::steady_clock::now();
    }
    
    template<typename Rep, typename Period>
    static void Wait(std::chrono::duration<Rep, Period> duration) {
        std::this_thread::sleep_for(duration);
    }
    
    // Measure execution time of a function
    template<typename Func>
    static auto TimeFunc(Func&& func) -> std::chrono::microseconds {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }
    
    // Animation timer - for smooth animations
    class AnimationTimer {
    private:
        CtrlTimer& parent;
        int interval;
        std::function<bool(double progress)> callback; // Returns true to continue
        std::atomic<bool> active{false};
        std::thread anim_thread;
        int duration;
        
    public:
        AnimationTimer(CtrlTimer& p) : parent(p), interval(16), duration(1000) {} // ~60 FPS default
        
        void Start(std::function<bool(double progress)> cb, int anim_duration = 1000) {
            callback = cb;
            duration = anim_duration;
            active = true;
            
            anim_thread = std::thread([this]() {
                auto start_time = std::chrono::steady_clock::now();
                auto total_time = std::chrono::milliseconds(duration);
                
                while (active.load() && 
                       std::chrono::steady_clock::now() - start_time < total_time) {
                    auto elapsed = std::chrono::steady_clock::now() - start_time;
                    double progress = (double)elapsed.count() / total_time.count();
                    
                    if (!callback(progress)) {
                        break; // Animation requested to stop
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                }
                
                // Ensure completion at 100%
                callback(1.0);
            });
        }
        
        void Stop() {
            active = false;
            if (anim_thread.joinable()) {
                anim_thread.join();
            }
        }
        
        bool IsActive() const {
            return active.load();
        }
    };
    
    // Create an animation timer for this control
    AnimationTimer CreateAnimationTimer() {
        return AnimationTimer(*this);
    }
};

// Helper class for creating controls with timer support
template<typename BaseCtrl>
class TimerCtrl : public BaseCtrl, public CtrlTimer {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    TimerCtrl() : BaseCtrl(), CtrlTimer(*this) {}
    explicit TimerCtrl(const Rect& r) : BaseCtrl(r), CtrlTimer(*this) {}
    
    // Start timer when control is created
    TimerCtrl& WithTimer() {
        StartTimer();
        return *this;
    }
    
    // Timer convenience methods
    TimerCtrl& Timeout(int ms, std::function<void()> callback) {
        SetTimer(ms, callback);
        return *this;
    }
    
    TimerCtrl& Tick(int ms, std::function<void()> callback) {
        SetRepeat(ms, callback);
        return *this;
    }
    
    TimerCtrl& Once(std::chrono::milliseconds delay, std::function<void()> callback) {
        RunAfter(delay, callback);
        return *this;
    }
    
    TimerCtrl& Every(std::chrono::milliseconds interval, std::function<void()> callback) {
        RunEvery(interval, callback);
        return *this;
    }
    
    // Animation helper
    template<typename Func>
    TimerCtrl& Animate(int duration, Func&& func) {
        AnimationTimer anim_timer(*this);
        anim_timer.Start(func, duration);
        return *this;
    }
    
    // Stop all timers when control is destroyed
    ~TimerCtrl() {
        StopTimer();
        CancelTimer();
    }
};

// Global timer utilities
class TimerUtil {
public:
    // Get current timestamp as string
    static String GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ".%03d", (int)ms.count());
        
        return String(buffer);
    }
    
    // Format time span
    static String FormatTimeSpan(std::chrono::milliseconds ms) {
        auto hours = std::chrono::duration_cast<std::chrono::hours>(ms);
        ms -= hours;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms);
        ms -= minutes;
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
        
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", 
                (int)hours.count(), (int)minutes.count(), (int)seconds.count(), (int)ms.count());
        
        return String(buffer);
    }
    
    // Measure execution time and log it
    template<typename Func>
    static void Benchmark(const String& name, Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        DLOG(name << " took " << duration.count() << " microseconds");
    }
};

// Convenience macros
#define TIMER_EXECUTE_AFTER(ctrl, ms, func) (ctrl).RunAfter(std::chrono::milliseconds(ms), [this]() { func; })
#define TIMER_EXECUTE_EVERY(ctrl, ms, func) (ctrl).RunEvery(std::chrono::milliseconds(ms), [this]() { func; })

#endif