#pragma once
#ifndef _Core_Random_h_
#define _Core_Random_h_

#include <random>
#include <chrono>
#include <cstdint>
#include "Core.h"

// Random number generation utilities for stdsrc

class Random {
private:
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> uint64_dist;
    std::uniform_real_distribution<double> double_dist;
    std::uniform_real_distribution<float> float_dist;
    
public:
    // Constructors
    Random() : rng(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
               uint64_dist(0, UINT64_MAX),
               double_dist(0.0, 1.0),
               float_dist(0.0f, 1.0f) {}
    
    explicit Random(uint64_t seed) : rng(seed),
                                     uint64_dist(0, UINT64_MAX),
                                     double_dist(0.0, 1.0),
                                     float_dist(0.0f, 1.0f) {}
    
    // Copy constructor and assignment
    Random(const Random& other) : rng(other.rng),
                                 uint64_dist(other.uint64_dist),
                                 double_dist(other.double_dist),
                                 float_dist(other.float_dist) {}
    
    Random& operator=(const Random& other) {
        if (this != &other) {
            rng = other.rng;
            uint64_dist = other.uint64_dist;
            double_dist = other.double_dist;
            float_dist = other.float_dist;
        }
        return *this;
    }
    
    // Move constructor and assignment
    Random(Random&& other) noexcept : rng(std::move(other.rng)),
                                     uint64_dist(std::move(other.uint64_dist)),
                                     double_dist(std::move(other.double_dist)),
                                     float_dist(std::move(other.float_dist)) {}
    
    Random& operator=(Random&& other) noexcept {
        if (this != &other) {
            rng = std::move(other.rng);
            uint64_dist = std::move(other.uint64_dist);
            double_dist = std::move(other.double_dist);
            float_dist = std::move(other.float_dist);
        }
        return *this;
    }
    
    // Seed management
    void Seed(uint64_t seed) {
        rng.seed(seed);
    }
    
    void Seed() {
        Seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    
    // Integer generation
    uint64 GetUInt64() {
        return uint64_dist(rng);
    }
    
    uint32 GetUInt32() {
        return static_cast<uint32>(GetUInt64() >> 32);
    }
    
    uint16 GetUInt16() {
        return static_cast<uint16>(GetUInt64() >> 48);
    }
    
    uint8 GetUInt8() {
        return static_cast<uint8>(GetUInt64() >> 56);
    }
    
    int64 GetInt64() {
        return static_cast<int64>(GetUInt64());
    }
    
    int32 GetInt32() {
        return static_cast<int32>(GetUInt32());
    }
    
    int16 GetInt16() {
        return static_cast<int16>(GetUInt16());
    }
    
    int8 GetInt8() {
        return static_cast<int8>(GetUInt8());
    }
    
    // Bounded integer generation
    uint64 GetUInt64(uint64 min, uint64 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<uint64_t> dist(min, max);
        return dist(rng);
    }
    
    uint32 GetUInt32(uint32 min, uint32 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<uint32_t> dist(min, max);
        return dist(rng);
    }
    
    uint16 GetUInt16(uint16 min, uint16 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<uint16_t> dist(min, max);
        return dist(rng);
    }
    
    uint8 GetUInt8(uint8 min, uint8 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<uint8_t> dist(min, max);
        return dist(rng);
    }
    
    int64 GetInt64(int64 min, int64 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int64_t> dist(min, max);
        return dist(rng);
    }
    
    int32 GetInt32(int32 min, int32 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int32_t> dist(min, max);
        return dist(rng);
    }
    
    int16 GetInt16(int16 min, int16 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int16_t> dist(min, max);
        return dist(rng);
    }
    
    int8 GetInt8(int8 min, int8 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int8_t> dist(min, max);
        return dist(rng);
    }
    
    // Floating-point generation
    double GetDouble() {
        return double_dist(rng);
    }
    
    float GetFloat() {
        return float_dist(rng);
    }
    
    // Bounded floating-point generation
    double GetDouble(double min, double max) {
        if (min >= max) return min;
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }
    
    float GetFloat(float min, float max) {
        if (min >= max) return min;
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
    
    // Boolean generation
    bool GetBool() {
        return (GetUInt32() & 1) != 0;
    }
    
    bool GetBool(double probability) {
        if (probability <= 0.0) return false;
        if (probability >= 1.0) return true;
        return GetDouble() < probability;
    }
    
    // Character generation
    char GetChar() {
        return static_cast<char>(GetUInt8());
    }
    
    char GetChar(char min, char max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int> dist(min, max);
        return static_cast<char>(dist(rng));
    }
    
    // Byte generation
    byte GetByte() {
        return static_cast<byte>(GetUInt8());
    }
    
    byte GetByte(byte min, byte max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int> dist(min, max);
        return static_cast<byte>(dist(rng));
    }
    
    // Word generation
    word GetWord() {
        return static_cast<word>(GetUInt16());
    }
    
    word GetWord(word min, word max) {
        if (min >= max) return min;
        std::uniform_int_distribution<int> dist(min, max);
        return static_cast<word>(dist(rng));
    }
    
    // Dword generation
    dword GetDWord() {
        return static_cast<dword>(GetUInt32());
    }
    
    dword GetDWord(dword min, dword max) {
        if (min >= max) return min;
        std::uniform_int_distribution<uint32_t> dist(min, max);
        return dist(rng);
    }
    
    // Range-based generation
    template<typename T>
    T Get(const T& min, const T& max) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_unsigned_v<T>) {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(rng);
            } else {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(rng);
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(rng);
        } else {
            // For custom types, use default distribution
            return T();
        }
    }
    
    // Choice from container
    template<typename Container>
    auto Choice(const Container& container) -> const typename Container::value_type& {
        if (container.empty()) {
            throw std::runtime_error("Cannot choose from empty container");
        }
        
        size_t index = Get(size_t(0), container.size() - 1);
        auto it = container.begin();
        std::advance(it, index);
        return *it;
    }
    
    // Shuffle container
    template<typename Container>
    void Shuffle(Container& container) {
        std::shuffle(container.begin(), container.end(), rng);
    }
    
    // Fill container with random values
    template<typename Container>
    void Fill(Container& container, const typename Container::value_type& min, const typename Container::value_type& max) {
        for (auto& item : container) {
            item = Get(min, max);
        }
    }
    
    // Generate random string
    String GetString(int length) {
        String result;
        result.Reserve(length);
        for (int i = 0; i < length; ++i) {
            result.Cat(GetChar('a', 'z'));
        }
        return result;
    }
    
    String GetString(int min_length, int max_length) {
        int length = Get(min_length, max_length);
        return GetString(length);
    }
    
    // Generate random alphanumeric string
    String GetAlphanumericString(int length) {
        static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        String result;
        result.Reserve(length);
        for (int i = 0; i < length; ++i) {
            result.Cat(chars[Get(0, 61)]);
        }
        return result;
    }
    
    // Generate random hex string
    String GetHexString(int length) {
        static const char hex_chars[] = "0123456789abcdef";
        String result;
        result.Reserve(length);
        for (int i = 0; i < length; ++i) {
            result.Cat(hex_chars[Get(0, 15)]);
        }
        return result;
    }
    
    // Normal distribution
    double GetNormal(double mean = 0.0, double stddev = 1.0) {
        std::normal_distribution<double> dist(mean, stddev);
        return dist(rng);
    }
    
    // Exponential distribution
    double GetExponential(double lambda = 1.0) {
        std::exponential_distribution<double> dist(lambda);
        return dist(rng);
    }
    
    // Gamma distribution
    double GetGamma(double alpha, double beta = 1.0) {
        std::gamma_distribution<double> dist(alpha, beta);
        return dist(rng);
    }
    
    // Beta distribution (approximation)
    double GetBeta(double alpha, double beta) {
        double x = GetGamma(alpha, 1.0);
        double y = GetGamma(beta, 1.0);
        return x / (x + y);
    }
    
    // Poisson distribution
    int GetPoisson(double mean) {
        std::poisson_distribution<int> dist(mean);
        return dist(rng);
    }
    
    // Bernoulli distribution
    bool GetBernoulli(double p) {
        std::bernoulli_distribution dist(p);
        return dist(rng);
    }
    
    // Binomial distribution
    int GetBinomial(int t, double p) {
        std::binomial_distribution<int> dist(t, p);
        return dist(rng);
    }
    
    // Geometric distribution
    int GetGeometric(double p) {
        std::geometric_distribution<int> dist(p);
        return dist(rng);
    }
    
    // Cauchy distribution
    double GetCauchy(double a = 0.0, double b = 1.0) {
        std::cauchy_distribution<double> dist(a, b);
        return dist(rng);
    }
    
    // Chi-squared distribution
    double GetChiSquared(double n) {
        std::chi_squared_distribution<double> dist(n);
        return dist(rng);
    }
    
    // Fisher-F distribution
    double GetFisherF(double m, double n) {
        std::fisher_f_distribution<double> dist(m, n);
        return dist(rng);
    }
    
    // Student t-distribution
    double GetStudentT(double n) {
        std::student_t_distribution<double> dist(n);
        return dist(rng);
    }
    
    // Weibull distribution
    double GetWeibull(double a, double b = 1.0) {
        std::weibull_distribution<double> dist(a, b);
        return dist(rng);
    }
    
    // Extreme value distribution
    double GetExtremeValue(double a = 0.0, double b = 1.0) {
        std::extreme_value_distribution<double> dist(a, b);
        return dist(rng);
    }
    
    // Lognormal distribution
    double GetLognormal(double m = 0.0, double s = 1.0) {
        std::lognormal_distribution<double> dist(m, s);
        return dist(rng);
    }
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        uint64_t state;
        if (s.IsStoring()) {
            // Save state (this is a simplified approach)
            state = rng();
        } else {
            // Restore state
            s % state;
            Seed(state);
        }
    }
    
