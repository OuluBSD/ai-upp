#pragma once
#ifndef _Draw_SIMD_h_
#define _Draw_SIMD_h_

#include "Draw.h"
#include <cstdint>
#include <cstring>

// SIMD (Single Instruction, Multiple Data) utilities for Draw module
// Provides optimized vector operations for graphics processing

#ifdef __SSE2__
#include <emmintrin.h>
using simd_int32 = __m128i;
using simd_float = __m128;
using simd_double = __m128d;
#elif defined(__ARM_NEON__)
#include <arm_neon.h>
using simd_int32 = int32x4_t;
using simd_float = float32x4_t;
using simd_double = float64x2_t;
#else
// Fallback scalar implementation
struct simd_int32 {
    int32_t data[4];
    simd_int32() = default;
    simd_int32(int32_t a, int32_t b, int32_t c, int32_t d) {
        data[0] = a; data[1] = b; data[2] = c; data[3] = d;
    }
};

struct simd_float {
    float data[4];
    simd_float() = default;
    simd_float(float a, float b, float c, float d) {
        data[0] = a; data[1] = b; data[2] = c; data[3] = d;
    }
};

struct simd_double {
    double data[2];
    simd_double() = default;
    simd_double(double a, double b) {
        data[0] = a; data[1] = b;
    }
};
#endif

// SIMD operations for Draw module
namespace SIMD {

// Load operations
inline simd_int32 LoadInt32(const int32_t* ptr) {
#ifdef __SSE2__
    return _mm_load_si128(reinterpret_cast<const __m128i*>(ptr));
#elif defined(__ARM_NEON__)
    return vld1q_s32(ptr);
#else
    return simd_int32(ptr[0], ptr[1], ptr[2], ptr[3]);
#endif
}

inline simd_float LoadFloat(const float* ptr) {
#ifdef __SSE2__
    return _mm_load_ps(ptr);
#elif defined(__ARM_NEON__)
    return vld1q_f32(ptr);
#else
    return simd_float(ptr[0], ptr[1], ptr[2], ptr[3]);
#endif
}

inline simd_double LoadDouble(const double* ptr) {
#ifdef __SSE2__
    return _mm_load_pd(ptr);
#elif defined(__ARM_NEON__)
    return vld1q_f64(ptr);
#else
    return simd_double(ptr[0], ptr[1]);
#endif
}

// Store operations
inline void StoreInt32(int32_t* ptr, const simd_int32& v) {
#ifdef __SSE2__
    _mm_store_si128(reinterpret_cast<__m128i*>(ptr), v);
#elif defined(__ARM_NEON__)
    vst1q_s32(ptr, v);
#else
    ptr[0] = v.data[0]; ptr[1] = v.data[1]; ptr[2] = v.data[2]; ptr[3] = v.data[3];
#endif
}

inline void StoreFloat(float* ptr, const simd_float& v) {
#ifdef __SSE2__
    _mm_store_ps(ptr, v);
#elif defined(__ARM_NEON__)
    vst1q_f32(ptr, v);
#else
    ptr[0] = v.data[0]; ptr[1] = v.data[1]; ptr[2] = v.data[2]; ptr[3] = v.data[3];
#endif
}

inline void StoreDouble(double* ptr, const simd_double& v) {
#ifdef __SSE2__
    _mm_store_pd(ptr, v);
#elif defined(__ARM_NEON__)
    vst1q_f64(ptr, v);
#else
    ptr[0] = v.data[0]; ptr[1] = v.data[1];
#endif
}

// Arithmetic operations
inline simd_int32 AddInt32(const simd_int32& a, const simd_int32& b) {
#ifdef __SSE2__
    return _mm_add_epi32(a, b);
#elif defined(__ARM_NEON__)
    return vaddq_s32(a, b);
#else
    return simd_int32(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1],
        a.data[2] + b.data[2],
        a.data[3] + b.data[3]
    );
#endif
}

inline simd_float AddFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_add_ps(a, b);
#elif defined(__ARM_NEON__)
    return vaddq_f32(a, b);
#else
    return simd_float(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1],
        a.data[2] + b.data[2],
        a.data[3] + b.data[3]
    );
#endif
}

inline simd_double AddDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_add_pd(a, b);
#elif defined(__ARM_NEON__)
    return vaddq_f64(a, b);
#else
    return simd_double(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1]
    );
#endif
}

inline simd_int32 SubInt32(const simd_int32& a, const simd_int32& b) {
#ifdef __SSE2__
    return _mm_sub_epi32(a, b);
#elif defined(__ARM_NEON__)
    return vsubq_s32(a, b);
#else
    return simd_int32(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1],
        a.data[2] - b.data[2],
        a.data[3] - b.data[3]
    );
