#pragma once
#ifndef _Core_Heap_h_
#define _Core_Heap_h_

#include <cstdlib>
#include <cstdint>
#include <new>
#include <memory>
#include <atomic>
#include <mutex>
#include "Core.h"

// Custom heap implementation for stdsrc
// Provides memory allocation with optional debugging features

// Heap statistics structure
struct HeapStatistics {
    std::atomic<size_t> total_allocated{0};
    std::atomic<size_t> peak_allocated{0};
    std::atomic<size_t> total_deallocated{0};
    std::atomic<size_t> allocation_count{0};
    std::atomic<size_t> deallocation_count{0};
    
    void Reset() {
        total_allocated = 0;
        peak_allocated = 0;
        total_deallocated = 0;
        allocation_count = 0;
        deallocation_count = 0;
    }
};

// Global heap statistics
extern HeapStatistics heap_stats;

// Memory allocation functions
inline void* HeapMalloc(size_t size) {
    if (size == 0) return nullptr;
    
    void* ptr = std::malloc(size);
    if (ptr) {
        size_t current = heap_stats.total_allocated.fetch_add(size) + size;
        heap_stats.allocation_count.fetch_add(1);
        
        // Update peak allocation
        size_t peak = heap_stats.peak_allocated.load();
        while (current > peak) {
            if (heap_stats.peak_allocated.compare_exchange_weak(peak, current)) {
                break;
            }
        }
    }
    
    return ptr;
}

inline void HeapFree(void* ptr, size_t size = 0) {
    if (ptr) {
        std::free(ptr);
        if (size > 0) {
            heap_stats.total_deallocated.fetch_add(size);
            heap_stats.deallocation_count.fetch_add(1);
        }
    }
}

inline void* HeapRealloc(void* ptr, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        HeapFree(ptr, old_size);
        return nullptr;
    }
    
    void* new_ptr = std::realloc(ptr, new_size);
    if (new_ptr) {
        if (ptr) {
            // Reallocation - adjust stats
            heap_stats.total_deallocated.fetch_add(old_size);
            heap_stats.deallocation_count.fetch_add(1);
        }
        
        size_t current = heap_stats.total_allocated.fetch_add(new_size) + new_size;
        heap_stats.allocation_count.fetch_add(1);
        
        // Update peak allocation
        size_t peak = heap_stats.peak_allocated.load();
        while (current > peak) {
            if (heap_stats.peak_allocated.compare_exchange_weak(peak, current)) {
                break;
            }
        }
    }
    
    return new_ptr;
}

inline void* HeapCalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    if (total_size == 0) return nullptr;
    if (num != 0 && total_size / num != size) return nullptr; // Overflow check
    
    void* ptr = std::calloc(num, size);
    if (ptr) {
        size_t current = heap_stats.total_allocated.fetch_add(total_size) + total_size;
        heap_stats.allocation_count.fetch_add(1);
        
        // Update peak allocation
        size_t peak = heap_stats.peak_allocated.load();
        while (current > peak) {
            if (heap_stats.peak_allocated.compare_exchange_weak(peak, current)) {
                break;
            }
        }
    }
    
    return ptr;
}

// Alignment-aware allocation
inline void* HeapAlignedMalloc(size_t size, size_t alignment) {
    if (size == 0) return nullptr;
    
#if defined(_MSC_VER)
    void* ptr = _aligned_malloc(size, alignment);
#elif defined(__GNUC__) || defined(__clang__)
    void* ptr = std::aligned_alloc(alignment, size);
#else
    // Fallback implementation
    void* raw_ptr = std::malloc(size + alignment - 1 + sizeof(void*));
    if (!raw_ptr) return nullptr;
    
    void** aligned_ptr = reinterpret_cast<void**>(
        (reinterpret_cast<uintptr_t>(raw_ptr) + sizeof(void*) + alignment - 1) & ~(alignment - 1)
    );
    *(aligned_ptr - 1) = raw_ptr;
    ptr = aligned_ptr;
#endif
    
    if (ptr) {
        size_t current = heap_stats.total_allocated.fetch_add(size) + size;
        heap_stats.allocation_count.fetch_add(1);
        
        // Update peak allocation
        size_t peak = heap_stats.peak_allocated.load();
        while (current > peak) {
            if (heap_stats.peak_allocated.compare_exchange_weak(peak, current)) {
                break;
            }
        }
    }
    
    return ptr;
}

inline void HeapAlignedFree(void* ptr) {
    if (!ptr) return;
    
#if defined(_MSC_VER)
    _aligned_free(ptr);
#elif defined(__GNUC__) || defined(__clang__)
    std::free(ptr);
#else
    // Fallback implementation
    std::free(*(reinterpret_cast<void**>(ptr) - 1));
#endif
}