    // Streaming operator
    template<typename Stream>
    friend void operator%(Stream& s, Random& r) {
        r.Serialize(s);
    }
    
    // String representation
    String ToString() const {
        return "Random(engine=mt19937_64)";
    }
    
    // Utility functions
    static Random& Global() {
        static Random instance;
        return instance;
    }
    
    static uint64 GetGlobalUInt64() {
        return Global().GetUInt64();
    }
    
    static double GetGlobalDouble() {
        return Global().GetDouble();
    }
    
    static bool GetGlobalBool() {
        return Global().GetBool();
    }
    
    static String GetGlobalString(int length) {
        return Global().GetString(length);
    }
    
    static void SeedGlobal(uint64 seed) {
        Global().Seed(seed);
    }
    
    static void SeedGlobal() {
        Global().Seed();
    }
};

// Global random functions for convenience
inline uint64 RandomUInt64() {
    return Random::GetGlobalUInt64();
}

inline uint32 RandomUInt32() {
    return static_cast<uint32>(RandomUInt64() >> 32);
}

inline uint16 RandomUInt16() {
    return static_cast<uint16>(RandomUInt64() >> 48);
}

inline uint8 RandomUInt8() {
    return static_cast<uint8>(RandomUInt64() >> 56);
}

inline int64 RandomInt64() {
    return static_cast<int64>(RandomUInt64());
}