#endif
}

inline simd_float SubFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_sub_ps(a, b);
#elif defined(__ARM_NEON__)
    return vsubq_f32(a, b);
#else
    return simd_float(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1],
        a.data[2] - b.data[2],
        a.data[3] - b.data[3]
    );
#endif
}

inline simd_double SubDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_sub_pd(a, b);
#elif defined(__ARM_NEON__)
    return vsubq_f64(a, b);
#else
    return simd_double(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1]
    );
#endif
}

inline simd_int32 MulInt32(const simd_int32& a, const simd_int32& b) {
#ifdef __SSE4_1__
    return _mm_mullo_epi32(a, b);
#elif defined(__ARM_NEON__)
    return vmulq_s32(a, b);
#else
    return simd_int32(
        a.data[0] * b.data[0],
        a.data[1] * b.data[1],
        a.data[2] * b.data[2],
        a.data[3] * b.data[3]
    );
#endif
}

inline simd_float MulFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_mul_ps(a, b);
#elif defined(__ARM_NEON__)
    return vmulq_f32(a, b);
#else
    return simd_float(
        a.data[0] * b.data[0],
        a.data[1] * b.data[1],
        a.data[2] * b.data[2],
        a.data[3] * b.data[3]
    );
#endif
}

inline simd_double MulDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_mul_pd(a, b);
#elif defined(__ARM_NEON__)
    return vmulq_f64(a, b);
#else
    return simd_double(
        a.data[0] * b.data[0],
        a.data[1] * b.data[1]
    );
#endif
}

// Comparison operations
inline simd_int32 CompareEqualInt32(const simd_int32& a, const simd_int32& b) {
#ifdef __SSE2__
    return _mm_cmpeq_epi32(a, b);
#elif defined(__ARM_NEON__)
    return vreinterpretq_s32_u32(vceqq_s32(a, b));
#else
    return simd_int32(
        a.data[0] == b.data[0] ? -1 : 0,
        a.data[1] == b.data[1] ? -1 : 0,
        a.data[2] == b.data[2] ? -1 : 0,
        a.data[3] == b.data[3] ? -1 : 0
    );
#endif
}

inline simd_float CompareEqualFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_cmpeq_ps(a, b);
#elif defined(__ARM_NEON__)
    return vreinterpretq_f32_u32(vceqq_f32(a, b));
#else
    return simd_float(
        a.data[0] == b.data[0] ? 1.0f : 0.0f,
        a.data[1] == b.data[1] ? 1.0f : 0.0f,
        a.data[2] == b.data[2] ? 1.0f : 0.0f,
        a.data[3] == b.data[3] ? 1.0f : 0.0f
    );
#endif
}

inline simd_double CompareEqualDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_cmpeq_pd(a, b);
#elif defined(__ARM_NEON__)
    return vreinterpretq_f64_u64(vceqq_f64(a, b));
#else
    return simd_double(
        a.data[0] == b.data[0] ? 1.0 : 0.0,
        a.data[1] == b.data[1] ? 1.0 : 0.0
    );
#endif
}

// Horizontal operations (reduce across vector elements)
inline int32_t HorizontalAddInt32(const simd_int32& v) {
#ifdef __SSE2__
    __m128i sum = _mm_add_epi32(v, _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1)));
    sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_SHUFFLE(1, 0, 3, 2)));
    return _mm_cvtsi128_si32(sum);
#elif defined(__ARM_NEON__)
    return vaddvq_s32(v);
#else
    return v.data[0] + v.data[1] + v.data[2] + v.data[3];
#endif
}

inline float HorizontalAddFloat(const simd_float& v) {
#ifdef __SSE2__
    __m128 sum = _mm_add_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    sum = _mm_add_ps(sum, _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(1, 0, 3, 2)));
    return _mm_cvtss_f32(sum);
#elif defined(__ARM_NEON__)
    return vaddvq_f32(v);
#else
    return v.data[0] + v.data[1] + v.data[2] + v.data[3];
#endif
}

inline double HorizontalAddDouble(const simd_double& v) {
#ifdef __SSE2__
    __m128d sum = _mm_add_pd(v, _mm_shuffle_pd(v, v, 1));
    return _mm_cvtsd_f64(sum);
#elif defined(__ARM_NEON__)
    return vaddvq_f64(v);
#else
    return v.data[0] + v.data[1];
#endif
}

// Set operations (create vectors with specific values)
inline simd_int32 SetInt32(int32_t a, int32_t b, int32_t c, int32_t d) {
#ifdef __SSE2__
    return _mm_set_epi32(d, c, b, a);
#elif defined(__ARM_NEON__)
    return (simd_int32){a, b, c, d};
#else
    return simd_int32(a, b, c, d);
#endif
}

