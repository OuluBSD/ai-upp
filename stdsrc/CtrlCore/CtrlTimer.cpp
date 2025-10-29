// STL-backed CtrlCore timer functionality implementation

#include "CtrlTimer.h"
#include <chrono>
#include <thread>
#include <map>
#include <mutex>

namespace Upp {

// Timer implementation details
class TimerManager {
private:
    static std::map<void*, std::thread> s_timers;
    static std::map<void*, bool> s_timer_active;
    static std::mutex s_timer_mutex;
    
public:
    static void SetTimer(int delay_ms, const Event<>& cb, void *id, bool repeating = false) {
        std::lock_guard<std::mutex> lock(s_timer_mutex);
        
        // Kill existing timer with same ID
        if (id && s_timer_active[id]) {
            s_timer_active[id] = false;
            if (s_timers.find(id) != s_timers.end()) {
                s_timers[id].detach();
            }
        }
        
        if (delay_ms <= 0) return; // Invalid delay
        
        // Create new timer
        if (id) {
            s_timer_active[id] = true;
            s_timers[id] = std::thread([delay_ms, cb, id, repeating]() {
                do {
                    std::this_thread::sleep_for(std::chrono::milliseconds(abs(delay_ms)));
                    
                    std::lock_guard<std::mutex> lock(s_timer_mutex);
                    if (s_timer_active[id]) {
                        cb(); // Execute callback
                        if (!repeating) {
                            s_timer_active[id] = false;
                            break;
                        }
                    } else {
                        break;
                    }
                } while (repeating && s_timer_active[id]);
                
                if (s_timer_active[id]) {
                    s_timer_active[id] = false;
                }
            });
        } else {
            // One-shot timer without ID
            std::thread([delay_ms, cb, repeating]() {
                do {
                    std::this_thread::sleep_for(std::chrono::milliseconds(abs(delay_ms)));
                    cb(); // Execute callback
                    if (!repeating) break;
                } while (repeating);
            }).detach();
        }
    }
    
    static void KillTimer(void *id) {
        if (!id) return;
        
        std::lock_guard<std::mutex> lock(s_timer_mutex);
        if (s_timer_active[id]) {
            s_timer_active[id] = false;
            if (s_timers.find(id) != s_timers.end()) {
                s_timers[id].detach();
                s_timers.erase(id);
            }
        }
    }
    
    static bool ExistsTimer(void *id) {
        if (!id) return false;
        
        std::lock_guard<std::mutex> lock(s_timer_mutex);
        auto it = s_timer_active.find(id);
        return it != s_timer_active.end() && it->second;
    }
    
