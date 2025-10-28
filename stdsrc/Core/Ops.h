#pragma once
#ifndef _Core_Ops_h_
#define _Core_Ops_h_

#include <cstring>
#include <cstdint>
#include <algorithm>
#include "Core.h"

#ifndef CPU_LE
#error Only little endian CPUs supported
#endif

// Use standard library or compiler intrinsics for byte swapping
#if defined(__GNUC__) || defined(__clang__)
    #include <byteswap.h>
    #define BUILTIN_BSWAP16 __builtin_bswap16
    #define BUILTIN_BSWAP32 __builtin_bswap32
    #define BUILTIN_BSWAP64 __builtin_bswap64
#elif defined(_MSC_VER)
    #include <stdlib.h>
    #define BUILTIN_BSWAP16 _byteswap_ushort
    #define BUILTIN_BSWAP32 _byteswap_ulong
    #define BUILTIN_BSWAP64 _byteswap_uint64
#else
    // Generic implementation if compiler intrinsics aren't available
    inline uint16_t BUILTIN_BSWAP16(uint16_t x) { return (x << 8) | (x >> 8); }
    inline uint32_t BUILTIN_BSWAP32(uint32_t x) { 
        return ((x & 0xff000000) >> 24) | 
               ((x & 0x00ff0000) >>  8) | 
               ((x & 0x0000ff00) <<  8) | 
               ((x & 0x000000ff) << 24); 
    }
    inline uint64_t BUILTIN_BSWAP64(uint64_t x) {
        return ((x & 0xff00000000000000ULL) >> 56) |
               ((x & 0x00ff000000000000ULL) >> 40) |
               ((x & 0x0000ff0000000000ULL) >> 24) |
               ((x & 0x000000ff00000000ULL) >>  8) |
               ((x & 0x00000000ff000000ULL) <<  8) |
               ((x & 0x0000000000ff0000ULL) << 24) |
               ((x & 0x000000000000ff00ULL) << 40) |
               ((x & 0x00000000000000ffULL) << 56);
    }
#endif

inline uint16  SwapEndian16(uint16 v)    { return BUILTIN_BSWAP16(v); }
inline int16   SwapEndian16(int16 v)    { return static_cast<int16>(BUILTIN_BSWAP16(static_cast<uint16>(v))); }
inline uint32  SwapEndian32(uint32 v)   { return BUILTIN_BSWAP32(v); }
inline int     SwapEndian32(int v)      { return static_cast<int>(BUILTIN_BSWAP32(static_cast<uint32>(v))); }

inline void    EndianSwap(uint16& v)    { v = SwapEndian16(v); }
inline void    EndianSwap(int16& v)     { v = SwapEndian16(v); }
inline void    EndianSwap(uint32& v)    { v = SwapEndian32(v); }
inline void    EndianSwap(int& v)       { v = SwapEndian32(v); }

#ifdef COMPILER_GCC
inline uint16  SwapEndian16(int w)      { return SwapEndian16(static_cast<uint16>(w)); }
inline uint16  SwapEndian16(uint32 w)   { return SwapEndian16(static_cast<uint16>(w)); }
#endif

// For 64-bit values
inline uint64  SwapEndian64(uint64 v)  { return BUILTIN_BSWAP64(v); }
inline int64   SwapEndian64(int64 v)   { return static_cast<int64>(BUILTIN_BSWAP64(static_cast<uint64>(v))); }

inline void    EndianSwap(int64& v)    { v = SwapEndian64(v); }
inline void    EndianSwap(uint64& v)   { v = SwapEndian64(v); }

inline uint16  SwapEndian16(int w)     { return SwapEndian16(static_cast<uint16>(w)); }
inline uint16  SwapEndian16(uint32 w)  { return SwapEndian16(static_cast<uint16>(w)); }

// Unaligned access - using memcpy for portability
inline int     Peek16(const void *ptr)        { uint16 x; std::memcpy(&x, ptr, 2); return x; }
inline int     Peek32(const void *ptr)        { uint32 x; std::memcpy(&x, ptr, 4); return x; }
inline int64   Peek64(const void *ptr)        { uint64 x; std::memcpy(&x, ptr, 8); return x; }

