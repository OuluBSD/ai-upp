#pragma once
#ifndef _Core_InMap_hpp_
#define _Core_InMap_hpp_

#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include "Core.h"

// InMap.hpp - Indexed map implementation for stdsrc
// Combines features of Index and VectorMap for efficient key-value storage

template <class K, class T>
class InMap {
private:
    // Vector to store values in indexed order
    std::vector<T> values;
    
    // Hash map for quick key lookup to index
    std::unordered_map<K, int> key_to_index;
    
    // Vector to store keys in indexed order (for iteration)
    std::vector<K> keys;
    
    // Unlinked items tracking (for sparse arrays)
    std::vector<int> unlinked_indices;
    
public:
    // Typedefs for compatibility
    typedef K KeyType;
    typedef T ValueType;
    typedef K key_type;
    typedef T mapped_type;
    typedef std::pair<const K, T> value_type;
    
    // Constructors
    InMap() = default;
    
    InMap(const InMap& other) 
        : values(other.values)
        , key_to_index(other.key_to_index)
        , keys(other.keys)
        , unlinked_indices(other.unlinked_indices) {}
    
    InMap(InMap&& other) noexcept 
        : values(std::move(other.values))
        , key_to_index(std::move(other.key_to_index))
        , keys(std::move(other.keys))
        , unlinked_indices(std::move(other.unlinked_indices)) {}
    
    InMap& operator=(const InMap& other) {
        if (this != &other) {
            values = other.values;
            key_to_index = other.key_to_index;
            keys = other.keys;
            unlinked_indices = other.unlinked_indices;
        }
        return *this;
    }
    
    InMap& operator=(InMap&& other) noexcept {
        if (this != &other) {
            values = std::move(other.values);
            key_to_index = std::move(other.key_to_index);
            keys = std::move(other.keys);
            unlinked_indices = std::move(other.unlinked_indices);
        }
        return *this;
    }
    
    // Destructor
    ~InMap() = default;
    