    static void Cleanup() {
        std::lock_guard<std::mutex> lock(s_timer_mutex);
        for (auto& pair : s_timers) {
            if (pair.second.joinable()) {
                pair.second.detach();
            }
        }
        s_timers.clear();
        s_timer_active.clear();
    }
};

// Static member definitions
std::map<void*, std::thread> TimerManager::s_timers;
std::map<void*, bool> TimerManager::s_timer_active;
std::mutex TimerManager::s_timer_mutex;

// CtrlTimer implementation
CtrlTimer::CtrlTimer(Ctrl& c) : ctrl(c) {}

void CtrlTimer::SetTimeCallback(int delay_ms, std::function<void()> cb, int id) {
    ctrl.SetTimeCallback(delay_ms, Event<>() << cb, id);
}

void CtrlTimer::KillTimeCallback(int id) {
    ctrl.KillTimeCallback(id);
}

void CtrlTimer::KillSetTimeCallback(int delay_ms, std::function<void()> cb, int id) {
    ctrl.KillSetTimeCallback(delay_ms, Event<>() << cb, id);
}

bool CtrlTimer::ExistsTimeCallback(int id) const {
    return ctrl.ExistsTimeCallback(id);
}

void CtrlTimer::PostCallback(std::function<void()> cb, int id) {
    ctrl.PostCallback(Event<>() << cb, id);
}

void CtrlTimer::KillPostCallback(std::function<void()> cb, int id) {
    ctrl.KillPostCallback(Event<>() << cb, id);
}

void CtrlTimer::SetGlobalTimeCallback(int delay_ms, std::function<void()> cb, void *id) {
    TimerManager::SetTimer(delay_ms, Event<>() << cb, id, delay_ms < 0);
}

void CtrlTimer::KillGlobalTimeCallback(void *id) {
    TimerManager::KillTimer(id);
}

bool CtrlTimer::ExistsGlobalTimeCallback(void *id) {
    return TimerManager::ExistsTimer(id);
}

dword CtrlTimer::GetTimeClick() {
    return UPP::GetTimeClick();
}

void CtrlTimer::Sleep(int milliseconds) {
    UPP::Sleep(milliseconds);
}

// Helper methods for common timer patterns
CtrlTimer& CtrlTimer::Timeout(int ms, std::function<void()> callback) {
    SetTimeCallback(ms, callback, 0);
    return *this;
}

CtrlTimer& CtrlTimer::Repeat(int ms, std::function<void()> callback) {
    SetTimeCallback(-ms, callback, 0); // Negative delay means repeat
    return *this;
}

CtrlTimer& CtrlTimer::Once(std::function<void()> callback) {
    PostCallback(callback, 0);
    return *this;
}

void CtrlTimer::StopTimer(int id) {
    KillTimeCallback(id);
}

void CtrlTimer::StopAllTimers() {
    // Stop all timer IDs (0-TIMEID_COUNT) that might be used
    for (int i = 0; i < Ctrl::TIMEID_COUNT; i++) {
        KillTimeCallback(i);
    }
}

bool CtrlTimer::HasTimer(int id) const {
    return ExistsTimeCallback(id);
}

void CtrlTimer::WhenDragStart(std::function<bool(Point, dword)> handler) {
    // Placeholder for drag start handler
}

void CtrlTimer::WhenDragOver(std::function<bool(Point, dword)> handler) {
    // Placeholder for drag over handler
}

void CtrlTimer::WhenDragDrop(std::function<bool(Point, dword)> handler) {
    // Placeholder for drag drop handler
}

void CtrlTimer::WhenDragLeave(std::function<bool(Point, dword)> handler) {
    // Placeholder for drag leave handler
}

// Global timer functions implementation
void SetGlobalTimer(int delay_ms, std::function<void()> cb, void *id) {
    CtrlTimer::SetGlobalTimeCallback(delay_ms, cb, id);
}

void KillGlobalTimer(void *id) {
    TimerManager::KillTimer(id);
}

bool HasGlobalTimer(void *id) {
    return TimerManager::ExistsTimer(id);
}

// Time utilities
dword GetTime() {
    return UPP::GetTimeClick();
}

// TimerCtrl template method implementations
template<typename BaseCtrl>
TimerCtrl<BaseCtrl>::TimerCtrl() : BaseCtrl() {}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>::TimerCtrl(const Rect& r) : BaseCtrl(r) {}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>& TimerCtrl<BaseCtrl>::timeout(int ms, std::function<void()> callback) {
    this->SetTimeCallback(ms, Event<>() << callback, 0);
    return *this;
}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>& TimerCtrl<BaseCtrl>::repeat(int ms, std::function<void()> callback) {
    this->SetTimeCallback(-ms, Event<>() << callback, 0); // Negative means repeat
    return *this;
}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>& TimerCtrl<BaseCtrl>::once(std::function<void()> callback) {
    this->PostCallback(Event<>() << callback, 0);
    return *this;
}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>& TimerCtrl<BaseCtrl>::cancel(int id) {
    this->KillTimeCallback(id);
    return *this;
}

template<typename BaseCtrl>
bool TimerCtrl<BaseCtrl>::has_timer(int id) const {
    return this->ExistsTimeCallback(id);
}

template<typename BaseCtrl>
TimerCtrl<BaseCtrl>::~TimerCtrl() {
    // Clean up any remaining timers when control is destroyed
    for (int i = 0; i < Ctrl::TIMEID_COUNT; i++) {
        this->KillTimeCallback(i);
    }
}

}