inline void    Poke16(void *ptr, uint16 val)  { std::memcpy(ptr, &val, 2); }
inline void    Poke32(void *ptr, uint32 val)  { std::memcpy(ptr, &val, 4); }
inline void    Poke64(void *ptr, int64 val)   { std::memcpy(ptr, &val, 8); }

// Little endian - same as native on little endian CPUs
inline int     Peek16le(const void *ptr)      { return Peek16(ptr); }
inline int     Peek32le(const void *ptr)      { return Peek32(ptr); }
inline int64   Peek64le(const void *ptr)      { return Peek64(ptr); }

inline void    Poke16le(void *ptr, uint16 val)   { Poke16(ptr, val); }
inline void    Poke32le(void *ptr, uint32 val)   { Poke32(ptr, val); }
inline void    Poke64le(void *ptr, int64 val)    { Poke64(ptr, val); }

// Big endian - swap on little endian
inline int     Peek16be(const void *ptr)      { return SwapEndian16(Peek16(ptr)); }
inline int     Peek32be(const void *ptr)      { return SwapEndian32(Peek32(ptr)); }
inline int64   Peek64be(const void *ptr)      { return SwapEndian64(Peek64(ptr)); }

inline void    Poke16be(void *ptr, uint16 val)  { Poke16(ptr, SwapEndian16(val)); }
inline void    Poke32be(void *ptr, uint32 val)  { Poke32(ptr, SwapEndian32(val)); }
inline void    Poke64be(void *ptr, int64 val)   { Poke64(ptr, SwapEndian64(val)); }

#define MAKE2B(b0, b1)                            MAKEWORD(b0, b1)
#define MAKE4B(b0, b1, b2, b3)                    MAKELONG(MAKEWORD(b0, b1), MAKEWORD(b2, b3))
#define MAKE8B(b0, b1, b2, b3, b4, b5, b6, b7)    MAKEQWORD(MAKE4B(b0, b1, b2, b3), MAKE4B(b4, b5, b6, b7))

#ifdef CPU_64
    #define HASH64
    #define HASH_CONST1 static_cast<uint64>(0xf7c21089bee7c0a5ULL)
    #define HASH_CONST2 static_cast<uint64>(0xc85abc8da7534a4dULL)
    #define HASH_CONST3 static_cast<uint64>(0x8642b0fe3e86671bULL)
    typedef uint64 hash_t;
    
    inline uint32  FoldHash(uint64 h) {
        return static_cast<uint32>(SwapEndian64(HASH_CONST3 * h));
    }
#else
    #define HASH_CONST1 0xbee7c0a5UL
    #define HASH_CONST2 0xa7534a4dUL
    #define HASH_CONST3 0x8e86671bUL
    typedef uint32 hash_t;
    
    inline uint32  FoldHash(uint32 h) {
        return SwapEndian32(HASH_CONST3 * h);
    }
#endif

inline uint32  FoldHash32(uint32 h) {
    return SwapEndian32(0x8e86671bUL * h);
}

inline byte    Saturate255(int x) { 
    return static_cast<byte>(~(x >> 24) & (x | (-(x >> 8) >> 24)) & 0xff); 
}

inline int     SignificantBits(uint32 x) {
    // Count leading zeros and subtract from bit width
#ifdef _MSC_VER
    if (x == 0) return 0;
    unsigned long index;
    _BitScanReverse(&index, x);
    return static_cast<int>(index) + 1;
#elif defined(__GNUC__) || defined(__clang__)
    return x ? 32 - __builtin_clz(x) : 0;
#else
    // Generic fallback
    if (x == 0) return 0;
    int count = 0;
    while (x) {
        count++;
        x >>= 1;
    }
    return count;
#endif
}

inline int     SignificantBits64(uint64 x) {
#ifdef _MSC_VER
    #ifdef CPU_64
        if (x == 0) return 0;
        unsigned long index;
        _BitScanReverse64(&index, x);
        return static_cast<int>(index) + 1;
    #else
        if (x & 0xffffffff00000000ULL)
            return SignificantBits(static_cast<uint32>(x >> 32)) + 32;
        else
            return SignificantBits(static_cast<uint32>(x & 0x00000000ffffffffULL));
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    return x ? 64 - __builtin_clzll(x) : 0;
#else
    // Generic fallback
    if (x == 0) return 0;
    int count = 0;
    while (x) {
        count++;
        x >>= 1;
    }
    return count;
#endif
}

