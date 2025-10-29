#pragma once
#ifndef _Core_Sorted_h_
#define _Core_Sorted_h_

#include <vector>
#include <algorithm>
#include <functional>
#include "Core.h"

// SortedArray - Maintains sorted order after each insertion
template <typename T, typename Compare = std::less<T>>
class SortedArray {
private:
    std::vector<T> data;
    Compare comp;
    
public:
    explicit SortedArray(const Compare& c = Compare()) : comp(c) {}
    
    // Insert element while maintaining sorted order
    void Add(const T& item) {
        auto pos = std::lower_bound(data.begin(), data.end(), item, comp);
        data.insert(pos, item);
    }
    
    void Add(T&& item) {
        auto pos = std::lower_bound(data.begin(), data.end(), item, comp);
        data.insert(pos, std::move(item));
    }
    
    // Insert multiple elements at once, then sort (more efficient for batch operations)
    template <typename InputIt>
    void AddRange(InputIt first, InputIt last) {
        data.insert(data.end(), first, last);
        std::sort(data.begin(), data.end(), comp);
    }
    
    // Find element using binary search
    int Find(const T& item) const {
        auto pos = std::lower_bound(data.begin(), data.end(), item, comp);
        if (pos != data.end() && !comp(item, *pos)) {
            return pos - data.begin();
        }
        return -1;
    }
    
    // Find or insert
    int FindAdd(const T& item) {
        int pos = Find(item);
        if (pos < 0) {
            Add(item);
            pos = Find(item);
        }
        return pos;
    }
    
    // Remove element
    bool Remove(const T& item) {
        int pos = Find(item);
        if (pos >= 0) {
            data.erase(data.begin() + pos);
            return true;
        }
        return false;
    }
    
    // Remove element at index
    void RemoveAt(int index) {
        if (index >= 0 && index < GetCount()) {
            data.erase(data.begin() + index);
        }
    }
    
    // Get element at index
    const T& operator[](int index) const {
        return data[index];
    }
    
    T& operator[](int index) {
        return data[index];
    }
    
    const T& Get(int index) const {
        return data[index];
    }
    
    T& Get(int index) {
        return data[index];
    }
    
    // Get count
    int GetCount() const {
        return static_cast<int>(data.size());
    }
    
    bool IsEmpty() const {
        return data.empty();
    }
    
    // Clear all elements
    void Clear() {
        data.clear();
    }
    
    // Get underlying vector (const)
    const std::vector<T>& GetVector() const {
        return data;
    }
    
    // Get underlying vector (non-const)
    std::vector<T>& GetVector() {
        return data;
    }
    
    // Binary search for position where element would be inserted
    int FindInsertPos(const T& item) const {
        auto pos = std::lower_bound(data.begin(), data.end(), item, comp);
        return pos - data.begin();
    }
    
    // Check if array contains element
    bool Contains(const T& item) const {
        return Find(item) >= 0;
    }
    
    // Iterator support
    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
    typename std::vector<T>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T>::const_iterator end() const { return data.end(); }
    
    // Swap with another SortedArray
    void Swap(SortedArray& other) {
        data.swap(other.data);
    }
};

// SortedIndex - Maintains sorted indices of elements
template <typename T, typename Compare = std::less<T>>
class SortedIndex {
private:
    std::vector<int> sorted_indices; // indices into external array
    const std::vector<T>* elements;
    Compare comp;
    
    void RebuildIndex() {
        sorted_indices.resize(elements->size());
        for (size_t i = 0; i < elements->size(); ++i) {
            sorted_indices[i] = i;
        }
        std::sort(sorted_indices.begin(), sorted_indices.end(), [this](int a, int b) {
            return comp((*elements)[a], (*elements)[b]);
        });
    }
    
public:
    explicit SortedIndex(const std::vector<T>* source, const Compare& c = Compare()) 
        : elements(source), comp(c) {
        RebuildIndex();
    }
    
    // Refresh indices after source data changes
    void Refresh() {
        RebuildIndex();
    }
    
    // Find index of element in source array
    int Find(const T& item) const {
        for (int idx : sorted_indices) {
            if (!comp((*elements)[idx], item) && !comp(item, (*elements)[idx])) {
                return idx;
            }
        }
        return -1;
    }
    
    // Get index at sorted position
    int GetIndex(int sorted_pos) const {
        if (sorted_pos >= 0 && sorted_pos < GetCount()) {
            return sorted_indices[sorted_pos];
        }
        return -1;
    }
    
    // Get element at sorted position
    const T& Get(int sorted_pos) const {
        int idx = GetIndex(sorted_pos);
        return (*elements)[idx];
    }
    
    int GetCount() const {
        return static_cast<int>(sorted_indices.size());
    }
    
    bool IsEmpty() const {
        return sorted_indices.empty();
    }
};

#endif