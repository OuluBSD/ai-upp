#pragma once
#ifndef _Core_Huge_h_
#define _Core_Huge_h_

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "Core.h"

// Huge data structures - for handling very large data sets efficiently
// These classes are designed to handle data that may not fit in memory

// Huge vector - stores data in chunks/pages to avoid memory fragmentation
template <class T, size_t PageSize = 1024 * 1024> // 1MB pages by default
class HugeVector {
private:
    struct Page {
        std::vector<T> data;
        bool is_dirty;
        
        Page() : is_dirty(false) {
            data.reserve(PageSize);
        }
        
        void Resize(size_t size) {
            data.resize(size);
            is_dirty = true;
        }
        
        void Clear() {
            data.clear();
            is_dirty = true;
        }
        
        bool IsDirty() const { return is_dirty; }
        void MarkClean() { is_dirty = false; }
    };
    
    std::vector<std::unique_ptr<Page>> pages;
    size_t total_count;
    size_t page_count;
    
    size_t GetPageIndex(size_t index) const {
        return index / PageSize;
    }
    
    size_t GetPageOffset(size_t index) const {
        return index % PageSize;
    }
    
    Page* GetOrCreatePage(size_t page_index) {
        if (page_index >= pages.size()) {
            pages.resize(page_index + 1);
        }
        
        if (!pages[page_index]) {
            pages[page_index] = std::make_unique<Page>();
        }
        
        return pages[page_index].get();
    }
    
    const Page* GetPage(size_t page_index) const {
        if (page_index >= pages.size() || !pages[page_index]) {
            return nullptr;
        }
        return pages[page_index].get();
    }
    
public:
    HugeVector() : total_count(0), page_count(0) {}
    
    ~HugeVector() = default;
    
    // Element access
    T& operator[](size_t index) {
        if (index >= total_count) {
            throw std::out_of_range("Index out of range");
        }
        
        size_t page_index = GetPageIndex(index);
        size_t page_offset = GetPageOffset(index);
        
        Page* page = GetOrCreatePage(page_index);
        if (page_offset >= page->data.size()) {
            throw std::out_of_range("Index out of range");
        }
        
        return page->data[page_offset];
    }
    
    const T& operator[](size_t index) const {
        if (index >= total_count) {
            throw std::out_of_range("Index out of range");
        }
        
        size_t page_index = GetPageIndex(index);
        size_t page_offset = GetPageOffset(index);
        
        const Page* page = GetPage(page_index);
        if (!page || page_offset >= page->data.size()) {
            throw std::out_of_range("Index out of range");
        }
        
        return page->data[page_offset];
    }
    
    // Size management
    size_t GetCount() const {
        return total_count;
    }
    
    bool IsEmpty() const {
        return total_count == 0;
    }
    
    void SetCount(size_t count) {
        if (count == total_count) return;
        
        if (count < total_count) {
            // Shrinking
            size_t new_page_count = (count + PageSize - 1) / PageSize;
            size_t last_page_items = count % PageSize;
            
            // Clear extra pages
            for (size_t i = new_page_count; i < pages.size(); ++i) {
                if (pages[i]) {
                    pages[i]->Clear();
                }
            }
            
            // Resize last page if needed
            if (new_page_count > 0 && new_page_count <= pages.size() && pages[new_page_count - 1]) {
                pages[new_page_count - 1]->Resize(last_page_items);
            }
        } else {
            // Growing - just update counts
            size_t new_page_count = (count + PageSize - 1) / PageSize;
            size_t last_page_items = count % PageSize;
            
            // Ensure pages exist
            if (new_page_count > pages.size()) {
                pages.resize(new_page_count);
            }
            
            // Create pages as needed
            for (size_t i = 0; i < new_page_count; ++i) {
                if (!pages[i]) {
                    pages[i] = std::make_unique<Page>();
                }
            }
        }
        
        total_count = count;
        page_count = (count + PageSize - 1) / PageSize;
    }
    
    void Clear() {
        for (auto& page : pages) {
            if (page) {
                page->Clear();
            }
        }
        total_count = 0;
        page_count = 0;
    }
    
    // Adding elements
    void Add(const T& item) {
        size_t page_index = total_count / PageSize;
        size_t page_offset = total_count % PageSize;
        
        Page* page = GetOrCreatePage(page_index);
        if (page->data.size() <= page_offset) {
            page->data.resize(page_offset + 1);
        }
        
        page->data[page_offset] = item;
        total_count++;
        
        if (page_offset == 0) {
            page_count++;
        }
    }
    
