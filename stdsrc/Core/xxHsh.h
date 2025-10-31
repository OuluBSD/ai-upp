#pragma once
#ifndef _Core_xxHsh_h_
#define _Core_xxHsh_h_

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "Core.h"

// xxHash implementation for stdsrc
// Provides fast non-cryptographic hash functions

class xxHsh {
private:
    static const uint32_t PRIME32_1 = 2654435761U;
    static const uint32_t PRIME32_2 = 2246822519U;
    static const uint32_t PRIME32_3 = 3266489917U;
    static const uint32_t PRIME32_4 = 668265263U;
    static const uint32_t PRIME32_5 = 374761393U;
    
    static const uint64_t PRIME64_1 = 11400714785074694791ULL;
    static const uint64_t PRIME64_2 = 14029467366897019727ULL;
    static const uint64_t PRIME64_3 = 1609587929392839161ULL;
    static const uint64_t PRIME64_4 = 9650029242287828579ULL;
    static const uint64_t PRIME64_5 = 2870177450012600261ULL;
    
    // Rotate left operation
    static inline uint32_t Rotl32(uint32_t x, int8_t r) {
        return (x << r) | (x >> (32 - r));
    }
    
    static inline uint64_t Rotl64(uint64_t x, int8_t r) {
        return (x << r) | (x >> (64 - r));
    }
    
    // 32-bit hash functions
    static inline uint32_t Get32bits(const void* memPtr) {
        uint32_t val;
        memcpy(&val, memPtr, sizeof(val));
        return val;
    }
    
    static inline uint32_t XXH32_round(uint32_t seed, uint32_t input) {
        seed += input * PRIME32_2;
        seed = Rotl32(seed, 13);
        seed *= PRIME32_1;
        return seed;
    }
    
    static inline uint32_t XXH32_mergeRound(uint32_t acc, uint32_t val) {
        acc ^= XXH32_round(0, val);
        acc = acc * PRIME32_1 + PRIME32_4;
        return acc;
    }
    
    // 64-bit hash functions
    static inline uint64_t Get64bits(const void* memPtr) {
        uint64_t val;
        memcpy(&val, memPtr, sizeof(val));
        return val;
    }
    
    static inline uint64_t XXH64_round(uint64_t acc, uint64_t input) {
        acc += input * PRIME64_2;
        acc = Rotl64(acc, 31);
        acc *= PRIME64_1;
        return acc;
    }
    
    static inline uint64_t XXH64_mergeRound(uint64_t acc, uint64_t val) {
        acc ^= XXH64_round(0, val);
        acc = acc * PRIME64_1 + PRIME64_4;
        return acc;
    }
    
public:
    // 32-bit xxHash
    static uint32_t XXH32(const void* input, size_t length, uint32_t seed = 0) {
        const uint8_t* p = (const uint8_t*)input;
        const uint8_t* const bEnd = p + length;
        uint32_t h32;
        
        if (length >= 16) {
            const uint8_t* const limit = bEnd - 15;
            uint32_t v1 = seed + PRIME32_1 + PRIME32_2;
            uint32_t v2 = seed + PRIME32_2;
            uint32_t v3 = seed + 0;
            uint32_t v4 = seed - PRIME32_1;
            
            do {
                v1 = XXH32_round(v1, Get32bits(p)); p += 4;
                v2 = XXH32_round(v2, Get32bits(p)); p += 4;
                v3 = XXH32_round(v3, Get32bits(p)); p += 4;
                v4 = XXH32_round(v4, Get32bits(p)); p += 4;
            } while (p < limit);
            
            h32 = Rotl32(v1, 1) + Rotl32(v2, 7) + Rotl32(v3, 12) + Rotl32(v4, 18);
        } else {
            h32 = seed + PRIME32_5;
        }
        
        h32 += (uint32_t)length;
        
        while (p + 4 <= bEnd) {
            h32 += Get32bits(p) * PRIME32_3;
            h32 = Rotl32(h32, 17) * PRIME32_4;
            p += 4;
        }
        
        while (p < bEnd) {
            h32 += (*p) * PRIME32_5;
            h32 = Rotl32(h32, 11) * PRIME32_1;
            p++;
        }
        
        h32 ^= h32 >> 15;
        h32 *= PRIME32_2;
        h32 ^= h32 >> 13;
        h32 *= PRIME32_3;
        h32 ^= h32 >> 16;
        
        return h32;
    }
    
