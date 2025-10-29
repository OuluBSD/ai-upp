// STL-backed implementation of U++ Core types

#include "CtrlCore.h"

namespace Upp {

// Key description utility function
std::string GetKeyDesc(dword key) {
    std::string desc;
    
    // Handle key up flag
    if (IsKeyUp(key)) {
        desc += "UP ";
    }
    
    // Handle modifiers
    if (IsCtrlPressed(key)) {
        desc += "Ctrl+";
    }
    if (IsAltPressed(key)) {
        desc += "Alt+";
    }
    if (IsShiftPressed(key)) {
        desc += "Shift+";
    }
    
    // Remove modifier flags for base key
    dword base_key = GetBaseKey(key);
    
    // Handle special keys
    if (base_key >= K_F1 && base_key <= K_F12) {
        int fn = base_key - K_F1 + 1;
        desc += "F" + std::to_string(fn);
    } else if (base_key >= K_NUMPAD0 && base_key <= K_NUMPAD9) {
        int num = base_key - K_NUMPAD0;
        desc += "Num" + std::to_string(num);
    } else {
        switch (base_key) {
            case K_LEFT: desc += "Left"; break;
            case K_UP: desc += "Up"; break;
            case K_RIGHT: desc += "Right"; break;
            case K_DOWN: desc += "Down"; break;
            case K_HOME: desc += "Home"; break;
            case K_END: desc += "End"; break;
            case K_PGUP: desc += "PageUp"; break;
            case K_PGDN: desc += "PageDown"; break;
            case K_INSERT: desc += "Insert"; break;
            case K_DELETE: desc += "Delete"; break;
            case K_BACKSPACE: desc += "Backspace"; break;
            case K_TAB: desc += "Tab"; break;
            case K_RETURN: desc += "Enter"; break;
            case K_ESCAPE: desc += "Escape"; break;
            case K_SPACE: desc += "Space"; break;
            case K_MULTIPLY: desc += "Num[*]"; break;
            case K_ADD: desc += "Num[+]"; break;
            case K_SUBTRACT: desc += "Num[-]"; break;
            case K_DIVIDE: desc += "Num[/]"; break;
            default:
                // Handle printable characters
                if (base_key >= 32 && base_key <= 126) {
                    desc += static_cast<char>(base_key);
                } else if (base_key >= 160) {
                    // Extended ASCII characters
                    desc += static_cast<char>(base_key & 0xFF);
                } else {
                    // Unknown key - show hex code
                    desc += "[0x" + std::to_string(base_key) + "]";
                }
                break;
        }
    }
    
    return desc;
}

// Timer utility functions implementation
#include <chrono>
#include <thread>
#include <map>
#include <mutex>

// Simple timer implementation using std::thread
static std::map<void*, std::thread> s_timers;
static std::map<void*, bool> s_timer_active;
static std::mutex s_timer_mutex;

void SetTimeCallback(int delay_ms, const Event<>& cb, void *id) {
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
        s_timers[id] = std::thread([delay_ms, cb, id]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            
            std::lock_guard<std::mutex> lock(s_timer_mutex);
            if (s_timer_active[id]) {
                cb(); // Execute callback
                s_timer_active[id] = false;
            }
        });
    } else {
        // One-shot timer without ID
        std::thread([delay_ms, cb]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            cb(); // Execute callback
        }).detach();
    }
}

void KillTimeCallback(void *id) {
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

bool ExistsTimeCallback(void *id) {
    if (!id) return false;
    
    std::lock_guard<std::mutex> lock(s_timer_mutex);
    auto it = s_timer_active.find(id);
    return it != s_timer_active.end() && it->second;
}

dword GetTimeClick() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return static_cast<dword>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

void Sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

}