inline simd_float SetFloat(float a, float b, float c, float d) {
#ifdef __SSE2__
    return _mm_set_ps(d, c, b, a);
#elif defined(__ARM_NEON__)
    return (simd_float){a, b, c, d};
#else
    return simd_float(a, b, c, d);
#endif
}

inline simd_double SetDouble(double a, double b) {
#ifdef __SSE2__
    return _mm_set_pd(b, a);
#elif defined(__ARM_NEON__)
    return (simd_double){a, b};
#else
    return simd_double(a, b);
#endif
}

// Zero operations
inline simd_int32 ZeroInt32() {
#ifdef __SSE2__
    return _mm_setzero_si128();
#elif defined(__ARM_NEON__)
    return vdupq_n_s32(0);
#else
    return simd_int32(0, 0, 0, 0);
#endif
}

inline simd_float ZeroFloat() {
#ifdef __SSE2__
    return _mm_setzero_ps();
#elif defined(__ARM_NEON__)
    return vdupq_n_f32(0.0f);
#else
    return simd_float(0.0f, 0.0f, 0.0f, 0.0f);
#endif
}

inline simd_double ZeroDouble() {
#ifdef __SSE2__
    return _mm_setzero_pd();
#elif defined(__ARM_NEON__)
    return vdupq_n_f64(0.0);
#else
    return simd_double(0.0, 0.0);
#endif
}

// Utility functions
template<typename T>
inline T Min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
inline T Max(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
inline T Clamp(const T& x, const T& min_val, const T& max_val) {
    return Min(Max(x, min_val), max_val);
}

// SIMD-enabled mathematical functions for graphics
inline simd_float SqrtFloat(const simd_float& v) {
#ifdef __SSE2__
    return _mm_sqrt_ps(v);
#elif defined(__ARM_NEON__)
    return vsqrtq_f32(v);
#else
    return simd_float(
        sqrtf(v.data[0]),
        sqrtf(v.data[1]),
        sqrtf(v.data[2]),
        sqrtf(v.data[3])
    );
#endif
}

inline simd_double SqrtDouble(const simd_double& v) {
#ifdef __SSE2__
    return _mm_sqrt_pd(v);
#elif defined(__ARM_NEON__)
    // NEON doesn't have direct double sqrt, approximate or use scalar
    return simd_double(sqrt(v.data[0]), sqrt(v.data[1]));
#else
    return simd_double(
        sqrt(v.data[0]),
        sqrt(v.data[1])
    );
#endif
}

// SIMD-enabled trigonometric functions (approximate implementations)
inline simd_float SinFloat(const simd_float& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_float(
        sinf(v.data[0]),
        sinf(v.data[1]),
        sinf(v.data[2]),
        sinf(v.data[3])
    );
}

inline simd_float CosFloat(const simd_float& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_float(
        cosf(v.data[0]),
        cosf(v.data[1]),
        cosf(v.data[2]),
        cosf(v.data[3])
    );
}

inline simd_double SinDouble(const simd_double& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_double(
        sin(v.data[0]),
        sin(v.data[1])
    );
}

inline simd_double CosDouble(const simd_double& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_double(
        cos(v.data[0]),
        cos(v.data[1])
    );
}

// SIMD-enabled exponential functions
inline simd_float ExpFloat(const simd_float& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_float(
        expf(v.data[0]),
        expf(v.data[1]),
        expf(v.data[2]),
        expf(v.data[3])
    );
}

inline simd_double ExpDouble(const simd_double& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_double(
        exp(v.data[0]),
        exp(v.data[1])
    );
}

// SIMD-enabled logarithmic functions
inline simd_float LogFloat(const simd_float& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_float(
        logf(v.data[0]),
        logf(v.data[1]),
        logf(v.data[2]),
        logf(v.data[3])
    );
}

inline simd_double LogDouble(const simd_double& v) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_double(
        log(v.data[0]),
        log(v.data[1])
    );
}

// SIMD-enabled power functions
inline simd_float PowFloat(const simd_float& base, const simd_float& exp) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_float(
        powf(base.data[0], exp.data[0]),
        powf(base.data[1], exp.data[1]),
        powf(base.data[2], exp.data[2]),
        powf(base.data[3], exp.data[3])
    );
}

inline simd_double PowDouble(const simd_double& base, const simd_double& exp) {
    // Simple approximation - for production use, consider more accurate implementations
    return simd_double(
        pow(base.data[0], exp.data[0]),
        pow(base.data[1], exp.data[1])
    );
}

// SIMD-enabled absolute value functions
inline simd_int32 AbsInt32(const simd_int32& v) {
#ifdef __SSSE3__
    return _mm_abs_epi32(v);
#elif defined(__ARM_NEON__)
    return vabsq_s32(v);
#else
    return simd_int32(
        abs(v.data[0]),
        abs(v.data[1]),
        abs(v.data[2]),
        abs(v.data[3])
    );
#endif
}

