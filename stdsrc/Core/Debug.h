#pragma once
#ifndef _Core_Debug_h_
#define _Core_Debug_h_

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include "Core.h"

#ifdef _DEBUG

// Debug output functions
inline void LOG(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

inline void LOGF(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

inline void DUMP(const char* name, const auto& value) {
    std::cout << name << " = " << value << std::endl;
}

inline void DUMPHEX(const char* name, const auto& value) {
    std::cout << name << " = 0x" << std::hex << value << std::dec << std::endl;
}

// Assertion macros
#define ASSERT(x) assert(x)
#define ASSERT_(x, msg) do { if (!(x)) { std::cerr << "Assertion failed: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; assert(x); } } while(0)

// Debug-only code execution
#define DEBUGCODE(x) x

#else

// Release mode - no debug output
inline void LOG(const char* format, ...) {}
inline void LOGF(const char* format, ...) {}
inline void DUMP(const char* name, const auto& value) {}
inline void DUMPHEX(const char* name, const auto& value) {}

// No assertions in release mode
#define ASSERT(x) ((void)0)
#define ASSERT_(x, msg) ((void)0)

// No debug code in release mode
#define DEBUGCODE(x)

#endif

// Common debugging utilities
inline void PrintStackTrace() {
#ifdef _MSC_VER
    // Windows-specific stack trace
    // Implementation would depend on Windows debugging APIs
#elif defined(__GNUC__) || defined(__clang__)
    // Unix-like systems can use backtrace
    // Implementation would depend on execinfo.h
#endif
}

// Memory debugging utilities
#ifdef _DEBUG
inline void CheckMemory() {
    // In debug mode, could integrate with memory debugging tools
}
#else
inline void CheckMemory() {
    // Release mode - no memory checking
}
#endif

// Profiling support
class DebugTimer {
    std::chrono::high_resolution_clock::time_point start_time;
    std::string name;
    
public:
    DebugTimer(const std::string& timer_name) : name(timer_name) {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    ~DebugTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        LOG("%s took %lld microseconds", name.c_str(), duration.count());
    }
};

// Macro for timing code blocks
#define TIMING(name) DebugTimer debug_timer##__LINE__(name)

// Conditional logging
#define LOG_IF(condition, message) do { if (condition) LOG(message); } while(0)

// Hex dump utilities
inline void HexDump(const void* data, size_t size) {
    const unsigned char* p = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            if (i > 0) LOG("");
            LOG("%08zx: ", i);
        }
        LOG("%02x ", p[i]);
    }
    LOG("");
}

// Debug breakpoints
#ifdef _MSC_VER
#define BREAKPOINT() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define BREAKPOINT() __builtin_trap()
#else
#define BREAKPOINT() abort()
#endif

// Debug heap checking
inline void DebugHeapCheck() {
#ifdef _DEBUG
#ifdef _MSC_VER
    // Check heap integrity
    _ASSERTE(_CrtCheckMemory());
#endif
#endif
}

// Debug string utilities
inline std::string DebugString(const std::string& s) {
#ifdef _DEBUG
    return "\"" + s + "\"";
#else
    return s;
#endif
}

#endif