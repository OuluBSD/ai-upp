#pragma once
#ifndef _CtrlCore_CtrlKbd_h_
#define _CtrlCore_CtrlKbd_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "MKeys.h"
#include <functional>
#include <map>

// Keyboard handling for controls
class CtrlKbd {
protected:
    Ctrl& ctrl;
    std::function<bool(dword key, int count)> key_handler;
    std::function<bool()> key_hook;
    std::map<dword, std::function<bool()>> key_map;
    
public:
    explicit CtrlKbd(Ctrl& c) : ctrl(c) {}
    
    // Set keyboard handler
    CtrlKbd& WhenKey(std::function<bool(dword key, int count)> handler) {
        key_handler = handler;
        return *this;
    }
    
    // Set key hook (called before normal processing)
    CtrlKbd& SetKeyHook(std::function<bool()> hook) {
        key_hook = hook;
        return *this;
    }
    
    // Handle a specific key
    CtrlKbd& AddKey(dword key, std::function<bool()> handler) {
        key_map[key] = handler;
        return *this;
    }
    
    // Remove a key handler
    CtrlKbd& RemoveKey(dword key) {
        key_map.erase(key);
        return *this;
    }
    
    // Process a key event
    virtual bool ProcessKey(dword key, int count = 1) {
        // Call key hook first if exists
        if (key_hook && key_hook()) {
            return true;
        }
        
        // Check for specific key handlers
        auto it = key_map.find(key);
        if (it != key_map.end() && it->second) {
            return it->second();
        }
        
        // Call general key handler
        if (key_handler) {
            return key_handler(key, count);
        }
        
        return false;
    }
    
    // Check if control can accept focus
    virtual bool IsAcceptable() const {
        return true; // By default, controls can accept focus
    }
    
    // Set if control can accept focus
    CtrlKbd& Accept(bool accept = true) {
        // In a real implementation, this would register with focus management
        return *this;
    }
    
    // Focus management
    virtual void SetFocus() {
        // In a real implementation, this would set focus to this control
    }
    
    virtual bool HasFocus() const {
        // In a real implementation, this would check if this control has focus
        return false;
    }
    
    // Get next control in tab order
    virtual std::shared_ptr<Ctrl> GetNextFocusCtrl() const {
        // This would return the next control in the focus chain
        return nullptr;
    }
    
    // Get previous control in tab order
    virtual std::shared_ptr<Ctrl> GetPrevFocusCtrl() const {
        // This would return the previous control in the focus chain
        return nullptr;
    }
    
    // Navigation keys support
    CtrlKbd& NavigateUp(std::shared_ptr<Ctrl> target) { return AddKey(K_UP, [target]() { if(target) target->SetFocus(); return true; }); }
    CtrlKbd& NavigateDown(std::shared_ptr<Ctrl> target) { return AddKey(K_DOWN, [target]() { if(target) target->SetFocus(); return true; }); }
    CtrlKbd& NavigateLeft(std::shared_ptr<Ctrl> target) { return AddKey(K_LEFT, [target]() { if(target) target->SetFocus(); return true; }); }
    CtrlKbd& NavigateRight(std::shared_ptr<Ctrl> target) { return AddKey(K_RIGHT, [target]() { if(target) target->SetFocus(); return true; }); }
    
    // Standard keyboard shortcuts
    CtrlKbd& OnEscape(std::function<bool()> handler) { return AddKey(K_ESC, handler); }
    CtrlKbd& OnEnter(std::function<bool()> handler) { return AddKey(K_ENTER, handler); }
    CtrlKbd& OnReturn(std::function<bool()> handler) { return AddKey(K_ENTER, handler); }
    CtrlKbd& OnTab(std::function<bool()> handler) { return AddKey(K_TAB, handler); }
    CtrlKbd& OnBackspace(std::function<bool()> handler) { return AddKey(K_BACKSPACE, handler); }
    CtrlKbd& OnDelete(std::function<bool()> handler) { return AddKey(K_DELETE, handler); }
    
    // Ctrl+key shortcuts
    CtrlKbd& OnCtrlA(std::function<bool()> handler) { return AddKey(K_CTRL_A, handler); }
    CtrlKbd& OnCtrlC(std::function<bool()> handler) { return AddKey(K_CTRL_C, handler); }
    CtrlKbd& OnCtrlV(std::function<bool()> handler) { return AddKey(K_CTRL_V, handler); }
    CtrlKbd& OnCtrlX(std::function<bool()> handler) { return AddKey(K_CTRL_X, handler); }
    CtrlKbd& OnCtrlZ(std::function<bool()> handler) { return AddKey(K_CTRL_Z, handler); }
    CtrlKbd& OnCtrlY(std::function<bool()> handler) { return AddKey(K_CTRL_Y, handler); }
    