inline simd_float AbsFloat(const simd_float& v) {
#ifdef __SSE2__
    __m128 mask = _mm_set1_ps(-0.0f);
    return _mm_andnot_ps(mask, v);
#elif defined(__ARM_NEON__)
    return vabsq_f32(v);
#else
    return simd_float(
        fabsf(v.data[0]),
        fabsf(v.data[1]),
        fabsf(v.data[2]),
        fabsf(v.data[3])
    );
#endif
}

inline simd_double AbsDouble(const simd_double& v) {
#ifdef __SSE2__
    __m128d mask = _mm_set1_pd(-0.0);
    return _mm_andnot_pd(mask, v);
#elif defined(__ARM_NEON__)
    return vabsq_f64(v);
#else
    return simd_double(
        fabs(v.data[0]),
        fabs(v.data[1])
    );
#endif
}

// SIMD-enabled minimum/maximum functions
inline simd_float MinFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_min_ps(a, b);
#elif defined(__ARM_NEON__)
    return vminq_f32(a, b);
#else
    return simd_float(
        Min(a.data[0], b.data[0]),
        Min(a.data[1], b.data[1]),
        Min(a.data[2], b.data[2]),
        Min(a.data[3], b.data[3])
    );
#endif
}

inline simd_double MinDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_min_pd(a, b);
#elif defined(__ARM_NEON__)
    return vminq_f64(a, b);
#else
    return simd_double(
        Min(a.data[0], b.data[0]),
        Min(a.data[1], b.data[1])
    );
#endif
}

inline simd_float MaxFloat(const simd_float& a, const simd_float& b) {
#ifdef __SSE2__
    return _mm_max_ps(a, b);
#elif defined(__ARM_NEON__)
    return vmaxq_f32(a, b);
#else
    return simd_float(
        Max(a.data[0], b.data[0]),
        Max(a.data[1], b.data[1]),
        Max(a.data[2], b.data[2]),
        Max(a.data[3], b.data[3])
    );
#endif
}

inline simd_double MaxDouble(const simd_double& a, const simd_double& b) {
#ifdef __SSE2__
    return _mm_max_pd(a, b);
#elif defined(__ARM_NEON__)
    return vmaxq_f64(a, b);
#else
    return simd_double(
        Max(a.data[0], b.data[0]),
        Max(a.data[1], b.data[1])
    );
#endif
}

// SIMD-enabled blend functions
inline simd_float BlendFloat(const simd_float& a, const simd_float& b, const simd_float& mask) {
#ifdef __SSE4_1__
    return _mm_blendv_ps(a, b, mask);
#elif defined(__ARM_NEON__)
    uint32x4_t mask_bits = vreinterpretq_u32_f32(mask);
    return vreinterpretq_f32_u32(vbslq_u32(mask_bits, vreinterpretq_u32_f32(b), vreinterpretq_u32_f32(a)));
#else
    return simd_float(
        mask.data[0] != 0.0f ? b.data[0] : a.data[0],
        mask.data[1] != 0.0f ? b.data[1] : a.data[1],
        mask.data[2] != 0.0f ? b.data[2] : a.data[2],
        mask.data[3] != 0.0f ? b.data[3] : a.data[3]
    );
#endif
}

inline simd_double BlendDouble(const simd_double& a, const simd_double& b, const simd_double& mask) {
#ifdef __SSE4_1__
    return _mm_blendv_pd(a, b, mask);
#elif defined(__ARM_NEON__)
    uint64x2_t mask_bits = vreinterpretq_u64_f64(mask);
    return vreinterpretq_f64_u64(vbslq_u64(mask_bits, vreinterpretq_u64_f64(b), vreinterpretq_u64_f64(a)));
#else
    return simd_double(
        mask.data[0] != 0.0 ? b.data[0] : a.data[0],
        mask.data[1] != 0.0 ? b.data[1] : a.data[1]
    );
#endif
}

// SIMD-enabled shuffle functions
inline simd_float ShuffleFloat(const simd_float& a, const simd_float& b, int imm8) {
#ifdef __SSE2__
    return _mm_shuffle_ps(a, b, imm8);
#else
    // Simple fallback implementation
    simd_float result;
    const float* pa = reinterpret_cast<const float*>(&a);
    const float* pb = reinterpret_cast<const float*>(&b);
    float* pr = reinterpret_cast<float*>(&result);
    
    for (int i = 0; i < 4; ++i) {
        int selector = (imm8 >> (i * 2)) & 0x3;
        if (selector < 2) {
            pr[i] = pa[selector];
        } else {
            pr[i] = pb[selector - 2];
        }
    }
    return result;
#endif
}

