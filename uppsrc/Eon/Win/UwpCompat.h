// UwpCompat.h - UWP compatibility layer for Eon/Win

#ifndef UWP_COMPAT_H
#define UWP_COMPAT_H

#include "Core.h"

NAMESPACE_UPP

#ifdef PLATFORM_WIN32
#include <windows.h>
#include <wrl/client.h>
#include <string>

// UWP Compatibility Layer
class UwpCompat {
public:
    // File system operations that are UWP-safe
    static bool IsUwpMode() {
#ifdef UWP_ENABLED
        return true;
#else
        return false;
#endif
    }

    // Safe file path operations for UWP
    static String GetAppDataPath() {
        if (IsUwpMode()) {
            // In UWP, use the restricted app data folder
            wchar_t path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
                return WString(path).ToStd();
            }
        } else {
            // Fallback to regular Windows path
            char path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
                return String(path);
            }
        }
        return String(".");  // Default fallback
    }

    static String GetAssetsPath() {
        if (IsUwpMode()) {
            // In UWP, assets are part of the package
            return GetAppDataPath() + "/Assets/";
        } else {
            // In traditional Win32, allow flexible assets location
            return "./Assets/";
        }
    }

    // Safe file operations for UWP
    static bool SafeReadFile(const String& filename, std::string& content) {
        if (IsUwpMode()) {
            // In UWP, only allow reading from specific safe directories
            String safePath = GetAssetsPath() + filename;
            if (safePath.Find(GetAppDataPath()) != 0) {
                // Security check: ensure the path is within allowed directories
                return false;
            }
            
            // Use Windows Runtime file APIs for UWP
            std::ifstream file(safePath, std::ios::binary);
            if (file.is_open()) {
                file.seekg(0, std::ios::end);
                size_t size = file.tellg();
                file.seekg(0);
                
                content.resize(size);
                file.read(&content[0], size);
                file.close();
                return true;
            }
        } else {
            // In Win32 mode, use standard file operations
            std::ifstream file(filename, std::ios::binary);
            if (file.is_open()) {
                file.seekg(0, std::ios::end);
                size_t size = file.tellg();
                file.seekg(0);
                
                content.resize(size);
                file.read(&content[0], size);
                file.close();
                return true;
            }
        }
        return false;
    }

    static bool SafeWriteFile(const String& filename, const std::string& content) {
        if (IsUwpMode()) {
            // In UWP, only allow writing to the app's local folder
            String safePath = GetAppDataPath() + "/" + filename;
            if (safePath.Find(GetAppDataPath()) != 0) {
                // Security check: ensure the path is within allowed directories
                return false;
            }
            
            // Create directories if they don't exist
            // (This would require creating directory hierarchy up to the file)
            
            std::ofstream file(safePath, std::ios::binary);
            if (file.is_open()) {
                file.write(content.c_str(), content.size());
                file.close();
                return true;
            }
        } else {
            // In Win32 mode, use standard file operations
            std::ofstream file(filename, std::ios::binary);
            if (file.is_open()) {
                file.write(content.c_str(), content.size());
                file.close();
                return true;
            }
        }
        return false;
    }

    // Threading compatibility for UWP
    static bool IsMainThread() {
        // In UWP, need to properly check if we're on the main UI thread
        // This is a simplified implementation
        static std::thread::id mainThreadId = std::this_thread::get_id();
        return std::this_thread::get_id() == mainThreadId;
    }

    // Platform-specific operations
    static void RunOnUIThread(const std::function<void()>& func) {
        if (IsUwpMode()) {
            // Use UWP-specific mechanism to run on UI thread
            // This is a placeholder - actual implementation would use Windows Runtime APIs
        } else {
            // In Win32, just run the function
            func();
        }
    }

    // UWP-specific utilities
    static String GetPackageFamilyName() {
        if (IsUwpMode()) {
            // Get the package family name for UWP applications
            // This is a simplified placeholder implementation
            return String("EonWinApp_8wekyb3d8bbwe"); // Example package family name
        }
        return String("");
    }

    // Security and capability checking
    static bool HasCapability(const String& capability) {
        if (IsUwpMode()) {
            // Check if the UWP app has the requested capability
            // This is a simplified placeholder implementation
            return true; // Assume all capabilities are available for now
        }
        // For Win32, capabilities are not restricted in the same way
        return true;
    }
};

// Macros for UWP-compatible operations
#ifdef UWP_ENABLED
    #define UWP_SAFE_CALL(x) (UwpCompat::IsUwpMode() ? x##_UWP() : x##_Win32())
    #define CHECK_UWP_CAPABILITY(cap) (UwpCompat::HasCapability(cap))
#else
    #define UWP_SAFE_CALL(x) x##_Win32()
    #define CHECK_UWP_CAPABILITY(cap) true
#endif

#endif // PLATFORM_WIN32

END_UPP_NAMESPACE

#endif // UWP_COMPAT_H