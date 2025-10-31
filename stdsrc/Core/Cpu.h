#pragma once
#ifndef _Core_Cpu_h_
#define _Core_Cpu_h_

#include "Core.h"

// CPU detection and feature detection for stdsrc
// This is a simplified version that detects common CPU features

#ifdef _MSC_VER
#include <intrin.h>
#endif

// CPU architecture detection
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64)
#define CPU_AMD64
#elif defined(__i386__) || defined(_M_IX86)
#define CPU_X86
#elif defined(__arm__) || defined(_M_ARM)
#define CPU_ARM
#elif defined(__aarch64__)
#define CPU_ARM64
#elif defined(__POWERPC__) || defined(__powerpc__)
#define CPU_POWERPC
#elif defined(__sparc__)
#define CPU_SPARC
#elif defined(__mips__)
#define CPU_MIPS
#endif

// Endianness detection
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CPU_LE
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CPU_BE
#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define CPU_LE  // Assume little endian for x86/x64
#else
#define CPU_BE  // Assume big endian for other architectures
#endif

// SIMD instruction set detection
#ifdef CPU_X86
#ifdef _MSC_VER
// Microsoft Visual C++
inline bool CpuMMX() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[3] & (1 << 23)) != 0;
}

inline bool CpuSSE() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[3] & (1 << 25)) != 0;
}

inline bool CpuSSE2() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[3] & (1 << 26)) != 0;
}

inline bool CpuSSE3() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 0)) != 0;
}

inline bool CpuSSSE3() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 9)) != 0;
}

inline bool CpuSSE41() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 19)) != 0;
}

inline bool CpuSSE42() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 20)) != 0;
}

inline bool CpuAVX() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 28)) != 0;
}

inline bool CpuAVX2() {
    int cpuInfo[4];
    __cpuidex(cpuInfo, 7, 0);
    return (cpuInfo[1] & (1 << 5)) != 0;
}

#elif defined(__GNUC__) || defined(__clang__)
// GCC or Clang
#include <cpuid.h>

inline bool CpuMMX() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (edx & (1 << 23)) != 0;
    return false;
}

inline bool CpuSSE() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (edx & (1 << 25)) != 0;
    return false;
}

inline bool CpuSSE2() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (edx & (1 << 26)) != 0;
    return false;
}

inline bool CpuSSE3() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (ecx & (1 << 0)) != 0;
    return false;
}

inline bool CpuSSSE3() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (ecx & (1 << 9)) != 0;
    return false;
}

inline bool CpuSSE41() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (ecx & (1 << 19)) != 0;
    return false;
}

inline bool CpuSSE42() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (ecx & (1 << 20)) != 0;
    return false;
}

inline bool CpuAVX() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return (ecx & (1 << 28)) != 0;
    return false;
}

inline bool CpuAVX2() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(7, &eax, &ebx, &ecx, &edx))
        return (ebx & (1 << 5)) != 0;
    return false;
}

#else
// Unknown compiler - assume no SIMD support
inline bool CpuMMX() { return false; }
inline bool CpuSSE() { return false; }
inline bool CpuSSE2() { return false; }
inline bool CpuSSE3() { return false; }
inline bool CpuSSSE3() { return false; }
inline bool CpuSSE41() { return false; }
inline bool CpuSSE42() { return false; }
inline bool CpuAVX() { return false; }
inline bool CpuAVX2() { return false; }
#endif

#else
// Non-x86 architecture - assume no x86 SIMD support
inline bool CpuMMX() { return false; }
inline bool CpuSSE() { return false; }
inline bool CpuSSE2() { return false; }
inline bool CpuSSE3() { return false; }
inline bool CpuSSSE3() { return false; }
inline bool CpuSSE41() { return false; }
inline bool CpuSSE42() { return false; }
inline bool CpuAVX() { return false; }
inline bool CpuAVX2() { return false; }
#endif

// ARM NEON support detection (if needed)
#ifdef CPU_ARM
inline bool CpuNEON() {
#ifdef __ARM_NEON__
    return true;
#else
    return false;
#endif
}
#else
inline bool CpuNEON() { return false; }
#endif

// Cache line size
#ifdef CPU_X86
inline int CpuCacheLineSize() {
    return 64; // Typical for modern x86 processors
}
#else
inline int CpuCacheLineSize() {
    return 64; // Assume 64-byte cache lines as default
}
#endif

// Number of CPU cores
inline int CpuCores() {
#ifdef _MSC_VER
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(__linux__)
    return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__APPLE__)
    int nm[2];
    size_t len = sizeof(nm[0]) * 2;
    sysctlbyname("hw.ncpu", &nm[0], &len, NULL, 0);
    return nm[0];
#else
    return 1; // Default to 1 core if we can't determine
#endif
}

#endif