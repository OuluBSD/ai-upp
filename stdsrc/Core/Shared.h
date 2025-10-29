#pragma once
#ifndef _Core_Shared_h_
#define _Core_Shared_h_

#include <memory>
#include <mutex>
#include "Core.h"

// SharedPtr is a wrapper around std::shared_ptr
template <typename T>
using SharedPtr = std::shared_ptr<T>;

// SharedPtr utilities
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Thread-safe shared resource wrapper
template <typename T>
class SharedResource {
private:
    mutable std::mutex mtx;
    T resource;
    
public:
    SharedResource() = default;
    
    template <typename... Args>
    explicit SharedResource(Args&&... args) : resource(std::forward<Args>(args)...) {}
    
    // Access resource with lock
    template <typename Func>
    auto Access(Func&& func) -> decltype(func(resource)) {
        std::lock_guard<std::mutex> lock(mtx);
        return func(resource);
    }
    
    // Const access
    template <typename Func>
    auto Access(Func&& func) const -> decltype(func(resource)) {
        std::lock_guard<std::mutex> lock(mtx);
        return func(resource);
    }
    
    // Get a copy of the resource
    T GetCopy() const {
        std::lock_guard<std::mutex> lock(mtx);
        return resource;
    }
    
    // Set a new resource
    void Set(const T& new_resource) {
        std::lock_guard<std::mutex> lock(mtx);
        resource = new_resource;
    }
    
    void Set(T&& new_resource) {
        std::lock_guard<std::mutex> lock(mtx);
        resource = std::move(new_resource);
    }
};

// Shared variable with atomic access
template <typename T>
class SharedVar {
private:
    mutable std::mutex mtx;
    T value;
    
public:
    SharedVar() = default;
    explicit SharedVar(const T& initial_value) : value(initial_value) {}
    explicit SharedVar(T&& initial_value) : value(std::move(initial_value)) {}
    
    T Get() const {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
    
    void Set(const T& new_value) {
        std::lock_guard<std::mutex> lock(mtx);
        value = new_value;
    }
    
    void Set(T&& new_value) {
        std::lock_guard<std::mutex> lock(mtx);
        value = std::move(new_value);
    }
    
    template <typename Func>
    void Modify(Func&& modifier) {
        std::lock_guard<std::mutex> lock(mtx);
        modifier(value);
    }
    
    T operator++() {
        std::lock_guard<std::mutex> lock(mtx);
        return ++value;
    }
    
    T operator++(int) {
        std::lock_guard<std::mutex> lock(mtx);
        return value++;
    }
    
    T operator--() {
        std::lock_guard<std::mutex> lock(mtx);
        return --value;
    }
    
    T operator--(int) {
        std::lock_guard<std::mutex> lock(mtx);
        return value--;
    }
    
    operator T() const {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

#endif