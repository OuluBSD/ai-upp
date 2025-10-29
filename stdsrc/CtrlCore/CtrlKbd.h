#pragma once
#ifndef _CtrlCore_CtrlKbd_h_
#define _CtrlCore_CtrlKbd_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "MKeys.h"
#include <functional>
#include <map>
#include <memory>

namespace Upp {

// Keyboard event class - represents a keyboard-related event
class KbdEvent {
public:
    dword key;
    int count;
    bool is_key_up;
    bool is_processed;
    
    KbdEvent(dword k = 0, int c = 1) : key(k), count(c), is_key_up(false), is_processed(false) {}
    
    bool IsChar() const { return key < K_DELTA && key >= 32; }
    bool IsFunctionKey() const { return key >= K_F1 && key <= K_F24; }
    bool IsNavigationKey() const { 
        return key == K_LEFT || key == K_RIGHT || key == K_UP || key == K_DOWN ||
               key == K_HOME || key == K_END || key == K_PGUP || key == K_PGDN;
    }
    bool IsModifier() const { 
        return key == K_CTRL_KEY || key == K_ALT_KEY || key == K_SHIFT_KEY;
    }
    
    bool IsCtrlPressed() const { return (key & K_CTRL) != 0; }
    bool IsAltPressed() const { return (key & K_ALT) != 0; }
    bool IsShiftPressed() const { return (key & K_SHIFT) != 0; }
    
    // Get the base key without modifiers
    dword GetBaseKey() const { 
        return key & ~(K_CTRL | K_ALT | K_SHIFT | K_KEYUP); 
    }
};

// Keyboard focus management class
class KbdFocusManager {
private:
    static std::weak_ptr<Ctrl> current_focus;
    static std::vector<std::function<bool(Ctrl*, dword, int)>> key_hooks;
    
public:
    // Focus management
    static std::shared_ptr<Ctrl> GetFocusCtrl();
    static void SetFocusCtrl(const std::shared_ptr<Ctrl>& ctrl);
    static void ClearFocus();
    static bool HasFocus(const std::shared_ptr<Ctrl>& ctrl);
    
    // Focus traversal
    static std::shared_ptr<Ctrl> GetNextFocusableCtrl(const std::shared_ptr<Ctrl>& start);
    static std::shared_ptr<Ctrl> GetPrevFocusableCtrl(const std::shared_ptr<Ctrl>& start);
    
    // Key hook management
    static void InstallKeyHook(std::function<bool(Ctrl*, dword, int)> hook);
    static void DeinstallKeyHook(std::function<bool(Ctrl*, dword, int)> hook);
    
    // Key processing
    static bool ProcessKey(Ctrl* ctrl, dword key, int count);
    static bool DispatchKey(dword key, int count);
    
    // Access keys support
    static void RefreshAccessKeys();
    static void RefreshAccessKeysDo(bool visible);
    
    // System hotkeys
    static int RegisterSystemHotKey(dword key, std::function<void()> callback);
    static void UnregisterSystemHotKey(int id);
};

// Helper class to add keyboard functionality to controls
template<typename BaseCtrl>
class WithKbd : public BaseCtrl {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    WithKbd() : BaseCtrl() {}
    explicit WithKbd(const Rect& r) : BaseCtrl(r) {}
    
    // Keyboard event handling methods that delegate to the control
    bool ProcessKey(dword key, int count = 1) {
        return this->Key(key, count);
    }
    
    void OnGotFocus() {
        this->GotFocus();
    }
    
    void OnLostFocus() {
        this->LostFocus();
    }
    
    bool HandleHotKey(dword key) {
        return this->HotKey(key);
    }
    
    // Fluent interface for focus management
    WithKbd& RequestFocus() {
        this->SetFocus();
        return *this;
    }
    
    bool IsFocused() const {
        return this->HasFocus();
    }
    
    WithKbd& WantFocus(bool want = true) {
        // In a real implementation, this would register with focus management
        return *this;
    }
    
