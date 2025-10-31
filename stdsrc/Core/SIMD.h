#pragma once
#ifndef _Core_SIMD_h_
#define _Core_SIMD_h_

#include <cstdint>
#include <cstring>
#include "Core.h"

// SIMD (Single Instruction, Multiple Data) utilities for stdsrc
// Provides cross-platform SIMD operations using standard library functions

// SIMD vector types
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

// SIMD operations
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

// SIMD-enabled mathematical functions
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

// String representation for debugging
inline String AsString(const simd_int32& v) {
#ifdef __SSE2__
    alignas(16) int32_t data[4];
    _mm_store_si128(reinterpret_cast<__m128i*>(data), v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << "]";
#elif defined(__ARM_NEON__)
    int32_t data[4];
    vst1q_s32(data, v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << "]";
#else
    return String().Cat() << "[" << v.data[0] << ", " << v.data[1] << ", " << v.data[2] << ", " << v.data[3] << "]";
#endif
}

inline String AsString(const simd_float& v) {
#ifdef __SSE2__
    alignas(16) float data[4];
    _mm_store_ps(data, v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << "]";
#elif defined(__ARM_NEON__)
    float data[4];
    vst1q_f32(data, v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << "]";
#else
    return String().Cat() << "[" << v.data[0] << ", " << v.data[1] << ", " << v.data[2] << ", " << v.data[3] << "]";
#endif
}

inline String AsString(const simd_double& v) {
#ifdef __SSE2__
    alignas(16) double data[2];
    _mm_store_pd(data, v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << "]";
#elif defined(__ARM_NEON__)
    double data[2];
    vst1q_f64(data, v);
    return String().Cat() << "[" << data[0] << ", " << data[1] << "]";
#else
    return String().Cat() << "[" << v.data[0] << ", " << v.data[1] << "]";
#endif
}

} // namespace SIMD

// Global functions
inline String AsString(const SIMD::simd_int32& v) { return SIMD::AsString(v); }
inline String AsString(const SIMD::simd_float& v) { return SIMD::AsString(v); }
inline String AsString(const SIMD::simd_double& v) { return SIMD::AsString(v); }

// Streaming operators
template<typename Stream>
void operator%(Stream& s, SIMD::simd_int32& v) {
    // Serialization would depend on the specific implementation
    // This is a placeholder implementation
}

template<typename Stream>
void operator%(Stream& s, SIMD::simd_float& v) {
    // Serialization would depend on the specific implementation
    // This is a placeholder implementation
}

template<typename Stream>
void operator%(Stream& s, SIMD::simd_double& v) {
    // Serialization would depend on the specific implementation
    // This is a placeholder implementation
}

#endif