    // Element access
    T& operator[](const K& key) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            return values[it->second];
        }
        
        // Add new key
        int index = static_cast<int>(values.size());
        values.emplace_back();
        keys.push_back(key);
        key_to_index[key] = index;
        return values[index];
    }
    
    T& At(const K& key) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            return values[it->second];
        }
        throw std::out_of_range("Key not found");
    }
    
    const T& At(const K& key) const {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            return values[it->second];
        }
        throw std::out_of_range("Key not found");
    }
    
    const T& operator[](const K& key) const {
        return At(key);
    }
    
    // Iterators
    class Iterator {
    private:
        InMap* map;
        int index;
        
        void SkipUnlinked() {
            while (index < static_cast<int>(map->values.size()) && 
                   std::find(map->unlinked_indices.begin(), map->unlinked_indices.end(), index) != map->unlinked_indices.end()) {
                ++index;
            }
        }
        
    public:
        Iterator(InMap* m, int i) : map(m), index(i) {
            SkipUnlinked();
        }
        
        value_type operator*() const {
            return {map->keys[index], map->values[index]};
        }
        
        Iterator& operator++() {
            ++index;
            SkipUnlinked();
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            return index != other.index || map != other.map;
        }
        
        bool operator==(const Iterator& other) const {
            return index == other.index && map == other.map;
        }
        
        K& key() const { return map->keys[index]; }
        T& value() const { return map->values[index]; }
        int GetIndex() const { return index; }
    };
    
    class ConstIterator {
    private:
        const InMap* map;
        int index;
        
        void SkipUnlinked() {
            while (index < static_cast<int>(map->values.size()) && 
                   std::find(map->unlinked_indices.begin(), map->unlinked_indices.end(), index) != map->unlinked_indices.end()) {
                ++index;
            }
        }
        
    public:
        ConstIterator(const InMap* m, int i) : map(m), index(i) {
            SkipUnlinked();
        }
        
        const value_type operator*() const {
            return {map->keys[index], map->values[index]};
        }
        
        ConstIterator& operator++() {
            ++index;
            SkipUnlinked();
            return *this;
        }
        
        bool operator!=(const ConstIterator& other) const {
            return index != other.index || map != other.map;
        }
        
        bool operator==(const ConstIterator& other) const {
            return index == other.index && map == other.map;
        }
        
        const K& key() const { return map->keys[index]; }
        const T& value() const { return map->values[index]; }
        int GetIndex() const { return index; }
    };
    
    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, static_cast<int>(values.size())); }
    
    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const { return ConstIterator(this, static_cast<int>(values.size())); }
    
    // Capacity
    bool IsEmpty() const { return values.empty(); }
    int GetCount() const { return static_cast<int>(values.size() - unlinked_indices.size()); }
    int GetTotalCount() const { return static_cast<int>(values.size()); }
    
    // Modifiers
    template<class... Args>
    T& Add(const K& key, Args&&... args) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            // Key already exists, update value
            values[it->second] = T(std::forward<Args>(args)...);
            return values[it->second];
        }
        
        // Add new key-value pair
        int index = static_cast<int>(values.size());
        values.emplace_back(std::forward<Args>(args)...);
        keys.push_back(key);
        key_to_index[key] = index;
        return values[index];
    }
    
    template<class... Args>
    T& Create(Args&&... args) {
        // Create with default key
        K key{};
        return Add(key, std::forward<Args>(args)...);
    }
    
    void Erase(const K& key) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            int index = it->second;
            
            // Mark as unlinked instead of removing to maintain indices
            unlinked_indices.push_back(index);
            
            // Remove from key_to_index
            key_to_index.erase(it);
        }
    }
    
    void Erase(int index) {
        if (index >= 0 && index < static_cast<int>(values.size())) {
            // Find the key for this index
            auto key_it = std::find_if(key_to_index.begin(), key_to_index.end(),
                                     [index](const auto& pair) { return pair.second == index; });
            
            if (key_it != key_to_index.end()) {
                // Mark as unlinked
                unlinked_indices.push_back(index);
                
                // Remove from key_to_index
                key_to_index.erase(key_it);
            }
        }
    }
    
    void Clear() {
        values.clear();
        key_to_index.clear();
        keys.clear();
        unlinked_indices.clear();
    }
    
    // Lookup
    int Find(const K& key) const {
        auto it = key_to_index.find(key);
        return it != key_to_index.end() ? it->second : -1;
    }
    
    bool Has(const K& key) const {
        return key_to_index.find(key) != key_to_index.end();
    }
    
    // Element access by index
    T& operator[](int index) {
        if (index >= 0 && index < static_cast<int>(values.size())) {
            return values[index];
        }
        throw std::out_of_range("Index out of range");
    }
    
    const T& operator[](int index) const {
        if (index >= 0 && index < static_cast<int>(values.size())) {
            return values[index];
        }
        throw std::out_of_range("Index out of range");
    }
    
    const K& GetKey(int index) const {
        if (index >= 0 && index < static_cast<int>(keys.size())) {
            return keys[index];
        }
        throw std::out_of_range("Index out of range");
    }
    
    // Set operations
    void Set(int index, const T& value) {
        if (index >= 0 && index < static_cast<int>(values.size())) {
            values[index] = value;
        } else {
            throw std::out_of_range("Index out of range");
        }
    }
    
    void SetKey(int index, const K& key) {
        if (index >= 0 && index < static_cast<int>(keys.size())) {
            // Update key mapping
            auto old_key_it = std::find_if(key_to_index.begin(), key_to_index.end(),
                                        [index](const auto& pair) { return pair.second == index; });
            
            if (old_key_it != key_to_index.end()) {
                key_to_index.erase(old_key_it);
            }
            
            keys[index] = key;
            key_to_index[key] = index;
        } else {
            throw std::out_of_range("Index out of range");
        }
    }
    
    // Unlink operations (mark as deleted without shifting indices)
    void Unlink(int index) {
        if (index >= 0 && index < static_cast<int>(values.size())) {
            // Find and remove from key_to_index
            auto key_it = std::find_if(key_to_index.begin(), key_to_index.end(),
                                     [index](const auto& pair) { return pair.second == index; });
            
            if (key_it != key_to_index.end()) {
                key_to_index.erase(key_it);
            }
            
            // Add to unlinked list
            unlinked_indices.push_back(index);
        }
    }
    
    void UnlinkKey(const K& key) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            int index = it->second;
            key_to_index.erase(it);
            unlinked_indices.push_back(index);
        }
    }
    
    bool IsUnlinked(int index) const {
        return std::find(unlinked_indices.begin(), unlinked_indices.end(), index) != unlinked_indices.end();
    }
    
    // Remove unlinked items and compact
    void Sweep() {
        if (unlinked_indices.empty()) return;
        
        // Sort indices in descending order for removal
        std::sort(unlinked_indices.begin(), unlinked_indices.end(), std::greater<int>());
        
        // Remove items from the back
        for (int index : unlinked_indices) {
            if (index >= 0 && index < static_cast<int>(values.size())) {
                values.erase(values.begin() + index);
                keys.erase(keys.begin() + index);
                
                // Update all indices in key_to_index that are greater than the removed index
                for (auto& pair : key_to_index) {
                    if (pair.second > index) {
                        pair.second--;
                    }
                }
            }
        }
        
        unlinked_indices.clear();
    }
    
    // Memory management
    void Reserve(int count) {
        values.reserve(count);
        keys.reserve(count);
    }
    
    void Shrink() {
        values.shrink_to_fit();
        keys.shrink_to_fit();
        unlinked_indices.shrink_to_fit();
    }
    
    // Comparison
    template<class U>
    bool operator==(const U& other) const {
        return IsEqualRange(*this, other);
    }
    
    template<class U>
    bool operator!=(const U& other) const {
        return !(*this == other);
    }
    
    // Serialization support
    template<class Stream>
    void Serialize(Stream& s) {
        s % values % key_to_index % keys;
        if (s.IsLoading()) {
            unlinked_indices.clear();
        }
    }
    
    // Utilities
    std::vector<K> GetKeys() const {
        std::vector<K> result;
        result.reserve(keys.size() - unlinked_indices.size());
        
        for (size_t i = 0; i < keys.size(); ++i) {
            if (std::find(unlinked_indices.begin(), unlinked_indices.end(), static_cast<int>(i)) == unlinked_indices.end()) {
                result.push_back(keys[i]);
            }
        }
        
        return result;
    }
    
    std::vector<T> GetValues() const {
        std::vector<T> result;
        result.reserve(values.size() - unlinked_indices.size());
        
        for (size_t i = 0; i < values.size(); ++i) {
            if (std::find(unlinked_indices.begin(), unlinked_indices.end(), static_cast<int>(i)) == unlinked_indices.end()) {
                result.push_back(values[i]);
            }
        }
        
        return result;
    }
    
    // Swap
    void Swap(InMap& other) {
        values.swap(other.values);
        key_to_index.swap(other.key_to_index);
        keys.swap(other.keys);
        unlinked_indices.swap(other.unlinked_indices);
    }
    
    // String representation
    std::string ToString() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& pair : *this) {
            if (!first) oss << ", ";
            oss << pair.first << ": " << pair.second;
            first = false;
        }
        oss << "}";
        return oss.str();
    }
};

// Global swap function
template<class K, class T>
void Swap(InMap<K, T>& a, InMap<K, T>& b) {
    a.Swap(b);
}

// Streaming operator
template<class Stream, class K, class T>
void operator%(Stream& s, InMap<K, T>& map) {
    map.Serialize(s);
}

// String conversion
template<class K, class T>
std::string AsString(const InMap<K, T>& map) {
    return map.ToString();
}

#endif