    WithKbd& NoWantFocus() {
        return WantFocus(false);
    }
    
    // Access keys support
    WithKbd& RefreshAccessKeys() {
        // In a real implementation, this would refresh access keys display
        return *this;
    }
    
    // Keyboard navigation helpers
    WithKbd& NavigateWithArrows(bool enable = true) {
        // Configure control to handle arrow key navigation
        return *this;
    }
    
    WithKbd& AcceptTabs(bool accept = true) {
        // Configure control to accept tab key navigation
        return *this;
    }
    
    // Standard keyboard shortcuts
    WithKbd& OnEnter(std::function<bool()> handler) {
        // Setup enter key handling
        return *this;
    }
    
    WithKbd& OnEscape(std::function<bool()> handler) {
        // Setup escape key handling
        return *this;
    }
    
    WithKbd& OnCtrlC(std::function<bool()> handler) {
        // Setup copy shortcut
        return *this;
    }
    
    WithKbd& OnCtrlV(std::function<bool()> handler) {
        // Setup paste shortcut
        return *this;
    }
    
    WithKbd& OnCtrlX(std::function<bool()> handler) {
        // Setup cut shortcut
        return *this;
    }
    
    WithKbd& OnCtrlZ(std::function<bool()> handler) {
        // Setup undo shortcut
        return *this;
    }
    
    WithKbd& OnCtrlY(std::function<bool()> handler) {
        // Setup redo shortcut
        return *this;
    }
    
    WithKbd& OnDelete(std::function<bool()> handler) {
        // Setup delete key handling
        return *this;
    }
    
    WithKbd& OnBackspace(std::function<bool()> handler) {
        // Setup backspace key handling
        return *this;
    }
};

// Global keyboard utilities
class KbdUtil {
public:
    // Convert key code to human-readable description
    static String GetKeyDesc(dword key);
    
    // Check if key combination matches
    static bool IsKey(dword key, dword target_key, bool ignore_case = true);
    
    // Normalize key code (handle case, modifiers, etc.)
    static dword NormalizeKey(dword key);
    
    // Check if key is a printable character
    static bool IsPrintable(dword key);
    
    // Convert key to uppercase/lowercase
    static dword ToUpper(dword key);
    static dword ToLower(dword key);
    
    // Get character from key code
    static String KeyToChar(dword key);
    
    // Check modifier states
    static bool IsCtrlPressed();
    static bool IsAltPressed();
    static bool IsShiftPressed();
    
    // Keyboard timing
    static int GetKeyboardDelay();
    static int GetKeyboardSpeed();
    static void SetKeyboardDelay(int delay);
    static void SetKeyboardSpeed(int speed);
};

// Keyboard accelerator table
class AccelTbl {
private:
    struct AccelEntry {
        dword key;
        std::function<void()> callback;
        String description;
        bool enabled;
        
        AccelEntry(dword k, std::function<void()> cb, const String& desc = "") 
            : key(k), callback(cb), description(desc), enabled(true) {}
    };
    
    std::vector<AccelEntry> entries;
    
public:
    // Add accelerator
    AccelTbl& Add(dword key, std::function<void()> callback, const String& description = "");
    
    // Remove accelerator
    AccelTbl& Remove(dword key);
    
    // Enable/disable accelerator
    AccelTbl& Enable(dword key, bool enable = true);
    
    // Check if accelerator exists
    bool Has(dword key) const;
    
    // Process accelerator
    bool Process(dword key) const;
    
    // Get accelerator description
    String GetDescription(dword key) const;
    
    // Clear all accelerators
    void Clear();
    
    // Get all accelerators
    const std::vector<AccelEntry>& GetEntries() const { return entries; }
};

// Macro for convenient keyboard handling setup
#define WITH_KEYBOARD(ctrl) WithKbd<std::decay_t<decltype(ctrl)>>()

}

#endif