inline simd_double ShuffleDouble(const simd_double& a, const simd_double& b, int imm8) {
#ifdef __SSE2__
    return _mm_shuffle_pd(a, b, imm8);
#else
    // Simple fallback implementation
    simd_double result;
    const double* pa = reinterpret_cast<const double*>(&a);
    const double* pb = reinterpret_cast<const double*>(&b);
    double* pr = reinterpret_cast<double*>(&result);
    
    pr[0] = (imm8 & 0x1) ? pb[0] : pa[0];
    pr[1] = (imm8 & 0x2) ? pb[1] : pa[1];
    return result;
#endif
}

// SIMD-enabled broadcast functions
inline simd_float BroadcastFloat(float a) {
#ifdef __SSE2__
    return _mm_set1_ps(a);
#elif defined(__ARM_NEON__)
    return vdupq_n_f32(a);
#else
    return simd_float(a, a, a, a);
#endif
}

inline simd_double BroadcastDouble(double a) {
#ifdef __SSE2__
    return _mm_set1_pd(a);
#elif defined(__ARM_NEON__)
    return vdupq_n_f64(a);
#else
    return simd_double(a, a);
#endif
}

inline simd_int32 BroadcastInt32(int32_t a) {
#ifdef __SSE2__
    return _mm_set1_epi32(a);
#elif defined(__ARM_NEON__)
    return vdupq_n_s32(a);
#else
    return simd_int32(a, a, a, a);
#endif
}

// Detection functions
inline bool IsSSE2Supported() {
#ifdef __SSE2__
    return true;
#else
    return false;
#endif
}

inline bool IsSSE41Supported() {
#ifdef __SSE4_1__
    return true;
#else
    return false;
#endif
}

inline bool IsAVXSupported() {
#ifdef __AVX__
    return true;
#else
    return false;
#endif
}

inline bool IsNEONSupported() {
#ifdef __ARM_NEON__
    return true;
#else
    return false;
#endif
}