    void Add(T&& item) {
        size_t page_index = total_count / PageSize;
        size_t page_offset = total_count % PageSize;
        
        Page* page = GetOrCreatePage(page_index);
        if (page->data.size() <= page_offset) {
            page->data.resize(page_offset + 1);
        }
        
        page->data[page_offset] = std::move(item);
        total_count++;
        
        if (page_offset == 0) {
            page_count++;
        }
    }
    
    template<typename... Args>
    void AddNew(Args&&... args) {
        size_t page_index = total_count / PageSize;
        size_t page_offset = total_count % PageSize;
        
        Page* page = GetOrCreatePage(page_index);
        if (page->data.size() <= page_offset) {
            page->data.resize(page_offset + 1);
        }
        
        page->data[page_offset] = T(std::forward<Args>(args)...);
        total_count++;
        
        if (page_offset == 0) {
            page_count++;
        }
    }
    
    // Removing elements
    void Remove(size_t index) {
        if (index >= total_count) {
            throw std::out_of_range("Index out of range");
        }
        
        // For simplicity, we'll just shrink the vector
        // A more efficient implementation would move elements
        if (index < total_count - 1) {
            // Move elements
            for (size_t i = index; i < total_count - 1; ++i) {
                operator[](i) = std::move(operator[](i + 1));
            }
        }
        
        SetCount(total_count - 1);
    }
    
    void Remove(size_t index, size_t count) {
        if (index >= total_count || index + count > total_count) {
            throw std::out_of_range("Index out of range");
        }
        
        // Move remaining elements
        for (size_t i = index; i + count < total_count; ++i) {
            operator[](i) = std::move(operator[](i + count));
        }
        
        SetCount(total_count - count);
    }
    
    // Iteration support
    class Iterator {
    private:
        HugeVector* vector;
        size_t index;
        
    public:
        Iterator(HugeVector* v, size_t i) : vector(v), index(i) {}
        
        T& operator*() { return (*vector)[index]; }
        T* operator->() { return &(*vector)[index]; }
        
        Iterator& operator++() { ++index; return *this; }
        Iterator& operator--() { --index; return *this; }
        
        bool operator==(const Iterator& other) const { return vector == other.vector && index == other.index; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        
        Iterator operator+(size_t n) const { return Iterator(vector, index + n); }
        Iterator operator-(size_t n) const { return Iterator(vector, index - n); }
        
        size_t GetIndex() const { return index; }
    };
    
    class ConstIterator {
    private:
        const HugeVector* vector;
        size_t index;
        
    public:
        ConstIterator(const HugeVector* v, size_t i) : vector(v), index(i) {}
        
        const T& operator*() const { return (*vector)[index]; }
        const T* operator->() const { return &(*vector)[index]; }
        
        ConstIterator& operator++() { ++index; return *this; }
        ConstIterator& operator--() { --index; return *this; }
        
        bool operator==(const ConstIterator& other) const { return vector == other.vector && index == other.index; }
        bool operator!=(const ConstIterator& other) const { return !(*this == other); }
        
        ConstIterator operator+(size_t n) const { return ConstIterator(vector, index + n); }
        ConstIterator operator-(size_t n) const { return ConstIterator(vector, index - n); }
        
        size_t GetIndex() const { return index; }
    };
    
    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, total_count); }
    
    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const { return ConstIterator(this, total_count); }
    
    // Utility functions
    size_t GetPageSize() const { return PageSize; }
    size_t GetPageCount() const { return page_count; }
    
    // Find functions
    template<typename Predicate>
    size_t FindIndex(Predicate pred) const {
        for (size_t i = 0; i < total_count; ++i) {
            if (pred(operator[](i))) {
                return i;
            }
        }
        return NPOS;
    }
    
    template<typename U>
    size_t Find(const U& value) const {
        return FindIndex([&value](const T& item) { return item == value; });
    }
    
    // Sort function
    template<typename Compare = std::less<T>>
    void Sort(Compare comp = Compare{}) {
        // For huge vectors, we might want to use external sorting
        // For now, we'll use std::sort on each page separately
        // A more sophisticated implementation would handle cross-page sorting
        
        for (auto& page : pages) {
            if (page && !page->data.empty()) {
                std::sort(page->data.begin(), page->data.end(), comp);
            }
        }
    }
    
    // Memory management
    void Shrink() {
        // Shrink pages to fit their data
        for (auto& page : pages) {
            if (page && !page->data.empty()) {
                page->data.shrink_to_fit();
            }
        }
    }
    
    void Reserve(size_t count) {
        // Reserve space for pages
        size_t required_pages = (count + PageSize - 1) / PageSize;
        if (required_pages > pages.size()) {
            pages.resize(required_pages);
        }
    }
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        size_t count = total_count;
        s / count;
        if (s.IsLoading()) {
            SetCount(count);
        }
        
        // Serialize page by page
        for (size_t i = 0; i < count; ++i) {
            s % operator[](i);
        }
    }
    
    // Comparison operators
    template<typename U>
    bool operator==(const U& other) const {
        if (GetCount() != other.GetCount()) return false;
        for (size_t i = 0; i < GetCount(); ++i) {
            if (operator[](i) != other[i]) return false;
        }
        return true;
    }
    
    template<typename U>
    bool operator!=(const U& other) const {
        return !(*this == other);
    }
};

