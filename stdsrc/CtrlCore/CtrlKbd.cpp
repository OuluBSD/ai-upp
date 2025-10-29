// STL-backed CtrlCore keyboard functionality implementation

#include "CtrlCore.h"
#include "CtrlKbd.h"

namespace Upp {

// Static members
std::weak_ptr<Ctrl> KbdFocusManager::current_focus;
std::vector<std::function<bool(Ctrl*, dword, int)>> KbdFocusManager::key_hooks;

// KbdFocusManager implementation
std::shared_ptr<Ctrl> KbdFocusManager::GetFocusCtrl() {
    return current_focus.lock();
}

void KbdFocusManager::SetFocusCtrl(const std::shared_ptr<Ctrl>& ctrl) {
    current_focus = ctrl;
    // In a real implementation, this would notify the control
    if (ctrl) {
        //ctrl->GotFocus();
    }
}

void KbdFocusManager::ClearFocus() {
    auto focus = current_focus.lock();
    if (focus) {
        //focus->LostFocus();
    }
    current_focus.reset();
}

bool KbdFocusManager::HasFocus(const std::shared_ptr<Ctrl>& ctrl) {
    auto focus = current_focus.lock();
    return focus && focus == ctrl;
}

std::shared_ptr<Ctrl> KbdFocusManager::GetNextFocusableCtrl(const std::shared_ptr<Ctrl>& start) {
    // In a real implementation, this would traverse the focus chain
    return nullptr;
}

std::shared_ptr<Ctrl> KbdFocusManager::GetPrevFocusableCtrl(const std::shared_ptr<Ctrl>& start) {
    // In a real implementation, this would traverse the focus chain backwards
    return nullptr;
}

void KbdFocusManager::InstallKeyHook(std::function<bool(Ctrl*, dword, int)> hook) {
    key_hooks.push_back(hook);
}

void KbdFocusManager::DeinstallKeyHook(std::function<bool(Ctrl*, dword, int)> hook) {
    // Remove the hook from the vector
    key_hooks.erase(
        std::remove_if(key_hooks.begin(), key_hooks.end(),
                      [&hook](const auto& existing_hook) {
                          return existing_hook.target_type() == hook.target_type();
                      }),
        key_hooks.end());
}

bool KbdFocusManager::ProcessKey(Ctrl* ctrl, dword key, int count) {
    // Process key hooks first
    for (const auto& hook : key_hooks) {
        if (hook(ctrl, key, count)) {
            return true; // Hook handled the key
        }
    }
    
    // In a real implementation, this would call the control's Key method
    return false;
}

bool KbdFocusManager::DispatchKey(dword key, int count) {
    auto focus = current_focus.lock();
    if (focus) {
        // In a real implementation, this would call the focused control's Key method
        return ProcessKey(focus.get(), key, count);
    }
    return false;
}

void KbdFocusManager::RefreshAccessKeys() {
    // In a real implementation, this would refresh access keys display
}

void KbdFocusManager::RefreshAccessKeysDo(bool visible) {
    // In a real implementation, this would show/hide access keys
}

int KbdFocusManager::RegisterSystemHotKey(dword key, std::function<void()> callback) {
    // In a real implementation, this would register a system-wide hotkey
    return -1; // Invalid ID
}

void KbdFocusManager::UnregisterSystemHotKey(int id) {
    // In a real implementation, this would unregister a system-wide hotkey
}

// AccelTbl implementation
AccelTbl& AccelTbl::Add(dword key, std::function<void()> callback, const String& description) {
    entries.emplace_back(key, callback, description);
    return *this;
}

AccelTbl& AccelTbl::Remove(dword key) {
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                      [key](const AccelEntry& entry) {
                          return entry.key == key;
                      }),
        entries.end());
    return *this;
}

AccelTbl& AccelTbl::Enable(dword key, bool enable) {
    for (auto& entry : entries) {
        if (entry.key == key) {
            entry.enabled = enable;
            break;
        }
    }
    return *this;
}

bool AccelTbl::Has(dword key) const {
    return std::any_of(entries.begin(), entries.end(),
                      [key](const AccelEntry& entry) {
                          return entry.key == key;
                      });
}

bool AccelTbl::Process(dword key) const {
    for (const auto& entry : entries) {
        if (entry.key == key && entry.enabled && entry.callback) {
            entry.callback();
            return true;
        }
    }
    return false;
}

String AccelTbl::GetDescription(dword key) const {
    auto it = std::find_if(entries.begin(), entries.end(),
                          [key](const AccelEntry& entry) {
                              return entry.key == key;
                          });
    return (it != entries.end()) ? it->description : String();
}

void AccelTbl::Clear() {
    entries.clear();
}

// KbdUtil implementation
String KbdUtil::GetKeyDesc(dword key) {
    // In a real implementation, this would convert key codes to human-readable descriptions
    return String("Key: ") + std::to_string(key);
}

bool KbdUtil::IsKey(dword key, dword target_key, bool ignore_case) {
    if (ignore_case) {
        return KbdUtil::ToLower(key) == KbdUtil::ToLower(target_key);
    }
    return key == target_key;
}

dword KbdUtil::NormalizeKey(dword key) {
    // Remove key-up flag and normalize
    dword normalized = key & ~K_KEYUP;
    return normalized;
}

bool KbdUtil::IsPrintable(dword key) {
    return (key >= 32 && key < 127) || (key >= 160);
}

dword KbdUtil::ToUpper(dword key) {
    if (key >= 'a' && key <= 'z') {
        return key - 'a' + 'A';
    }
    return key;
}

dword KbdUtil::ToLower(dword key) {
    if (key >= 'A' && key <= 'Z') {
        return key - 'A' + 'a';
    }
    return key;
}

String KbdUtil::KeyToChar(dword key) {
    if (IsPrintable(key)) {
        return String(1, static_cast<char>(key & 0xFF));
    }
    return String();
}

bool KbdUtil::IsCtrlPressed() {
    // In a real implementation, this would check actual key state
    return false;
}

bool KbdUtil::IsAltPressed() {
    // In a real implementation, this would check actual key state
    return false;
}

bool KbdUtil::IsShiftPressed() {
    // In a real implementation, this would check actual key state
    return false;
}

int KbdUtil::GetKeyboardDelay() {
    // In a real implementation, this would get system keyboard delay
    return 500; // Default 500ms
}

int KbdUtil::GetKeyboardSpeed() {
    // In a real implementation, this would get system keyboard speed
    return 30; // Default 30 repeats/sec
}

void KbdUtil::SetKeyboardDelay(int delay) {
    // In a real implementation, this would set system keyboard delay
}

void KbdUtil::SetKeyboardSpeed(int speed) {
    // In a real implementation, this would set system keyboard speed
}

}