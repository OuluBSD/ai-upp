#pragma once
#ifndef _Core_Dli_h_
#define _Core_Dli_h_

#include "Core.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>

// Dynamic library interface
class Dli {
private:
    void* handle;
    String library_path;
    static std::mutex global_mutex;
    static std::unordered_map<String, std::shared_ptr<Dli>> loaded_libraries;

public:
    Dli();
    explicit Dli(const String& path);
    ~Dli();
    
    // Load a dynamic library
    bool Load(const String& path);
    
    // Unload the library
    void Unload();
    
    // Check if library is loaded
    bool IsLoaded() const { return handle != nullptr; }
    
    // Get a function from the library
    template<typename T>
    T GetFunction(const String& name) {
        if (!handle) {
            return nullptr;
        }
        
#ifdef _WIN32
        return reinterpret_cast<T>(GetProcAddress((HMODULE)handle, name));
#else
        return reinterpret_cast<T>(dlsym(handle, name));
#endif
    }
    
    // Get library path
    const String& GetPath() const { return library_path; }
    
    // Get error message
    String GetLastError() const;
    
    // Static methods for global library management
    static std::shared_ptr<Dli> LoadLibrary(const String& path);
    static void UnloadLibrary(const String& path);
    static std::shared_ptr<Dli> GetLoadedLibrary(const String& path);
    static void UnloadAllLibraries();
};

// Dynamic library interface helper
class DliHelper {
public:
    // Load and get function in one call
    template<typename T>
    static T LoadFunction(const String& library_path, const String& function_name) {
        auto lib = Dli::LoadLibrary(library_path);
        if (!lib || !lib->IsLoaded()) {
            return nullptr;
        }
        
        return lib->GetFunction<T>(function_name);
    }
    
    // Check if library exists and can be loaded
    static bool CanLoadLibrary(const String& path);
};

// Plugin interface using DLI
class Plugin {
protected:
    std::shared_ptr<Dli> library;
    String plugin_path;
    
public:
    Plugin();
    explicit Plugin(const String& path);
    virtual ~Plugin();
    
    // Load plugin from path
    bool Load(const String& path);
    
    // Unload plugin
    void Unload();
    
    // Check if plugin is loaded
    bool IsLoaded() const { return library && library->IsLoaded(); }
    
    // Initialize plugin (optional virtual function)
    virtual bool Initialize();
    
    // Shutdown plugin (optional virtual function)
    virtual void Shutdown();
    
    // Get plugin path
    const String& GetPath() const { return plugin_path; }
    
    // Get function from plugin
    template<typename T>
    T GetFunction(const String& name) {
        if (!library) {
            return nullptr;
        }
        return library->GetFunction<T>(name);
    }
};

// Plugin manager
class PluginManager {
private:
    std::vector<std::shared_ptr<Plugin>> plugins;
    std::mutex plugins_mutex;
    
public:
    // Load plugin from file
    std::shared_ptr<Plugin> LoadPlugin(const String& path);
    
    // Unload plugin
    void UnloadPlugin(std::shared_ptr<Plugin> plugin);
    
    // Unload plugin by path
    void UnloadPlugin(const String& path);
    
    // Get all loaded plugins
    const std::vector<std::shared_ptr<Plugin>>& GetPlugins() const { return plugins; }
    
    // Find plugin by path
    std::shared_ptr<Plugin> FindPlugin(const String& path);
    
    // Initialize all plugins
    void InitializeAll();
    
    // Shutdown all plugins
    void ShutdownAll();
    
    // Unload all plugins
    void UnloadAll();
};

// Function pointer typedefs for common plugin patterns
typedef int (*PluginVersionFunc)();
typedef bool (*PluginInitializeFunc)();
typedef void (*PluginShutdownFunc)();
typedef const char* (*PluginGetNameFunc)();

// Base plugin class with common functionality
class BasePlugin : public Plugin {
public:
    BasePlugin() = default;
    explicit BasePlugin(const String& path) : Plugin(path) {}
    
    // Common plugin functions
    int GetVersion();
    String GetName();
    String GetDescription();
    
    // Override initialize to call plugin-specific init
    bool Initialize() override;
};

#endif