    // Focus events
    std::function<void()> when_focus;
    std::function<void()> when_lose_focus;
    
    CtrlKbd& WhenFocus(std::function<void()> handler) { when_focus = handler; return *this; }
    CtrlKbd& WhenLoseFocus(std::function<void()> handler) { when_lose_focus = handler; return *this; }
    
    // Check if character key
    static bool IsCharKey(dword key) {
        return (key >= 32 && key <= 126) || (key >= 160 && key <= 255);
    }
    
    // Check if special key (not printable)
    static bool IsSpecialKey(dword key) {
        return !IsCharKey(key) && key < 0x1000;
    }
    
    // Convert key code to character
    static String KeyToChar(dword key) {
        if (IsCharKey(key)) {
            return String((char)key);
        }
        return String();
    }
    
    // Check if key is a navigation key
    static bool IsNavKey(dword key) {
        return key == K_UP || key == K_DOWN || key == K_LEFT || key == K_RIGHT;
    }
    
    // Check if key is a function key
    static bool IsFunctionKey(dword key) {
        return (key >= K_F1 && key <= K_F24);
    }
    
    // Check if control key is pressed
    static bool IsCtrlPressed() {
        // In a real implementation, this would check the actual key state
        return false;
    }
    
    // Check if shift key is pressed
    static bool IsShiftPressed() {
        // In a real implementation, this would check the actual key state
        return false;
    }
    
    // Check if alt key is pressed
    static bool IsAltPressed() {
        // In a real implementation, this would check the actual key state
        return false;
    }
    
    // Check if any modifier key is pressed
    static bool IsModifierPressed() {
        return IsCtrlPressed() || IsShiftPressed() || IsAltPressed();
    }
    
    // Translate key to uppercase if it's a letter
    static dword ToUpperKey(dword key) {
        if (key >= 'a' && key <= 'z') {
            return key - 'a' + 'A';
        }
        return key;
    }
    
    // Translate key to lowercase if it's a letter
    static dword ToLowerKey(dword key) {
        if (key >= 'A' && key <= 'Z') {
            return key - 'A' + 'a';
        }
        return key;
    }
};

// Keyboard focus management
class KbdFocus {
private:
    static std::weak_ptr<Ctrl> focused_ctrl;
    
public:
    static std::shared_ptr<Ctrl> GetFocusedCtrl();
    static void SetFocusedCtrl(const std::shared_ptr<Ctrl>& ctrl);
    static void ClearFocus();
    static bool IsFocused(const std::shared_ptr<Ctrl>& ctrl);
    
    // Focus traversal
    static std::shared_ptr<Ctrl> GetNextFocusableCtrl(const std::shared_ptr<Ctrl>& start);
    static std::shared_ptr<Ctrl> GetPrevFocusableCtrl(const std::shared_ptr<Ctrl>& start);
    
    // Process key globally
    static bool ProcessGlobalKey(dword key, int count = 1);
};

// Helper class for creating controls with keyboard support
template<typename BaseCtrl>
class KbdCtrl : public BaseCtrl, public CtrlKbd {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    KbdCtrl() : BaseCtrl(), CtrlKbd(*this) {}
    explicit KbdCtrl(const Rect& r) : BaseCtrl(r), CtrlKbd(*this) {}
    
    // Override key processing to use CtrlKbd's implementation
    bool ProcessKey(dword key, int count = 1) {
        return CtrlKbd::ProcessKey(key, count);
    }
    
    // Helper methods to make keyboard setup easier
    KbdCtrl& OnKey(std::function<bool(dword key, int count)> handler) {
        return WhenKey(handler);
    }
    
    KbdCtrl& OnEsc(std::function<bool()> handler) {
        return OnEscape(handler);
    }
    
    KbdCtrl& OnEnter(std::function<bool()> handler) {
        return AddKey(K_ENTER, handler);
    }
    
    KbdCtrl& AcceptFocus(bool accept = true) {
        return Accept(accept);
    }
    
    // Standard keyboard operations
    KbdCtrl& Accepts(std::function<bool()> condition) {
        // This would conditionally accept focus based on condition
        return *this;
    }
    
    // Set up common editing keys
    KbdCtrl& EditKeys() {
        OnBackspace([this]() { /* default backspace behavior */; return true; });
        OnDelete([this]() { /* default delete behavior */; return true; });
        return *this;
    }
};

#endif