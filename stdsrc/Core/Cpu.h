#pragma once
#ifndef _Core_Cpu_h_
#define _Core_Cpu_h_

#include "Core.h"

// CPU capabilities and information
class CPU {
public:
    static bool HasMMX();
    static bool HasSSE();
    static bool HasSSE2();
    static bool HasSSE3();
    static bool HasSSSE3();
    static bool HasSSE41();
    static bool HasSSE42();
    static bool HasAVX();
    static bool HasAVX2();
    static bool HasAES();
    static bool HasSHA();
    static bool HasBMI1();
    static bool HasBMI2();
    static bool HasPOPCNT();
    static bool HasLZCNT();
    
    // Get CPU vendor string
    static String GetVendor();
    
    // Get CPU brand string
    static String GetBrand();
    
    // Get number of logical CPU cores
    static int GetLogicalCpuCount();
    
    // Get CPU architecture
    static String GetArchitecture();
    
    // Get cache line size
    static int GetCacheLineSize();
    
private:
    static void Initialize();
    static bool initialized;
    static bool mmx, sse, sse2, sse3, ssse3, sse41, sse42, avx, avx2, aes, sha, bmi1, bmi2, popcnt, lzcnt;
    static String vendor, brand;
    static int logical_cpu_count;
    static String architecture;
    static int cache_line_size;
};

// Cache line alignment
#define CACHE_LINE_SIZE CPU::GetCacheLineSize()

// Alignment macros
#define ALIGN_CACHE_LINE alignas(CACHE_LINE_SIZE)

// Compiler-specific intrinsics for CPU features
#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif

// Cross-platform CPUID implementation
class CPUID {
public:
    static void GetCpuId(int function_id, int cpuinfo[4]);
    static void GetExtendedCpuId(int function_id, int cpuinfo[4]);
};

#endif