// Huge string - efficient string handling for very large strings
class HugeString {
private:
    HugeVector<char> data;
    
public:
    HugeString() = default;
    explicit HugeString(size_t size) { data.SetCount(size); }
    HugeString(const std::string& str) {
        data.SetCount(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            data[i] = str[i];
        }
    }
    
    HugeString(const char* str) {
        if (str) {
            size_t len = strlen(str);
            data.SetCount(len);
            for (size_t i = 0; i < len; ++i) {
                data[i] = str[i];
            }
        }
    }
    
    // Element access
    char& operator[](size_t index) { return data[index]; }
    const char& operator[](size_t index) const { return data[index]; }
    
    char& At(size_t index) { return data[index]; }
    const char& At(size_t index) const { return data[index]; }
    
    // String operations
    size_t GetLength() const { return data.GetCount(); }
    size_t GetCount() const { return data.GetCount(); }
    bool IsEmpty() const { return data.IsEmpty(); }
    
    void SetLength(size_t length) { data.SetCount(length); }
    void Clear() { data.Clear(); }
    
    // Append operations
    void Cat(char c) { data.Add(c); }
    void Cat(const std::string& str) {
        size_t old_size = data.GetCount();
        data.SetCount(old_size + str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            data[old_size + i] = str[i];
        }
    }
    
    void Cat(const char* str) {
        if (str) {
            size_t len = strlen(str);
            size_t old_size = data.GetCount();
            data.SetCount(old_size + len);
            for (size_t i = 0; i < len; ++i) {
                data[old_size + i] = str[i];
            }
        }
    }
    
    // Conversion to std::string
    std::string ToString() const {
        std::string result;
        result.reserve(data.GetCount());
        for (size_t i = 0; i < data.GetCount(); ++i) {
            result += data[i];
        }
        return result;
    }
    
    // C-string interface
    const char* Begin() const {
        // This is problematic for huge strings as we can't guarantee contiguous memory
        // For now, we'll return nullptr to indicate this isn't supported
        return nullptr;
    }
    
    const char* End() const {
        return nullptr;
    }
    
    // Iterator support
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    
    // Comparison operators
    bool operator==(const std::string& other) const {
        if (GetLength() != other.size()) return false;
        for (size_t i = 0; i < GetLength(); ++i) {
            if (data[i] != other[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const std::string& other) const {
        return !(*this == other);
    }
    
    bool operator==(const char* other) const {
        if (!other) return IsEmpty();
        size_t len = strlen(other);
        if (GetLength() != len) return false;
        for (size_t i = 0; i < GetLength(); ++i) {
            if (data[i] != other[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const char* other) const {
        return !(*this == other);
    }
    
    // Assignment operators
    HugeString& operator=(const std::string& str) {
        data.SetCount(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            data[i] = str[i];
        }
        return *this;
    }
    
    HugeString& operator=(const char* str) {
        if (str) {
            size_t len = strlen(str);
            data.SetCount(len);
            for (size_t i = 0; i < len; ++i) {
                data[i] = str[i];
            }
        } else {
            data.Clear();
        }
        return *this;
    }
    
    // Implicit conversion to std::string
    operator std::string() const {
        return ToString();
    }
};

// Streaming operators
template<typename Stream, typename T, size_t PageSize>
void operator%(Stream& s, HugeVector<T, PageSize>& vec) {
    vec.Serialize(s);
}

template<typename Stream>
void operator%(Stream& s, HugeString& str) {
    size_t len = str.GetLength();
    s / len;
    if (s.IsLoading()) {
        str.SetLength(len);
    }
    for (size_t i = 0; i < len; ++i) {
        s % str[i];
    }
}

#endif