// Graphics-specific SIMD operations
namespace Graphics {

// Color operations
inline simd_int32 PackRGBA(const simd_int32& r, const simd_int32& g, const simd_int32& b, const simd_int32& a) {
#ifdef __SSE2__
    // Pack 4 RGBA values into 4 32-bit integers
    __m128i r_shifted = _mm_slli_epi32(r, 24);
    __m128i g_shifted = _mm_slli_epi32(g, 16);
    __m128i b_shifted = _mm_slli_epi32(b, 8);
    return _mm_or_si128(_mm_or_si128(_mm_or_si128(r_shifted, g_shifted), b_shifted), a);
#elif defined(__ARM_NEON__)
    // Pack 4 RGBA values into 4 32-bit integers
    uint32x4_t r_shifted = vshlq_n_u32(vreinterpretq_u32_s32(r), 24);
    uint32x4_t g_shifted = vshlq_n_u32(vreinterpretq_u32_s32(g), 16);
    uint32x4_t b_shifted = vshlq_n_u32(vreinterpretq_u32_s32(b), 8);
    uint32x4_t r_g = vorrq_u32(r_shifted, g_shifted);
    uint32x4_t b_a = vorrq_u32(b_shifted, vreinterpretq_u32_s32(a));
    return vreinterpretq_s32_u32(vorrq_u32(r_g, b_a));
#else
    return simd_int32(
        (r.data[0] << 24) | (g.data[0] << 16) | (b.data[0] << 8) | a.data[0],
        (r.data[1] << 24) | (g.data[1] << 16) | (b.data[1] << 8) | a.data[1],
        (r.data[2] << 24) | (g.data[2] << 16) | (b.data[2] << 8) | a.data[2],
        (r.data[3] << 24) | (g.data[3] << 16) | (b.data[3] << 8) | a.data[3]
    );
#endif
}

inline void UnpackRGBA(const simd_int32& rgba, simd_int32& r, simd_int32& g, simd_int32& b, simd_int32& a) {
#ifdef __SSE2__
    // Unpack 4 32-bit integers into RGBA components
    r = _mm_srli_epi32(_mm_slli_epi32(rgba, 0), 24);
    g = _mm_srli_epi32(_mm_slli_epi32(rgba, 8), 24);
    b = _mm_srli_epi32(_mm_slli_epi32(rgba, 16), 24);
    a = _mm_srli_epi32(_mm_slli_epi32(rgba, 24), 24);
#elif defined(__ARM_NEON__)
    // Unpack 4 32-bit integers into RGBA components
    uint32x4_t urgba = vreinterpretq_u32_s32(rgba);
    r = vreinterpretq_s32_u32(vshrq_n_u32(vshlq_n_u32(urgba, 0), 24));
    g = vreinterpretq_s32_u32(vshrq_n_u32(vshlq_n_u32(urgba, 8), 24));
    b = vreinterpretq_s32_u32(vshrq_n_u32(vshlq_n_u32(urgba, 16), 24));
    a = vreinterpretq_s32_u32(vshrq_n_u32(vshlq_n_u32(urgba, 24), 24));
#else
    r = simd_int32(
        (rgba.data[0] >> 24) & 0xFF,
        (rgba.data[1] >> 24) & 0xFF,
        (rgba.data[2] >> 24) & 0xFF,
        (rgba.data[3] >> 24) & 0xFF
    );
    g = simd_int32(
        (rgba.data[0] >> 16) & 0xFF,
        (rgba.data[1] >> 16) & 0xFF,
        (rgba.data[2] >> 16) & 0xFF,
        (rgba.data[3] >> 16) & 0xFF
    );
    b = simd_int32(
        (rgba.data[0] >> 8) & 0xFF,
        (rgba.data[1] >> 8) & 0xFF,
        (rgba.data[2] >> 8) & 0xFF,
        (rgba.data[3] >> 8) & 0xFF
    );
    a = simd_int32(
        rgba.data[0] & 0xFF,
        rgba.data[1] & 0xFF,
        rgba.data[2] & 0xFF,
        rgba.data[3] & 0xFF
    );
#endif
}

// Blend operations for graphics
inline simd_int32 AlphaBlend(const simd_int32& src, const simd_int32& dst, const simd_int32& src_alpha) {
    // Simple alpha blending: result = src * α + dst * (1 - α)
    // This is a simplified implementation - real graphics blending is more complex
    simd_int32 inv_alpha;
#ifdef __SSE2__
    inv_alpha = _mm_sub_epi32(BroadcastInt32(255), src_alpha);
#elif defined(__ARM_NEON__)
    inv_alpha = vsubq_s32(BroadcastInt32(255), src_alpha);
#else
    inv_alpha = simd_int32(
        255 - src_alpha.data[0],
        255 - src_alpha.data[1],
        255 - src_alpha.data[2],
        255 - src_alpha.data[3]
    );
#endif
    
    simd_int32 src_r, src_g, src_b, src_a;
    simd_int32 dst_r, dst_g, dst_b, dst_a;
    
    UnpackRGBA(src, src_r, src_g, src_b, src_a);
    UnpackRGBA(dst, dst_r, dst_g, dst_b, dst_a);
    
    // Simple blending formula: result = src * α + dst * (1 - α)
    simd_int32 result_r, result_g, result_b, result_a;
    
#ifdef __SSE2__
    result_r = _mm_add_epi32(
        _mm_mullo_epi32(src_r, src_alpha),
        _mm_mullo_epi32(dst_r, inv_alpha)
    );
    result_g = _mm_add_epi32(
        _mm_mullo_epi32(src_g, src_alpha),
        _mm_mullo_epi32(dst_g, inv_alpha)
    );
    result_b = _mm_add_epi32(
        _mm_mullo_epi32(src_b, src_alpha),
        _mm_mullo_epi32(dst_b, inv_alpha)
    );
    result_a = _mm_add_epi32(
        _mm_mullo_epi32(src_a, src_alpha),
        _mm_mullo_epi32(dst_a, inv_alpha)
    );
#elif defined(__ARM_NEON__)
    result_r = vaddq_s32(
        vmulq_s32(src_r, src_alpha),
        vmulq_s32(dst_r, inv_alpha)
    );
    result_g = vaddq_s32(
        vmulq_s32(src_g, src_alpha),
        vmulq_s32(dst_g, inv_alpha)
    );
    result_b = vaddq_s32(
        vmulq_s32(src_b, src_alpha),
        vmulq_s32(dst_b, inv_alpha)
    );
    result_a = vaddq_s32(
        vmulq_s32(src_a, src_alpha),
        vmulq_s32(dst_a, inv_alpha)
    );
#else
    result_r = simd_int32(
        (src_r.data[0] * src_alpha.data[0] + dst_r.data[0] * inv_alpha.data[0]) / 255,
        (src_r.data[1] * src_alpha.data[1] + dst_r.data[1] * inv_alpha.data[1]) / 255,
        (src_r.data[2] * src_alpha.data[2] + dst_r.data[2] * inv_alpha.data[2]) / 255,
        (src_r.data[3] * src_alpha.data[3] + dst_r.data[3] * inv_alpha.data[3]) / 255
    );
    result_g = simd_int32(
        (src_g.data[0] * src_alpha.data[0] + dst_g.data[0] * inv_alpha.data[0]) / 255,
        (src_g.data[1] * src_alpha.data[1] + dst_g.data[1] * inv_alpha.data[1]) / 255,
        (src_g.data[2] * src_alpha.data[2] + dst_g.data[2] * inv_alpha.data[2]) / 255,
        (src_g.data[3] * src_alpha.data[3] + dst_g.data[3] * inv_alpha.data[3]) / 255
    );
    result_b = simd_int32(
        (src_b.data[0] * src_alpha.data[0] + dst_b.data[0] * inv_alpha.data[0]) / 255,
        (src_b.data[1] * src_alpha.data[1] + dst_b.data[1] * inv_alpha.data[1]) / 255,
        (src_b.data[2] * src_alpha.data[2] + dst_b.data[2] * inv_alpha.data[2]) / 255,
        (src_b.data[3] * src_alpha.data[3] + dst_b.data[3] * inv_alpha.data[3]) / 255
    );
    result_a = simd_int32(
        (src_a.data[0] * src_alpha.data[0] + dst_a.data[0] * inv_alpha.data[0]) / 255,
        (src_a.data[1] * src_alpha.data[1] + dst_a.data[1] * inv_alpha.data[1]) / 255,
        (src_a.data[2] * src_alpha.data[2] + dst_a.data[2] * inv_alpha.data[2]) / 255,
        (src_a.data[3] * src_alpha.data[3] + dst_a.data[3] * inv_alpha.data[3]) / 255
    );
#endif
    
    return PackRGBA(result_r, result_g, result_b, result_a);
}

// Brightness adjustment
inline simd_int32 AdjustBrightness(const simd_int32& color, const simd_int32& brightness) {
    simd_int32 r, g, b, a;
    UnpackRGBA(color, r, g, b, a);
    
#ifdef __SSE2__
    r = _mm_add_epi32(r, brightness);
    g = _mm_add_epi32(g, brightness);
    b = _mm_add_epi32(b, brightness);
    
    // Clamp to [0, 255]
    r = _mm_max_epi32(_mm_min_epi32(r, BroadcastInt32(255)), BroadcastInt32(0));
    g = _mm_max_epi32(_mm_min_epi32(g, BroadcastInt32(255)), BroadcastInt32(0));
    b = _mm_max_epi32(_mm_min_epi32(b, BroadcastInt32(255)), BroadcastInt32(0));
#elif defined(__ARM_NEON__)
    r = vaddq_s32(r, brightness);
    g = vaddq_s32(g, brightness);
    b = vaddq_s32(b, brightness);
    
    // Clamp to [0, 255]
    r = vmaxq_s32(vminq_s32(r, BroadcastInt32(255)), BroadcastInt32(0));
    g = vmaxq_s32(vminq_s32(g, BroadcastInt32(255)), BroadcastInt32(0));
    b = vmaxq_s32(vminq_s32(b, BroadcastInt32(255)), BroadcastInt32(0));
#else
    for (int i = 0; i < 4; ++i) {
        r.data[i] = Clamp(r.data[i] + brightness.data[i], 0, 255);
        g.data[i] = Clamp(g.data[i] + brightness.data[i], 0, 255);
        b.data[i] = Clamp(b.data[i] + brightness.data[i], 0, 255);
    }
#endif
    
    return PackRGBA(r, g, b, a);
}

// Contrast adjustment
inline simd_int32 AdjustContrast(const simd_int32& color, const simd_float& contrast) {
    simd_int32 r, g, b, a;
    UnpackRGBA(color, r, g, b, a);
    
    // Convert to float for calculation
    simd_float rf, gf, bf;
#ifdef __SSE2__
    rf = _mm_cvtepi32_ps(r);
    gf = _mm_cvtepi32_ps(g);
    bf = _mm_cvtepi32_ps(b);
#elif defined(__ARM_NEON__)
    rf = vcvtq_f32_s32(r);
    gf = vcvtq_f32_s32(g);
    bf = vcvtq_f32_s32(b);
#else
    rf = simd_float(
        static_cast<float>(r.data[0]),
        static_cast<float>(r.data[1]),
        static_cast<float>(r.data[2]),
        static_cast<float>(r.data[3])
    );
    gf = simd_float(
        static_cast<float>(g.data[0]),
        static_cast<float>(g.data[1]),
        static_cast<float>(g.data[2]),
        static_cast<float>(g.data[3])
    );
    bf = simd_float(
        static_cast<float>(b.data[0]),
        static_cast<float>(b.data[1]),
        static_cast<float>(b.data[2]),
        static_cast<float>(b.data[3])
    );
#endif
    
    // Apply contrast adjustment: (color - 128) * contrast + 128
    simd_float mid = BroadcastFloat(128.0f);
    simd_float adjusted_rf = AddFloat(MulFloat(SubFloat(rf, mid), contrast), mid);
    simd_float adjusted_gf = AddFloat(MulFloat(SubFloat(gf, mid), contrast), mid);
    simd_float adjusted_bf = AddFloat(MulFloat(SubFloat(bf, mid), contrast), mid);
    
    // Convert back to integer and clamp
#ifdef __SSE2__
    r = _mm_cvtps_epi32(adjusted_rf);
    g = _mm_cvtps_epi32(adjusted_gf);
    b = _mm_cvtps_epi32(adjusted_bf);
    
    // Clamp to [0, 255]
    r = _mm_max_epi32(_mm_min_epi32(r, BroadcastInt32(255)), BroadcastInt32(0));
    g = _mm_max_epi32(_mm_min_epi32(g, BroadcastInt32(255)), BroadcastInt32(0));
    b = _mm_max_epi32(_mm_min_epi32(b, BroadcastInt32(255)), BroadcastInt32(0));
#elif defined(__ARM_NEON__)
    r = vcvtq_s32_f32(adjusted_rf);
    g = vcvtq_s32_f32(adjusted_gf);
    b = vcvtq_s32_f32(adjusted_bf);
    
    // Clamp to [0, 255]
    r = vmaxq_s32(vminq_s32(r, BroadcastInt32(255)), BroadcastInt32(0));
    g = vmaxq_s32(vminq_s32(g, BroadcastInt32(255)), BroadcastInt32(0));
    b = vmaxq_s32(vminq_s32(b, BroadcastInt32(255)), BroadcastInt32(0));
#else
    for (int i = 0; i < 4; ++i) {
        r.data[i] = static_cast<int>(Clamp(adjusted_rf.data[i], 0.0f, 255.0f));
        g.data[i] = static_cast<int>(Clamp(adjusted_gf.data[i], 0.0f, 255.0f));
        b.data[i] = static_cast<int>(Clamp(adjusted_bf.data[i], 0.0f, 255.0f));
    }
#endif
    
    return PackRGBA(r, g, b, a);
}

} // namespace Graphics

