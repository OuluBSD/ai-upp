#ifndef _Core_Debug_h_
#define _Core_Debug_h_

#include "Core.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <vector>

// Debugging and logging macros
#ifdef _DEBUG
    #define DLOG(x) DebugLog() << x << " [File: " << __FILE__ << ", Line: " << __LINE__ << "]"
    #define DUMP(x) DebugLog() << #x << " = " << (x) << " [File: " << __FILE__ << ", Line: " << __LINE__ << "]"
    #define ASSERT(condition) do { if (!(condition)) { DebugLog() << "ASSERTION FAILED: " << #condition << " [File: " << __FILE__ << ", Line: " << __LINE__ << "]"; abort(); } } while(0)
    #define VERIFY(condition) ASSERT(condition)
#else
    #define DLOG(x) (void)0
    #define DUMP(x) (void)0
    #define ASSERT(condition) (void)0
    #define VERIFY(condition) (condition)
#endif

// Performance timing
#ifdef _DEBUG
    #define TIME(x) TimeScope _time_scope_(#x)
#else
    #define TIME(x) (void)0
#endif

// Debug logging class
class DebugLog {
private:
    std::ostringstream buffer;
    static std::mutex log_mutex;
    static std::ofstream log_file;
    static bool initialized;
    static void Initialize();

public:
    DebugLog();
    ~DebugLog();
    
    template<typename T>
    DebugLog& operator<<(const T& value) {
        buffer << value;
        return *this;
    }
    
    DebugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        buffer << manipulator;
        return *this;
    }
    
    static void SetLogFile(const String& filename);
    static void CloseLogFile();
};

// Memory leak detection
class DebugMemory {
private:
    static std::map<void*, size_t> allocations;
    static std::mutex alloc_mutex;
    static size_t total_bytes;
    static size_t peak_bytes;
    
public:
    static void* Malloc(size_t size);
    static void Free(void* ptr);
    static void* Realloc(void* ptr, size_t size);
    static void ReportLeaks();
    static size_t GetTotalAllocated() { return total_bytes; }
    static size_t GetPeakUsage() { return peak_bytes; }
};

// Scope-based timing
class TimeScope {
private:
    std::string operation_name;
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    TimeScope(const std::string& name);
    ~TimeScope();
};

// Call trace
class CallTrace {
private:
    static std::vector<std::string> call_stack;
    static std::mutex trace_mutex;
    
public:
    static void Enter(const std::string& function_name);
    static void Leave(const std::string& function_name);
    static void PrintTrace();
    static void ClearTrace();
};

// Breakpoint helper
inline void Breakpoint() {
#ifdef _MSC_VER
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    // For other compilers or platforms, just log
    DLOG("Breakpoint hit");
#endif
}

// Assertion helpers
template<typename T>
bool IsNull(T* ptr) { return ptr == nullptr; }

template<typename T>
bool IsNotNull(T* ptr) { return ptr != nullptr; }

// Watch variable for debugging
template<typename T>
class DebugWatch {
private:
    const T* watched_var;
    String var_name;
    T last_value;
    bool has_last_value;
    
public:
    DebugWatch(const T* var, const String& name) : watched_var(var), var_name(name), has_last_value(false) {}
    
    void Check() {
        if (has_last_value && last_value != *watched_var) {
            DLOG("Variable '" << var_name << "' changed from " << last_value << " to " << *watched_var);
        }
        last_value = *watched_var;
        has_last_value = true;
    }
};

// Conditional debugging
class ConditionalDebug {
private:
    static std::map<String, bool> debug_flags;
    
public:
    static void SetFlag(const String& flag, bool value = true);
    static bool IsEnabled(const String& flag);
    static void Enable(const String& flag) { SetFlag(flag, true); }
    static void Disable(const String& flag) { SetFlag(flag, false); }
};

// Debugging macros for conditional execution
#define CLOG(flag, x) do { if (ConditionalDebug::IsEnabled(flag)) { DebugLog() << x; } } while(0)
#define CDUMP(flag, x) do { if (ConditionalDebug::IsEnabled(flag)) { DebugLog() << #x << " = " << (x); } } while(0)

#endif