#pragma once
#ifndef _Core_InVector_h_
#define _Core_InVector_h_

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Core.h"

// InVector - indirect vector implementation for stdsrc
// Stores pointers to objects rather than objects directly for efficiency

template <class T>
class InVector {
private:
    std::vector<std::unique_ptr<T>> items;
    
public:
    // Typedefs
    typedef T ValueType;
    typedef T* PointerType;
    typedef const T* ConstPointerType;
    
    // Constructors
    InVector() = default;
    
    InVector(const InVector& other) {
        items.reserve(other.items.size());
        for (const auto& item : other.items) {
            if (item) {
                items.push_back(std::make_unique<T>(*item));
            } else {
                items.push_back(nullptr);
            }
        }
    }
    
    InVector(InVector&& other) noexcept 
        : items(std::move(other.items)) {}
    
    InVector& operator=(const InVector& other) {
        if (this != &other) {
            items.clear();
            items.reserve(other.items.size());
            for (const auto& item : other.items) {
                if (item) {
                    items.push_back(std::make_unique<T>(*item));
                } else {
                    items.push_back(nullptr);
                }
            }
        }
        return *this;
    }
    
    InVector& operator=(InVector&& other) noexcept {
        if (this != &other) {
            items = std::move(other.items);
        }
        return *this;
    }
    
    // Destructor
    ~InVector() = default;
    
    // Element access
    T& operator[](int index) {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        if (!items[index]) {
            throw std::runtime_error("Accessing null pointer");
        }
        return *items[index];
    }
    
    const T& operator[](int index) const {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        if (!items[index]) {
            throw std::runtime_error("Accessing null pointer");
        }
        return *items[index];
    }
    
