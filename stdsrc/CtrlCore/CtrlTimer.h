#pragma once
#ifndef _CtrlCore_CtrlTimer_h_
#define _CtrlCore_CtrlTimer_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include <functional>

namespace Upp {

// Timer system for controls - matching the original U++ architecture
// Timer functionality is built into the Ctrl class itself
class CtrlTimer {
protected:
    Ctrl& ctrl;
    
public:
    explicit CtrlTimer(Ctrl& c) : ctrl(c) {}
    
    // Control-specific timer methods (these call Ctrl methods)
    void SetTimeCallback(int delay_ms, std::function<void()> cb, int id = 0) {
        ctrl.SetTimeCallback(delay_ms, Event<>() << cb, id);
    }
    
    void KillTimeCallback(int id = 0) {
        ctrl.KillTimeCallback(id);
    }
    
    void KillSetTimeCallback(int delay_ms, std::function<void()> cb, int id = 0) {
        ctrl.KillSetTimeCallback(delay_ms, Event<>() << cb, id);
    }
    
    bool ExistsTimeCallback(int id = 0) const {
        return ctrl.ExistsTimeCallback(id);
    }
    
    void PostCallback(std::function<void()> cb, int id = 0) {
        ctrl.PostCallback(Event<>() << cb, id);
    }
    
    void KillPostCallback(std::function<void()> cb, int id = 0) {
        ctrl.KillPostCallback(Event<>() << cb, id);
    }
    
    // Static timer utilities
    static void SetGlobalTimeCallback(int delay_ms, std::function<void()> cb, void *id = nullptr) {
        UPP::SetTimeCallback(delay_ms, Event<>() << cb, id);
    }
    
    static void KillGlobalTimeCallback(void *id = nullptr) {
        UPP::KillTimeCallback(id);
    }
    
    static bool ExistsGlobalTimeCallback(void *id) {
        return UPP::ExistsTimeCallback(id);
    }
    
    static dword GetTimeClick() {
        return UPP::GetTimeClick();
    }
    
    // Timer utilities
    static void Sleep(int milliseconds) {
        UPP::Sleep(milliseconds);
    }
    
    // Helper methods for common timer patterns
    CtrlTimer& Timeout(int ms, std::function<void()> callback) {
        SetTimeCallback(ms, callback, 0);
        return *this;
    }
    
    CtrlTimer& Repeat(int ms, std::function<void()> callback) {
        SetTimeCallback(-ms, callback, 0); // Negative delay means repeat
        return *this;
    }
    
    CtrlTimer& Once(std::function<void()> callback) {
        PostCallback(callback, 0);
        return *this;
    }
    
    void StopTimer(int id = 0) {
        KillTimeCallback(id);
    }
    
    void StopAllTimers() {
        // Stop all timer IDs (0-TIMEID_COUNT) that might be used
        for (int i = 0; i < Ctrl::TIMEID_COUNT; i++) {
            KillTimeCallback(i);
        }
    }
    
    // Check if control has active timers
    bool HasTimer(int id = 0) const {
        return ExistsTimeCallback(id);
    }
};

// Global timer functions
inline void SetGlobalTimer(int delay_ms, std::function<void()> cb, void *id = nullptr) {
    CtrlTimer::SetGlobalTimeCallback(delay_ms, cb, id);
}

inline void KillGlobalTimer(void *id = nullptr) {
    UPP::KillTimeCallback(id);
}

inline bool HasGlobalTimer(void *id) {
    return UPP::ExistsTimeCallback(id);
}

// Time utilities
inline dword GetTime() {
    return UPP::GetTimeClick();
}

// Helper class for controls with timer support
template<typename BaseCtrl>
class TimerCtrl : public BaseCtrl {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    TimerCtrl() : BaseCtrl() {}
    explicit TimerCtrl(const Rect& r) : BaseCtrl(r) {}
    
    // Fluent interface for timer operations
    TimerCtrl& timeout(int ms, std::function<void()> callback) {
        this->SetTimeCallback(ms, Event<>() << callback, 0);
        return *this;
    }
    
    TimerCtrl& repeat(int ms, std::function<void()> callback) {
        this->SetTimeCallback(-ms, Event<>() << callback, 0); // Negative means repeat
        return *this;
    }
    
    TimerCtrl& once(std::function<void()> callback) {
        this->PostCallback(Event<>() << callback, 0);
        return *this;
    }
    
    TimerCtrl& cancel(int id = 0) {
        this->KillTimeCallback(id);
        return *this;
    }
    
    bool has_timer(int id = 0) const {
        return this->ExistsTimeCallback(id);
    }
    
    ~TimerCtrl() {
        // Clean up any remaining timers when control is destroyed
        for (int i = 0; i < Ctrl::TIMEID_COUNT; i++) {
            this->KillTimeCallback(i);
        }
    }
};

}

#endif