// Debug heap functions
#ifdef _DEBUG
inline void HeapCheck() {
    // In debug mode, could integrate with memory debugging tools
    // For now, just a placeholder
}

inline bool HeapIsValidPtr(const void* ptr) {
    // Validate pointer in debug mode
    // For now, just return true
    return ptr != nullptr;
}
#else
inline void HeapCheck() {}
inline bool HeapIsValidPtr(const void* ptr) { return ptr != nullptr; }
#endif

// Memory pool implementation
template<size_t BlockSize = 4096, size_t Alignment = 8>
class HeapPool {
private:
    struct Block {
        Block* next;
        char data[BlockSize - sizeof(Block*)];
    };
    
    struct FreeBlock {
        FreeBlock* next;
    };
    
    Block* blocks;
    FreeBlock* free_list;
    std::mutex pool_mutex;
    size_t object_size;
    size_t objects_per_block;
    
    void AddBlock() {
        Block* new_block = static_cast<Block*>(HeapMalloc(sizeof(Block)));
        if (!new_block) return;
        
        new_block->next = blocks;
        blocks = new_block;
        
        // Add all objects in the block to free list
        char* ptr = new_block->data;
        char* end = ptr + (objects_per_block * object_size);
        while (ptr < end) {
            FreeBlock* fb = reinterpret_cast<FreeBlock*>(ptr);
            fb->next = free_list;
            free_list = fb;
            ptr += object_size;
        }
    }
    
public:
    explicit HeapPool(size_t obj_size) 
        : blocks(nullptr), free_list(nullptr), object_size(obj_size) {
        // Align object size
        object_size = (object_size + Alignment - 1) & ~(Alignment - 1);
        objects_per_block = (BlockSize - sizeof(Block*)) / object_size;
    }
    
    ~HeapPool() {
        std::lock_guard<std::mutex> lock(pool_mutex);
        while (blocks) {
            Block* next = blocks->next;
            HeapFree(blocks, sizeof(Block));
            blocks = next;
        }
    }
    
    void* Allocate() {
        std::lock_guard<std::mutex> lock(pool_mutex);
        
        if (!free_list) {
            AddBlock();
            if (!free_list) return nullptr;
        }
        
        FreeBlock* fb = free_list;
        free_list = fb->next;
        return fb;
    }
    
    void Deallocate(void* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(pool_mutex);
        FreeBlock* fb = static_cast<FreeBlock*>(ptr);
        fb->next = free_list;
        free_list = fb;
    }
    
    size_t GetObjectSize() const { return object_size; }
    size_t GetBlockSize() const { return BlockSize; }
};

// Smart pointer with custom heap allocation
template<typename T>
class HeapPtr {
private:
    T* ptr;
    
public:
    explicit HeapPtr(T* p = nullptr) : ptr(p) {}
    
    ~HeapPtr() {
        if (ptr) {
            ptr->~T();
            HeapFree(ptr, sizeof(T));
        }
    }
    
    // Move semantics
    HeapPtr(HeapPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    
    HeapPtr& operator=(HeapPtr&& other) noexcept {
        if (this != &other) {
            if (ptr) {
                ptr->~T();
                HeapFree(ptr, sizeof(T));
            }
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    
    // Delete copy semantics
    HeapPtr(const HeapPtr&) = delete;
    HeapPtr& operator=(const HeapPtr&) = delete;
    
    T* operator->() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* get() const { return ptr; }
    
    void reset() {
        if (ptr) {
            ptr->~T();
            HeapFree(ptr, sizeof(T));
            ptr = nullptr;
        }
    }
    
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    
    explicit operator bool() const { return ptr != nullptr; }
};

// Helper function to create HeapPtr
template<typename T, typename... Args>
HeapPtr<T> MakeHeap(Args&&... args) {
    void* raw = HeapMalloc(sizeof(T));
    if (!raw) throw std::bad_alloc();
    
    try {
        T* ptr = new(raw) T(std::forward<Args>(args)...);
        return HeapPtr<T>(ptr);
    } catch (...) {
        HeapFree(raw, sizeof(T));
        throw;
    }
}

// Memory statistics functions
inline const HeapStatistics& GetHeapStatistics() {
    return heap_stats;
}

inline void ResetHeapStatistics() {
    heap_stats.Reset();
}

inline size_t GetHeapTotalAllocated() {
    return heap_stats.total_allocated.load();
}

inline size_t GetHeapPeakAllocated() {
    return heap_stats.peak_allocated.load();
}

inline size_t GetHeapTotalDeallocated() {
    return heap_stats.total_deallocated.load();
}

inline size_t GetHeapAllocationCount() {
    return heap_stats.allocation_count.load();
}

inline size_t GetHeapDeallocationCount() {
    return heap_stats.deallocation_count.load();
}

// Initialize global heap statistics
HeapStatistics heap_stats;

#endif