// String representation
inline std::string AsString(const SIMD::simd_int32& v) {
#ifdef __SSE2__
    alignas(16) int32_t data[4];
    _mm_store_si128(reinterpret_cast<__m128i*>(data), v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + 
           ", " + std::to_string(data[2]) + ", " + std::to_string(data[3]) + "]";
#elif defined(__ARM_NEON__)
    int32_t data[4];
    vst1q_s32(data, v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + 
           ", " + std::to_string(data[2]) + ", " + std::to_string(data[3]) + "]";
#else
    return std::string("[") + std::to_string(v.data[0]) + ", " + std::to_string(v.data[1]) + 
           ", " + std::to_string(v.data[2]) + ", " + std::to_string(v.data[3]) + "]";
#endif
}

inline std::string AsString(const SIMD::simd_float& v) {
#ifdef __SSE2__
    alignas(16) float data[4];
    _mm_store_ps(data, v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + 
           ", " + std::to_string(data[2]) + ", " + std::to_string(data[3]) + "]";
#elif defined(__ARM_NEON__)
    float data[4];
    vst1q_f32(data, v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + 
           ", " + std::to_string(data[2]) + ", " + std::to_string(data[3]) + "]";
#else
    return std::string("[") + std::to_string(v.data[0]) + ", " + std::to_string(v.data[1]) + 
           ", " + std::to_string(v.data[2]) + ", " + std::to_string(v.data[3]) + "]";
#endif
}

inline std::string AsString(const SIMD::simd_double& v) {
#ifdef __SSE2__
    alignas(16) double data[2];
    _mm_store_pd(data, v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + "]";
#elif defined(__ARM_NEON__)
    double data[2];
    vst1q_f64(data, v);
    return std::string("[") + std::to_string(data[0]) + ", " + std::to_string(data[1]) + "]";
#else
    return std::string("[") + std::to_string(v.data[0]) + ", " + std::to_string(v.data[1]) + "]";
#endif
}

// Streaming operators
template<typename Stream>
void operator%(Stream& s, SIMD::simd_int32& v) {
    // SIMD vectors are typically not serialized directly
    // This is just a placeholder to satisfy interface requirements
    s.LoadError();
}

template<typename Stream>
void operator%(Stream& s, SIMD::simd_float& v) {
    // SIMD vectors are typically not serialized directly
    // This is just a placeholder to satisfy interface requirements
    s.LoadError();
}

template<typename Stream>
void operator%(Stream& s, SIMD::simd_double& v) {
    // SIMD vectors are typically not serialized directly
    // This is just a placeholder to satisfy interface requirements
    s.LoadError();
}

#endif