inline bool    FitsInInt64(double x) {
    return x >= -9223372036854775808.0 && x < 9223372036854775808.0;
}

inline int     CountBits(uint32 mask) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount(mask);
#elif defined(_MSC_VER)
    #ifdef CPU_64
        return __popcnt(mask);
    #else
        return __popcnt(mask);
    #endif
#else
    // Fallback implementation
    mask = mask - ((mask >> 1) & 0x55555555);
    mask = (mask & 0x33333333) + ((mask >> 2) & 0x33333333);
    mask = (mask + (mask >> 4)) & 0x0F0F0F0F;
    mask = mask + (mask >> 8);
    mask = mask + (mask >> 16);
    return mask & 0x3F;
#endif
}

inline int     CountBits64(uint64 mask) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(mask);
#elif defined(_MSC_VER) && defined(CPU_64)
    return static_cast<int>(__popcnt64(mask));
#elif defined(_MSC_VER)
    return CountBits(static_cast<uint32>(mask)) + CountBits(static_cast<uint32>(mask >> 32));
#else
    // Fallback implementation
    mask = mask - ((mask >> 1) & 0x5555555555555555ULL);
    mask = (mask & 0x3333333333333333ULL) + ((mask >> 2) & 0x3333333333333333ULL);
    mask = (mask + (mask >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    mask = mask + (mask >> 8);
    mask = mask + (mask >> 16);
    mask = mask + (mask >> 32);
    return static_cast<int>(mask & 0x7F);
#endif
}

inline int     CountTrailingZeroBits(uint32 x) {
#if defined(__GNUC__) || defined(__clang__)
    if (x == 0) return 32;  // or undefined, depending on implementation
    return __builtin_ctz(x);
#elif defined(_MSC_VER)
    if (x == 0) return 32;
    unsigned long index;
    _BitScanForward(&index, x);
    return static_cast<int>(index);
#else
    // Fallback implementation
    if (x == 0) return 32;
    int count = 0;
    while ((x & 1) == 0) {
        count++;
        x >>= 1;
    }
    return count;
#endif
}

inline int     CountTrailingZeroBits64(uint64 x) {
#if defined(__GNUC__) || defined(__clang__)
    if (x == 0) return 64;
    return __builtin_ctzll(x);
#elif defined(_MSC_VER) && defined(CPU_64)
    if (x == 0) return 64;
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
#else
    if (x == 0) return 64;
    return (x & 0xffffffff) ? CountTrailingZeroBits(static_cast<uint32>(x)) 
                            : CountTrailingZeroBits(static_cast<uint32>(x >> 32)) + 32;
#endif
}

// Helper functions for 64-bit arithmetic operations
inline uint8   addc64(uint64& r, uint64 a, uint8 carry) {
    r += a + carry;
    return carry ? r <= a : r < a;
}

inline uint64  mul64(uint64 a, uint64 b, uint64& hi) {
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
    // Use 128-bit integers if available
    __uint128_t prod = static_cast<__uint128_t>(a) * b;
    hi = static_cast<uint64>(prod >> 64);
    return static_cast<uint64>(prod);
#elif defined(_MSC_VER) && defined(_M_X64)
    // Use MSVC intrinsic
    return _umul128(a, b, &hi);
#else
    // Generic implementation
    uint64 lo_lo = (a & 0xFFFFFFFF) * (b & 0xFFFFFFFF);
    uint64 hi_lo = (a >> 32)        * (b & 0xFFFFFFFF);
    uint64 lo_hi = (a & 0xFFFFFFFF) * (b >> 32);
    uint64 hi_hi = (a >> 32)        * (b >> 32);
    
    uint64 cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
    hi = (hi_lo >> 32) + (cross >> 32)        + hi_hi;

    return (cross << 32) | (lo_lo & 0xFFFFFFFF);
#endif
}

// Function declarations for EndianSwap overloads
void EndianSwap(uint16 *v, size_t count);
void EndianSwap(int16 *v, size_t count);
void EndianSwap(uint32 *v, size_t count);
void EndianSwap(int *v, size_t count);
void EndianSwap(int64 *v, size_t count);
void EndianSwap(uint64 *v, size_t count);

#endif