inline int32 RandomInt32() {
    return static_cast<int32>(RandomUInt32());
}

inline int16 RandomInt16() {
    return static_cast<int16>(RandomUInt16());
}

inline int8 RandomInt8() {
    return static_cast<int8>(RandomUInt8());
}

inline double RandomDouble() {
    return Random::GetGlobalDouble();
}

inline float RandomFloat() {
    return static_cast<float>(RandomDouble());
}

inline bool RandomBool() {
    return Random::GetGlobalBool();
}

inline char RandomChar() {
    return static_cast<char>(RandomUInt8());
}

inline byte RandomByte() {
    return static_cast<byte>(RandomUInt8());
}

inline word RandomWord() {
    return static_cast<word>(RandomUInt16());
}

inline dword RandomDWord() {
    return static_cast<dword>(RandomUInt32());
}

inline String RandomString(int length) {
    return Random::GetGlobalString(length);
}

inline String RandomAlphanumericString(int length) {
    static Random r;
    return r.GetAlphanumericString(length);
}

inline String RandomHexString(int length) {
    static Random r;
    return r.GetHexString(length);
}

// Convenience functions for bounded random values
inline uint64 RandomUInt64(uint64 min, uint64 max) {
    static Random r;
    return r.GetUInt64(min, max);
}

inline uint32 RandomUInt32(uint32 min, uint32 max) {
    static Random r;
    return r.GetUInt32(min, max);
}

inline uint16 RandomUInt16(uint16 min, uint16 max) {
    static Random r;
    return r.GetUInt16(min, max);
}

inline uint8 RandomUInt8(uint8 min, uint8 max) {
    static Random r;
    return r.GetUInt8(min, max);
}

inline int64 RandomInt64(int64 min, int64 max) {
    static Random r;
    return r.GetInt64(min, max);
}

inline int32 RandomInt32(int32 min, int32 max) {
    static Random r;
    return r.GetInt32(min, max);
}

inline int16 RandomInt16(int16 min, int16 max) {
    static Random r;
    return r.GetInt16(min, max);
}

inline int8 RandomInt8(int8 min, int8 max) {
    static Random r;
    return r.GetInt8(min, max);
}

inline double RandomDouble(double min, double max) {
    static Random r;
    return r.GetDouble(min, max);
}

inline float RandomFloat(float min, float max) {
    static Random r;
    return r.GetFloat(min, max);
}

inline bool RandomBool(double probability) {
    static Random r;
    return r.GetBool(probability);
}

inline char RandomChar(char min, char max) {
    static Random r;
    return r.GetChar(min, max);
}

inline byte RandomByte(byte min, byte max) {
    static Random r;
    return r.GetByte(min, max);
}

inline word RandomWord(word min, word max) {
    static Random r;
    return r.GetWord(min, max);
}

inline dword RandomDWord(dword min, dword max) {
    static Random r;
    return r.GetDWord(min, max);
}

inline String RandomString(int min_length, int max_length) {
    static Random r;
    return r.GetString(min_length, max_length);
}

// Streaming operator
template<typename Stream>
void operator%(Stream& s, Random& r) {
    r.Serialize(s);
}

// String conversion
inline String AsString(const Random& r) {
    return r.ToString();
}

#endif