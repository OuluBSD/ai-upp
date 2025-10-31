#pragma once
#ifndef _Core_PackedData_h_
#define _Core_PackedData_h_

#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include "Core.h"

// PackedData - efficient binary serialization for stdsrc
// Provides compact storage and fast serialization of data structures

class PackedData {
private:
    std::vector<byte> data;
    size_t position;
    
public:
    // Constructors
    PackedData() : position(0) {}
    
    PackedData(const PackedData& other) 
        : data(other.data), position(other.position) {}
    
    PackedData(PackedData&& other) noexcept 
        : data(std::move(other.data)), position(other.position) {
        other.position = 0;
    }
    
    PackedData& operator=(const PackedData& other) {
        if (this != &other) {
            data = other.data;
            position = other.position;
        }
        return *this;
    }
    
    PackedData& operator=(PackedData&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            position = other.position;
            other.position = 0;
        }
        return *this;
    }
    
    // Destructor
    ~PackedData() = default;
    
    // Data access
    const byte* GetData() const { return data.data(); }
    size_t GetSize() const { return data.size(); }
    bool IsEmpty() const { return data.empty(); }
    
    void Clear() {
        data.clear();
        position = 0;
    }
    
    void Reserve(size_t size) {
        data.reserve(size);
    }
    
    void Shrink() {
        data.shrink_to_fit();
    }
    
    // Position management
    size_t GetPosition() const { return position; }
    void SetPosition(size_t pos) { position = pos; }
    void Seek(size_t pos) { position = pos; }
    void Skip(size_t count) { position += count; }
    
    // Writing operations
    void Write(const void* ptr, size_t size) {
        if (position + size > data.size()) {
            data.resize(position + size);
        }
        std::memcpy(data.data() + position, ptr, size);
        position += size;
    }
    
    void WriteByte(byte b) {
        Write(&b, 1);
    }
    
    void WriteWord(word w) {
        Write(&w, sizeof(w));
    }
    
    void WriteDWord(dword dw) {
        Write(&dw, sizeof(dw));
    }
    
    void WriteInt(int i) {
        Write(&i, sizeof(i));
    }
    
    void WriteInt64(int64 i) {
        Write(&i, sizeof(i));
    }
    
    void WriteUInt64(uint64 u) {
        Write(&u, sizeof(u));
    }
    
    void WriteFloat(float f) {
        Write(&f, sizeof(f));
    }
    
    void WriteDouble(double d) {
        Write(&d, sizeof(d));
    }
    
    void WriteString(const String& s) {
        size_t len = s.GetCount();
        WriteUInt64(static_cast<uint64>(len));
        if (len > 0) {
            Write(~s, len);
        }
    }
    
    void WriteWString(const WString& s) {
        size_t len = s.GetCount();
        WriteUInt64(static_cast<uint64>(len));
        if (len > 0) {
            Write(~s, len * sizeof(wchar));
        }
    }
    
    // Reading operations
    bool Read(void* ptr, size_t size) {
        if (position + size > data.size()) {
            return false;
        }
        std::memcpy(ptr, data.data() + position, size);
        position += size;
        return true;
    }
    
    bool ReadByte(byte& b) {
        return Read(&b, 1);
    }
    
    bool ReadWord(word& w) {
        return Read(&w, sizeof(w));
    }
    
    bool ReadDWord(dword& dw) {
        return Read(&dw, sizeof(dw));
    }
    
    bool ReadInt(int& i) {
        return Read(&i, sizeof(i));
    }
    
    bool ReadInt64(int64& i) {
        return Read(&i, sizeof(i));
    }
    
    bool ReadUInt64(uint64& u) {
        return Read(&u, sizeof(u));
    }
    
    bool ReadFloat(float& f) {
        return Read(&f, sizeof(f));
    }
    
    bool ReadDouble(double& d) {
        return Read(&d, sizeof(d));
    }
    
    bool ReadString(String& s) {
        uint64 len;
        if (!ReadUInt64(len)) {
            return false;
        }
        
        if (len > data.size() - position) {
            return false;
        }
        
        s = String(reinterpret_cast<const char*>(data.data() + position), static_cast<int>(len));
        position += static_cast<size_t>(len);
        return true;
    }
    
    bool ReadWString(WString& s) {
        uint64 len;
        if (!ReadUInt64(len)) {
            return false;
        }
        
        if (len * sizeof(wchar) > data.size() - position) {
            return false;
        }
        
        s = WString(reinterpret_cast<const wchar*>(data.data() + position), static_cast<int>(len));
        position += static_cast<size_t>(len * sizeof(wchar));
        return true;
    }
    
    // Variable-length encoding for integers
    void WriteVLQ(uint64 value) {
        do {
            byte b = value & 0x7F;
            value >>= 7;
            if (value > 0) {
                b |= 0x80;
            }
            WriteByte(b);
        } while (value > 0);
    }
    
    bool ReadVLQ(uint64& value) {
        value = 0;
        int shift = 0;
        
        while (true) {
            byte b;
            if (!ReadByte(b)) {
                return false;
            }
            
            value |= static_cast<uint64>(b & 0x7F) << shift;
            
            if ((b & 0x80) == 0) {
                break;
            }
            
            shift += 7;
        }
        
        return true;
    }
    
    // Zig-zag encoding for signed integers
    void WriteSignedVLQ(int64 value) {
        uint64 zigzag = (value << 1) ^ (value >> 63);
        WriteVLQ(zigzag);
    }
    
    bool ReadSignedVLQ(int64& value) {
        uint64 zigzag;
        if (!ReadVLQ(zigzag)) {
            return false;
        }
        
        value = (zigzag >> 1) ^ (-(zigzag & 1));
        return true;
    }
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        size_t size = data.size();
        s / size;
        
        if (s.IsLoading()) {
            data.resize(size);
            if (size > 0) {
                s.SerializeRaw(data.data(), size);
            }
            position = 0;
        } else {
            if (size > 0) {
                s.SerializeRaw(data.data(), size);
            }
        }
    }
    
    // Streaming operators
    template<typename Stream>
    friend void operator%(Stream& s, PackedData& pd) {
        pd.Serialize(s);
    }
    
    // String representation
    String ToString() const {
        return "PackedData(" + AsString(data.size()) + " bytes)";
    }
    
    // Utility functions
    static PackedData FromString(const String& s) {
        PackedData pd;
        pd.Write(~s, s.GetCount());
        return pd;
    }
    
    String AsString() const {
        return String(reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()));
    }
    
    // Comparison operators
    bool operator==(const PackedData& other) const {
        return data == other.data;
    }
    
    bool operator!=(const PackedData& other) const {
        return !(*this == other);
    }
    
    // Hash value
    hash_t GetHashValue() const {
        return memhash(data.data(), data.size());
    }
    
    // Swap
    void Swap(PackedData& other) {
        data.swap(other.data);
        std::swap(position, other.position);
    }
};

// Global swap function
inline void Swap(PackedData& a, PackedData& b) {
    a.Swap(b);
}

// Streaming operator
template<typename Stream>
void operator%(Stream& s, PackedData& pd) {
    pd.Serialize(s);
}

// String conversion
inline String AsString(const PackedData& pd) {
    return pd.ToString();
}

// Hash function
inline hash_t GetHashValue(const PackedData& pd) {
    return pd.GetHashValue();
}

#endif