    // 64-bit xxHash
    static uint64_t XXH64(const void* input, size_t length, uint64_t seed = 0) {
        const uint8_t* p = (const uint8_t*)input;
        const uint8_t* const bEnd = p + length;
        uint64_t h64;
        
        if (length >= 32) {
            const uint8_t* const limit = bEnd - 31;
            uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
            uint64_t v2 = seed + PRIME64_2;
            uint64_t v3 = seed + 0;
            uint64_t v4 = seed - PRIME64_1;
            
            do {
                v1 = XXH64_round(v1, Get64bits(p)); p += 8;
                v2 = XXH64_round(v2, Get64bits(p)); p += 8;
                v3 = XXH64_round(v3, Get64bits(p)); p += 8;
                v4 = XXH64_round(v4, Get64bits(p)); p += 8;
            } while (p < limit);
            
            h64 = Rotl64(v1, 1) + Rotl64(v2, 7) + Rotl64(v3, 12) + Rotl64(v4, 18);
            
            v1 *= PRIME64_2; v1 = Rotl64(v1, 31); v1 *= PRIME64_1; h64 ^= v1;
            h64 = h64 * PRIME64_1 + PRIME64_4;
            
            v2 *= PRIME64_2; v2 = Rotl64(v2, 31); v2 *= PRIME64_1; h64 ^= v2;
            h64 = h64 * PRIME64_1 + PRIME64_4;
            
            v3 *= PRIME64_2; v3 = Rotl64(v3, 31); v3 *= PRIME64_1; h64 ^= v3;
            h64 = h64 * PRIME64_1 + PRIME64_4;
            
            v4 *= PRIME64_2; v4 = Rotl64(v4, 31); v4 *= PRIME64_1; h64 ^= v4;
            h64 = h64 * PRIME64_1 + PRIME64_4;
        } else {
            h64 = seed + PRIME64_5;
        }
        
        h64 += (uint64_t)length;
        
        while (p + 8 <= bEnd) {
            uint64_t k1 = XXH64_round(0, Get64bits(p));
            h64 ^= k1;
            h64 = Rotl64(h64, 27) * PRIME64_1 + PRIME64_4;
            p += 8;
        }
        
        if (p + 4 <= bEnd) {
            h64 ^= (uint64_t)(Get32bits(p)) * PRIME64_1;
            h64 = Rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
            p += 4;
        }
        
        while (p < bEnd) {
            h64 ^= (*p) * PRIME64_5;
            h64 = Rotl64(h64, 11) * PRIME64_1;
            p++;
        }
        
        h64 ^= h64 >> 33;
        h64 *= PRIME64_2;
        h64 ^= h64 >> 29;
        h64 *= PRIME64_3;
        h64 ^= h64 >> 32;
        
        return h64;
    }
    
    // Convenience functions for common types
    static uint32_t Hash32(const std::string& str, uint32_t seed = 0) {
        return XXH32(str.data(), str.length(), seed);
    }
    
    static uint64_t Hash64(const std::string& str, uint64_t seed = 0) {
        return XXH64(str.data(), str.length(), seed);
    }
    
    static uint32_t Hash32(const char* str, uint32_t seed = 0) {
        return XXH32(str, strlen(str), seed);
    }
    
    static uint64_t Hash64(const char* str, uint64_t seed = 0) {
        return XXH64(str, strlen(str), seed);
    }
    
    static uint32_t Hash32(int value, uint32_t seed = 0) {
        return XXH32(&value, sizeof(value), seed);
    }
    
    static uint64_t Hash64(int value, uint64_t seed = 0) {
        return XXH64(&value, sizeof(value), seed);
    }
    
    static uint32_t Hash32(int64_t value, uint32_t seed = 0) {
        return XXH32(&value, sizeof(value), seed);
    }
    
    static uint64_t Hash64(int64_t value, uint64_t seed = 0) {
        return XXH64(&value, sizeof(value), seed);
    }
    
    static uint32_t Hash32(double value, uint32_t seed = 0) {
        return XXH32(&value, sizeof(value), seed);
    }
    
    static uint64_t Hash64(double value, uint64_t seed = 0) {
        return XXH64(&value, sizeof(value), seed);
    }
    
    static uint32_t Hash32(const std::vector<uint8_t>& data, uint32_t seed = 0) {
        return XXH32(data.data(), data.size(), seed);
    }
    
    static uint64_t Hash64(const std::vector<uint8_t>& data, uint64_t seed = 0) {
        return XXH64(data.data(), data.size(), seed);
    }
    
    // Streaming hash computation
    class Stream32 {
    private:
        uint32_t v1, v2, v3, v4;
        uint32_t seed;
        uint64_t total_len;
        std::vector<uint8_t> buffer;
        size_t bufpos;
        
    public:
        Stream32(uint32_t seed = 0) : seed(seed), total_len(0), bufpos(0) {
            v1 = seed + PRIME32_1 + PRIME32_2;
            v2 = seed + PRIME32_2;
            v3 = seed + 0;
            v4 = seed - PRIME32_1;
            buffer.resize(16);
        }
        