    T* At(int index) {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            return nullptr;
        }
        return items[index].get();
    }
    
    const T* At(int index) const {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            return nullptr;
        }
        return items[index].get();
    }
    
    T& Get(int index) {
        T* ptr = At(index);
        if (!ptr) {
            throw std::runtime_error("Accessing null pointer");
        }
        return *ptr;
    }
    
    const T& Get(int index) const {
        const T* ptr = At(index);
        if (!ptr) {
            throw std::runtime_error("Accessing null pointer");
        }
        return *ptr;
    }
    
    // Safe access with default value
    T& Get(int index, T& default_value) {
        T* ptr = At(index);
        return ptr ? *ptr : default_value;
    }
    
    const T& Get(int index, const T& default_value) const {
        const T* ptr = At(index);
        return ptr ? *ptr : default_value;
    }
    
    // Adding elements
    template<typename... Args>
    T& Add(Args&&... args) {
        items.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *items.back();
    }
    
    T& Add(const T& item) {
        items.push_back(std::make_unique<T>(item));
        return *items.back();
    }
    
    T& Add(T&& item) {
        items.push_back(std::make_unique<T>(std::move(item)));
        return *items.back();
    }
    
    T* Add() {
        items.push_back(std::make_unique<T>());
        return items.back().get();
    }
    
    void Add(T* item) {
        items.push_back(std::unique_ptr<T>(item));
    }
    
    // Emplace
    template<typename... Args>
    T& Emplace(Args&&... args) {
        items.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *items.back();
    }
    
    // Insertion
    template<typename... Args>
    T& Insert(int index, Args&&... args) {
        if (index < 0 || index > static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        items.insert(items.begin() + index, std::make_unique<T>(std::forward<Args>(args)...));
        return *items[index];
    }
    
    T& Insert(int index, const T& item) {
        if (index < 0 || index > static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        items.insert(items.begin() + index, std::make_unique<T>(item));
        return *items[index];
    }
    
    void Insert(int index, T* item) {
        if (index < 0 || index > static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        items.insert(items.begin() + index, std::unique_ptr<T>(item));
    }
    
    // Removal
    void Remove(int index) {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            throw std::out_of_range("Index out of range");
        }
        items.erase(items.begin() + index);
    }
    
    void Remove(int index, int count) {
        if (index < 0 || index >= static_cast<int>(items.size()) || count < 0) {
            throw std::out_of_range("Index or count out of range");
        }
        if (index + count > static_cast<int>(items.size())) {
            throw std::out_of_range("Range exceeds vector bounds");
        }
        items.erase(items.begin() + index, items.begin() + index + count);
    }
    
    // Remove by value
    int Remove(const T& item) {
        int removed = 0;
        for (auto it = items.begin(); it != items.end(); ) {
            if (*it && **it == item) {
                it = items.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }
    
    // Remove if predicate
    template<typename Predicate>
    int RemoveIf(Predicate pred) {
        int removed = 0;
        for (auto it = items.begin(); it != items.end(); ) {
            if (*it && pred(**it)) {
                it = items.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }
    
    // Clear
    void Clear() {
        items.clear();
    }
    
    // Size
    int GetCount() const {
        return static_cast<int>(items.size());
    }
    
    bool IsEmpty() const {
        return items.empty();
    }
    
    // Capacity
    void Reserve(int capacity) {
        items.reserve(capacity);
    }
    
    void SetCount(int count) {
        if (count < 0) {
            throw std::invalid_argument("Count cannot be negative");
        }
        
        if (count > static_cast<int>(items.size())) {
            items.resize(count);
        } else if (count < static_cast<int>(items.size())) {
            items.resize(count);
        }
    }
    
    void SetCount(int count, const T& init) {
        if (count < 0) {
            throw std::invalid_argument("Count cannot be negative");
        }
        
        if (count > static_cast<int>(items.size())) {
            items.reserve(count);
            for (int i = static_cast<int>(items.size()); i < count; ++i) {
                items.push_back(std::make_unique<T>(init));
            }
        } else if (count < static_cast<int>(items.size())) {
            items.resize(count);
        }
    }
    
    int GetAlloc() const {
        return static_cast<int>(items.capacity());
    }
    
    void Shrink() {
        items.shrink_to_fit();
    }
    
    // Iterators
    class Iterator {
    private:
        typename std::vector<std::unique_ptr<T>>::iterator it;
        
    public:
        Iterator(typename std::vector<std::unique_ptr<T>>::iterator i) : it(i) {}
        
        T& operator*() { return **it; }
        T* operator->() { return it->get(); }
        
        Iterator& operator++() { ++it; return *this; }
        Iterator& operator--() { --it; return *this; }
        
        bool operator==(const Iterator& other) const { return it == other.it; }
        bool operator!=(const Iterator& other) const { return it != other.it; }
        
        int GetIndex() const { return static_cast<int>(it - items.begin()); }
    };
    
    class ConstIterator {
    private:
        typename std::vector<std::unique_ptr<T>>::const_iterator it;
        
    public:
        ConstIterator(typename std::vector<std::unique_ptr<T>>::const_iterator i) : it(i) {}
        
        const T& operator*() const { return **it; }
        const T* operator->() const { return it->get(); }
        
        ConstIterator& operator++() { ++it; return *this; }
        ConstIterator& operator--() { --it; return *this; }
        
        bool operator==(const ConstIterator& other) const { return it == other.it; }
        bool operator!=(const ConstIterator& other) const { return it != other.it; }
        
        int GetIndex() const { return static_cast<int>(it - items.begin()); }
    };
    
    Iterator begin() { return Iterator(items.begin()); }
    Iterator end() { return Iterator(items.end()); }
    
    ConstIterator begin() const { return ConstIterator(items.begin()); }
    ConstIterator end() const { return ConstIterator(items.end()); }
    
    ConstIterator cbegin() const { return ConstIterator(items.cbegin()); }
    ConstIterator cend() const { return ConstIterator(items.cend()); }
    
    // Find operations
    int Find(const T& item) const {
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i] && *items[i] == item) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
    
    int FindIndex(std::function<bool(const T&)> predicate) const {
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i] && predicate(*items[i])) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
    
    // Contains
    bool Contains(const T& item) const {
        return Find(item) >= 0;
    }
    
    // Sort
    void Sort() {
        std::sort(items.begin(), items.end(), 
                  [](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) {
                      if (!a && !b) return false;
                      if (!a) return true;
                      if (!b) return false;
                      return *a < *b;
                  });
    }
    
    template<typename Compare>
    void Sort(Compare comp) {
        std::sort(items.begin(), items.end(), 
                  [comp](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) {
                      if (!a && !b) return false;
                      if (!a) return true;
                      if (!b) return false;
                      return comp(*a, *b);
                  });
    }
    
    // Reverse
    void Reverse() {
        std::reverse(items.begin(), items.end());
    }
    
    // Unique (remove duplicates)
    void Unique() {
        std::sort(items.begin(), items.end(), 
                  [](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) {
                      if (!a && !b) return false;
                      if (!a) return true;
                      if (!b) return false;
                      return *a < *b;
                  });
        
        items.erase(std::unique(items.begin(), items.end(), 
                                [](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) {
                                    if (!a && !b) return true;
                                    if (!a || !b) return false;
                                    return *a == *b;
                                }), items.end());
    }
    
    // Apply function to all elements
    template<typename Function>
    void Apply(Function func) {
        for (auto& item : items) {
            if (item) {
                func(*item);
            }
        }
    }
    
    // Transform all elements
    template<typename Function>
    void Transform(Function func) {
        for (auto& item : items) {
            if (item) {
                *item = func(std::move(*item));
            }
        }
    }
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        int count = GetCount();
        s / count;
        
        if (s.IsLoading()) {
            Clear();
            SetCount(count);
            for (int i = 0; i < count; ++i) {
                if (items[i]) {
                    s % *items[i];
                }
            }
        } else {
            for (int i = 0; i < count; ++i) {
                if (items[i]) {
                    s % *items[i];
                }
            }
        }
    }
    
    // Swap
    void Swap(InVector& other) {
        items.swap(other.items);
    }
    
    // Pick (move semantics)
    InVector Pick() {
        InVector result;
        result.items = std::move(items);
        return result;
    }
    
    // String representation
    std::string ToString() const {
        std::ostringstream oss;
        oss << "[";
        bool first = true;
        for (const auto& item : items) {
            if (!first) oss << ", ";
            if (item) {
                oss << *item;
            } else {
                oss << "nullptr";
            }
            first = false;
        }
        oss << "]";
        return oss.str();
    }
};

// Global swap function
template<class T>
void Swap(InVector<T>& a, InVector<T>& b) {
    a.Swap(b);
}

// Streaming operator
template<typename Stream, class T>
void operator%(Stream& s, InVector<T>& vec) {
    vec.Serialize(s);
}

// String conversion
template<class T>
std::string AsString(const InVector<T>& vec) {
    return vec.ToString();
}

#endif