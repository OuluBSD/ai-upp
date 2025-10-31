#pragma once
#ifndef _Core_Dli_h_
#define _Core_Dli_h_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <dlfcn.h>
#include "Core.h"

// Dynamic Library Interface (DLI) for stdsrc
// Provides cross-platform dynamic library loading and symbol resolution

class DLI {
private:
    void* handle;
    std::string library_path;
    bool loaded;
    
public:
    DLI() : handle(nullptr), loaded(false) {}
    
    DLI(const std::string& library_name) : handle(nullptr), library_path(library_name), loaded(false) {
        Load(library_name);
    }
    
    ~DLI() {
        Unload();
    }
    
    // Load a dynamic library
    bool Load(const std::string& library_name) {
        if (loaded) {
            Unload();
        }
        
        // Try loading with different extensions based on platform
#ifdef __APPLE__
        std::string lib_with_ext = library_name + ".dylib";
#elif defined(_WIN32)
        std::string lib_with_ext = library_name + ".dll";
#else
        std::string lib_with_ext = "lib" + library_name + ".so";
#endif
        
        handle = dlopen(lib_with_ext.c_str(), RTLD_LAZY);
        if (!handle) {
            // Try without prefix/suffix
            handle = dlopen(library_name.c_str(), RTLD_LAZY);
        }
        
        if (handle) {
            library_path = lib_with_ext;
            loaded = true;
            return true;
        }
        
        return false;
    }
    
    // Unload the dynamic library
    void Unload() {
        if (handle && loaded) {
            dlclose(handle);
            handle = nullptr;
            loaded = false;
        }
    }
    
    // Check if library is loaded
    bool IsLoaded() const {
        return loaded && handle != nullptr;
    }
    
    // Get library path
    const std::string& GetLibraryPath() const {
        return library_path;
    }
    
    // Get error message from last operation
    std::string GetLastError() const {
        const char* error = dlerror();
        return error ? std::string(error) : std::string("No error");
    }
    
    // Resolve a symbol from the library
    template<typename T>
    T Resolve(const std::string& symbol_name) {
        if (!loaded || !handle) {
            return nullptr;
        }
        
        void* symbol = dlsym(handle, symbol_name.c_str());
        if (!symbol) {
            return nullptr;
        }
        
        return reinterpret_cast<T>(symbol);
    }
    
    // Resolve a symbol and store it in a variable
    template<typename T>
    bool Get(const std::string& symbol_name, T& result) {
        T symbol = Resolve<T>(symbol_name);
        if (symbol) {
            result = symbol;
            return true;
        }
        return false;
    }
    
    // Operator to check if loaded
    operator bool() const {
        return IsLoaded();
    }
    
    // Move constructor
    DLI(DLI&& other) noexcept 
        : handle(other.handle), library_path(std::move(other.library_path)), loaded(other.loaded) {
        other.handle = nullptr;
        other.loaded = false;
    }
    
    // Move assignment
    DLI& operator=(DLI&& other) noexcept {
        if (this != &other) {
            Unload();
            handle = other.handle;
            library_path = std::move(other.library_path);
            loaded = other.loaded;
            other.handle = nullptr;
            other.loaded = false;
        }
        return *this;
    }
    
    // Delete copy constructor and assignment
    DLI(const DLI&) = delete;
    DLI& operator=(const DLI&) = delete;
};

// Global DLI manager for tracking loaded libraries
class DLManager {
private:
    static std::map<std::string, std::shared_ptr<DLI>> loaded_libraries;
    
public:
    // Load a library (or return existing if already loaded)
    static std::shared_ptr<DLI> Load(const std::string& library_name) {
        auto it = loaded_libraries.find(library_name);
        if (it != loaded_libraries.end()) {
            return it->second;
        }
        
        auto dli = std::make_shared<DLI>(library_name);
        if (*dli) {
            loaded_libraries[library_name] = dli;
            return dli;
        }
        
        return nullptr;
    }
    
    // Unload a library
    static void Unload(const std::string& library_name) {
        auto it = loaded_libraries.find(library_name);
        if (it != loaded_libraries.end()) {
            it->second->Unload();
            loaded_libraries.erase(it);
        }
    }
    
    // Check if a library is loaded
    static bool IsLoaded(const std::string& library_name) {
        auto it = loaded_libraries.find(library_name);
        return it != loaded_libraries.end() && it->second->IsLoaded();
    }
    
    // Get all loaded libraries
    static const std::map<std::string, std::shared_ptr<DLI>>& GetLoadedLibraries() {
        return loaded_libraries;
    }
    
    // Unload all libraries
    static void UnloadAll() {
        for (auto& pair : loaded_libraries) {
            pair.second->Unload();
        }
        loaded_libraries.clear();
    }
};

// Initialize static member
std::map<std::string, std::shared_ptr<DLI>> DLManager::loaded_libraries;

// Utility functions for common DLI operations
template<typename T>
inline bool DliGet(const std::string& library_name, const std::string& symbol_name, T& result) {
    auto dli = DLManager::Load(library_name);
    if (dli) {
        return dli->Get(symbol_name, result);
    }
    return false;
}

template<typename T>
inline T DliResolve(const std::string& library_name, const std::string& symbol_name) {
    auto dli = DLManager::Load(library_name);
    if (dli) {
        return dli->Resolve<T>(symbol_name);
    }
    return nullptr;
}

// Convenience macros for DLI usage
#define DLI_LOAD(lib) DLManager::Load(lib)
#define DLI_GET(lib, sym, var) DliGet<decltype(var)>(lib, sym, var)
#define DLI_RESOLVE(lib, sym, type) DliResolve<type>(lib, sym)

#endif