        void Update(const void* input, size_t length) {
            const uint8_t* p = (const uint8_t*)input;
            total_len += length;
            
            if (bufpos + length < 16) {
                memcpy(buffer.data() + bufpos, p, length);
                bufpos += length;
                return;
            }
            
            if (bufpos > 0) {
                size_t fill = 16 - bufpos;
                memcpy(buffer.data() + bufpos, p, fill);
                ProcessBuffer();
                length -= fill;
                p += fill;
            }
            
            while (length >= 16) {
                ProcessBlock(p);
                p += 16;
                length -= 16;
            }
            
            bufpos = 0;
            if (length > 0) {
                memcpy(buffer.data(), p, length);
                bufpos = length;
            }
        }
        
        uint32_t Digest() {
            const uint8_t* p = buffer.data();
            uint32_t h32;
            
            if (total_len >= 16) {
                h32 = Rotl32(v1, 1) + Rotl32(v2, 7) + Rotl32(v3, 12) + Rotl32(v4, 18);
            } else {
                h32 = seed + PRIME32_5;
            }
            
            h32 += (uint32_t)total_len;
            
            while (bufpos >= 4) {
                h32 += Get32bits(p) * PRIME32_3;
                h32 = Rotl32(h32, 17) * PRIME32_4;
                p += 4;
                bufpos -= 4;
            }
            
            while (bufpos > 0) {
                h32 += (*p) * PRIME32_5;
                h32 = Rotl32(h32, 11) * PRIME32_1;
                p++;
                bufpos--;
            }
            
            h32 ^= h32 >> 15;
            h32 *= PRIME32_2;
            h32 ^= h32 >> 13;
            h32 *= PRIME32_3;
            h32 ^= h32 >> 16;
            
            return h32;
        }
        
    private:
        void ProcessBuffer() {
            ProcessBlock(buffer.data());
            bufpos = 0;
        }
        
        void ProcessBlock(const uint8_t* p) {
            v1 = XXH32_round(v1, Get32bits(p)); p += 4;
            v2 = XXH32_round(v2, Get32bits(p)); p += 4;
            v3 = XXH32_round(v3, Get32bits(p)); p += 4;
            v4 = XXH32_round(v4, Get32bits(p));
        }
    };
    
    class Stream64 {
    private:
        uint64_t v1, v2, v3, v4;
        uint64_t seed;
        uint64_t total_len;
        std::vector<uint8_t> buffer;
        size_t bufpos;
        
    public:
        Stream64(uint64_t seed = 0) : seed(seed), total_len(0), bufpos(0) {
            v1 = seed + PRIME64_1 + PRIME64_2;
            v2 = seed + PRIME64_2;
            v3 = seed + 0;
            v4 = seed - PRIME64_1;
            buffer.resize(32);
        }
        
        void Update(const void* input, size_t length) {
            const uint8_t* p = (const uint8_t*)input;
            total_len += length;
            
            if (bufpos + length < 32) {
                memcpy(buffer.data() + bufpos, p, length);
                bufpos += length;
                return;
            }
            
            if (bufpos > 0) {
                size_t fill = 32 - bufpos;
                memcpy(buffer.data() + bufpos, p, fill);
                ProcessBuffer();
                length -= fill;
                p += fill;
            }
            
            while (length >= 32) {
                ProcessBlock(p);
                p += 32;
                length -= 32;
            }
            
            bufpos = 0;
            if (length > 0) {
                memcpy(buffer.data(), p, length);
                bufpos = length;
            }
        }
        
        uint64_t Digest() {
            const uint8_t* p = buffer.data();
            uint64_t h64;
            
            if (total_len >= 32) {
                h64 = Rotl64(v1, 1) + Rotl64(v2, 7) + Rotl64(v3, 12) + Rotl64(v4, 18);
                
                v1 *= PRIME64_2; v1 = Rotl64(v1, 31); v1 *= PRIME64_1; h64 ^= v1;
                h64 = h64 * PRIME64_1 + PRIME64_4;
                
                v2 *= PRIME64_2; v2 = Rotl64(v2, 31); v2 *= PRIME64_1; h64 ^= v2;
                h64 = h64 * PRIME64_1 + PRIME64_4;
                
                v3 *= PRIME64_2; v3 = Rotl64(v3, 31); v3 *= PRIME64_1; h64 ^= v3;
                h64 = h64 * PRIME64_1 + PRIME64_4;
                
                v4 *= PRIME64_2; v4 = Rotl64(v4, 31); v4 *= PRIME64_1; h64 ^= v4;
                h64 = h64 * PRIME64_1 + PRIME64_4;
            } else {
                h64 = seed + PRIME64_5;
            }
            
            h64 += total_len;
            
            while (bufpos >= 8) {
                uint64_t k1 = XXH64_round(0, Get64bits(p));
                h64 ^= k1;
                h64 = Rotl64(h64, 27) * PRIME64_1 + PRIME64_4;
                p += 8;
                bufpos -= 8;
            }
            
            if (bufpos >= 4) {
                h64 ^= (uint64_t)(Get32bits(p)) * PRIME64_1;
                h64 = Rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
                p += 4;
                bufpos -= 4;
            }
            
            while (bufpos > 0) {
                h64 ^= (*p) * PRIME64_5;
                h64 = Rotl64(h64, 11) * PRIME64_1;
                p++;
                bufpos--;
            }
            
            h64 ^= h64 >> 33;
            h64 *= PRIME64_2;
            h64 ^= h64 >> 29;
            h64 *= PRIME64_3;
            h64 ^= h64 >> 32;
            
            return h64;
        }
        
