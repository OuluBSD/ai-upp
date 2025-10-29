#pragma once
#ifndef _Core_InVector_h_
#define _Core_InVector_h_

#include <vector>
#include <memory>
#include <algorithm>
#include "Core.h"

// InVector - A vector that maintains position-based indices for elements
template <typename T>
class InVector {
private:
    std::vector<T> data;
    
public:
    InVector() = default;
    
    // Initializer list constructor
    InVector(std::initializer_list<T> init) : data(init) {}
    
    // Check if empty
    bool IsEmpty() const { return data.empty(); }
    
    // Get count
    int GetCount() const { return static_cast<int>(data.size()); }
    
    // Clear all elements
    void Clear() { data.clear(); }
    
    // Get element at index
    const T& operator[](int index) const { 
        ASSERT(index >= 0 && index < GetCount()); 
        return data[index]; 
    }
    
    T& operator[](int index) { 
        ASSERT(index >= 0 && index < GetCount()); 
        return data[index]; 
    }
    
    const T& Get(int index) const { 
        return operator[](index); 
    }
    
    T& Get(int index) { 
        return operator[](index); 
    }
    
    // Get last element
    const T& Top() const { 
        ASSERT(!IsEmpty()); 
        return data.back(); 
    }
    
    T& Top() { 
        ASSERT(!IsEmpty()); 
        return data.back(); 
    }
    
    // Add element at end
    void Add(const T& item) { 
        data.push_back(item); 
    }
    
    void Add(T&& item) { 
        data.push_back(std::move(item)); 
    }
    
    // Add multiple elements
    template <typename... Args>
    void Add(const T& item, Args&&... args) {
        Add(item);
        Add(std::forward<Args>(args)...);
    }
    
    // Add and return index
    int AddAt(const T& item) {
        int index = GetCount();
        Add(item);
        return index;
    }
    
    int AddAt(T&& item) {
        int index = GetCount();
        Add(std::move(item));
        return index;
    }
    
    // Insert element at position
    void Insert(int pos, const T& item) {
        ASSERT(pos >= 0 && pos <= GetCount());
        data.insert(data.begin() + pos, item);
    }
    
    void Insert(int pos, T&& item) {
        ASSERT(pos >= 0 && pos <= GetCount());
        data.insert(data.begin() + pos, std::move(item));
    }
    
    // Remove element at index
    void Remove(int index) {
        ASSERT(index >= 0 && index < GetCount());
        data.erase(data.begin() + index);
    }
    
    // Remove range [from, to)
    void Remove(int from, int to) {
        ASSERT(from >= 0 && to <= GetCount() && from <= to);
        data.erase(data.begin() + from, data.begin() + to);
    }
    
    // Remove last element
    void RemoveLast() {
        ASSERT(!IsEmpty());
        data.pop_back();
    }
    
    // Remove element without preserving order (faster than Remove)
    void RemoveIndex(int index) {
        ASSERT(index >= 0 && index < GetCount());
        if (index == GetCount() - 1) {
            RemoveLast();
        } else {
            std::swap(data[index], data.back());
            RemoveLast();
        }
    }
    
    // Find element
    int Find(const T& item) const {
        for (int i = 0; i < GetCount(); i++) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    // Find with predicate
    template<typename Predicate>
    int Find(Predicate pred) const {
        for (int i = 0; i < GetCount(); i++) {
            if (pred(data[i])) {
                return i;
            }
        }
        return -1;
    }
    
    // Contains check
    bool Contains(const T& item) const {
        return Find(item) >= 0;
    }
    
    // Find or add
    int FindAdd(const T& item) {
        int pos = Find(item);
        if (pos < 0) {
            pos = AddAt(item);
        }
        return pos;
    }
    
    // Set element at index
    void Set(int index, const T& item) {
        ASSERT(index >= 0 && index < GetCount());
        data[index] = item;
    }
    
    void Set(int index, T&& item) {
        ASSERT(index >= 0 && index < GetCount());
        data[index] = std::move(item);
    }
    
    // Set count (resize)
    void SetCount(int count) {
        if (count < 0) count = 0;
        data.resize(count);
    }
    
    // Reserve capacity
    void Reserve(int capacity) {
        data.reserve(capacity);
    }
    
    // Get capacity
    int GetCapacity() const {
        return static_cast<int>(data.capacity());
    }
    
    // Shrink to fit actual size
    void Shrink() {
        data.shrink_to_fit();
    }
    
    // Find position of first element that matches predicate or add it
    template<typename Predicate>
    int FindAdd(Predicate pred, const T& item) {
        int pos = Find(pred);
        if (pos < 0) {
            pos = AddAt(item);
        }
        return pos;
    }
    
    // Sort using default comparison
    void Sort() {
        std::sort(data.begin(), data.end());
    }
    
    // Sort using custom comparison
    template<typename Compare>
    void Sort(Compare comp) {
        std::sort(data.begin(), data.end(), comp);
    }
    
    // Iterator support
    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
    typename std::vector<T>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T>::const_iterator end() const { return data.end(); }
};

#endif