    private:
        void ProcessBuffer() {
            ProcessBlock(buffer.data());
            bufpos = 0;
        }
        
        void ProcessBlock(const uint8_t* p) {
            v1 = XXH64_round(v1, Get64bits(p)); p += 8;
            v2 = XXH64_round(v2, Get64bits(p)); p += 8;
            v3 = XXH64_round(v3, Get64bits(p)); p += 8;
            v4 = XXH64_round(v4, Get64bits(p));
        }
    };
    
    // Utility functions
    static std::string Hash32ToString(uint32_t hash) {
        char buffer[9];
        sprintf(buffer, "%08X", hash);
        return std::string(buffer);
    }
    
    static std::string Hash64ToString(uint64_t hash) {
        char buffer[17];
        sprintf(buffer, "%016llX", (unsigned long long)hash);
        return std::string(buffer);
    }
    
    // Streaming operators
    template<typename Stream>
    friend void operator%(Stream& s, xxHsh& x) {
        // xxHsh is stateless, so nothing to serialize
    }
    
    // String representation
    static std::string ToString() {
        return "xxHsh(hash functions)";
    }
};

// Global functions for convenience
inline uint32_t XXH32(const void* input, size_t length, uint32_t seed = 0) {
    return xxHsh::XXH32(input, length, seed);
}

inline uint64_t XXH64(const void* input, size_t length, uint64_t seed = 0) {
    return xxHsh::XXH64(input, length, seed);
}

inline uint32_t Hash32(const std::string& str, uint32_t seed = 0) {
    return xxHsh::Hash32(str, seed);
}

inline uint64_t Hash64(const std::string& str, uint64_t seed = 0) {
    return xxHsh::Hash64(str, seed);
}

inline uint32_t Hash32(const char* str, uint32_t seed = 0) {
    return xxHsh::Hash32(str, seed);
}

inline uint64_t Hash64(const char* str, uint64_t seed = 0) {
    return xxHsh::Hash64(str, seed);
}

inline uint32_t Hash32(int value, uint32_t seed = 0) {
    return xxHsh::Hash32(value, seed);
}

inline uint64_t Hash64(int value, uint64_t seed = 0) {
    return xxHsh::Hash64(value, seed);
}

inline uint32_t Hash32(int64_t value, uint32_t seed = 0) {
    return xxHsh::Hash32(value, seed);
}

inline uint64_t Hash64(int64_t value, uint64_t seed = 0) {
    return xxHsh::Hash64(value, seed);
}

inline uint32_t Hash32(double value, uint32_t seed = 0) {
    return xxHsh::Hash32(value, seed);
}

inline uint64_t Hash64(double value, uint64_t seed = 0) {
    return xxHsh::Hash64(value, seed);
}

inline uint32_t Hash32(const std::vector<uint8_t>& data, uint32_t seed = 0) {
    return xxHsh::Hash32(data, seed);
}

inline uint64_t Hash64(const std::vector<uint8_t>& data, uint64_t seed = 0) {
    return xxHsh::Hash64(data, seed);
}

// Streaming operators
template<typename Stream>
void operator%(Stream& s, xxHsh::Stream32& stream) {
    // Serialization for streaming hash computation
}

template<typename Stream>
void operator%(Stream& s, xxHsh::Stream64& stream) {
    // Serialization for streaming hash computation
}

// String conversion
inline std::string AsString(const xxHsh& x) {
    return xxHsh::ToString();
}

inline std::string AsString(const xxHsh::Stream32& stream) {
    return "xxHsh::Stream32";
}

inline std::string AsString(const xxHsh::Stream64& stream) {
    return "xxHsh::Stream64";
}

#endif