#pragma once
#ifndef _CtrlCore_Types_h_
#define _CtrlCore_Types_h_

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <wchar.h>
#include <map>
#include <unordered_map>

namespace Upp {

// U++ Value type - can hold different types of values
// Simplified implementation to avoid variant conflicts
class Value {
private:
    enum Type { 
        TYPE_INT, TYPE_DOUBLE, TYPE_STRING, TYPE_BOOL, TYPE_PTR, TYPE_NULL 
    };
    
    Type type;
    union {
        int int_val;
        double double_val;
        bool bool_val;
        void* ptr_val;
    };
    std::string* string_val; // Use heap for string to fit in union

public:
    Value() : type(TYPE_NULL), ptr_val(nullptr), string_val(nullptr) {}
    Value(int i) : type(TYPE_INT), int_val(i), string_val(nullptr) {}
    Value(double d) : type(TYPE_DOUBLE), double_val(d), string_val(nullptr) {}
    Value(const std::string& s) : type(TYPE_STRING), string_val(new std::string(s)) {}
    Value(const char* s) : type(TYPE_STRING), string_val(new std::string(s ? s : "")) {}
    Value(bool b) : type(TYPE_BOOL), bool_val(b), string_val(nullptr) {}
    Value(void* p) : type(TYPE_PTR), ptr_val(p), string_val(nullptr) {}
    
    // Copy constructor
    Value(const Value& other) : string_val(nullptr) {
        type = other.type;
        switch(type) {
            case TYPE_STRING:
                string_val = other.string_val ? new std::string(*other.string_val) : nullptr;
                break;
            case TYPE_INT:
                int_val = other.int_val;
                break;
            case TYPE_DOUBLE:
                double_val = other.double_val;
                break;
            case TYPE_BOOL:
                bool_val = other.bool_val;
                break;
            case TYPE_PTR:
                ptr_val = other.ptr_val;
                break;
            case TYPE_NULL:
            default:
                ptr_val = nullptr;
                break;
        }
    }
    
    // Assignment operator
    Value& operator=(const Value& other) {
        if (this != &other) {
            // Clean up old string if needed
            if (type == TYPE_STRING && string_val) {
                delete string_val;
            }
            
            type = other.type;
            switch(type) {
                case TYPE_STRING:
                    string_val = other.string_val ? new std::string(*other.string_val) : nullptr;
                    break;
                case TYPE_INT:
                    int_val = other.int_val;
                    break;
                case TYPE_DOUBLE:
                    double_val = other.double_val;
                    break;
                case TYPE_BOOL:
                    bool_val = other.bool_val;
                    break;
                case TYPE_PTR:
                    ptr_val = other.ptr_val;
                    break;
                case TYPE_NULL:
                default:
                    ptr_val = nullptr;
                    break;
            }
        }
        return *this;
    }
    
    // Destructor
    ~Value() {
        if (type == TYPE_STRING && string_val) {
            delete string_val;
        }
    }
    
    bool IsNull() const {
        return type == TYPE_NULL;
    }
    
    template<typename T>
    T Get() const {
        // This is a simplified implementation - in a real scenario,
        // proper type checking and conversion would be needed
        return T{};
    }
    
    // Specific getters for each type
    int GetInt() const { return type == TYPE_INT ? int_val : 0; }
    double GetDouble() const { return type == TYPE_DOUBLE ? double_val : 0.0; }
    std::string GetString() const { return type == TYPE_STRING && string_val ? *string_val : std::string(); }
    bool GetBool() const { return type == TYPE_BOOL ? bool_val : false; }
    void* GetPtr() const { return type == TYPE_PTR ? ptr_val : nullptr; }
};

// U++ template types for containers
template<typename T>
class Array {
private:
    std::vector<T> items;

public:
    void Add(const T& item) { items.push_back(item); }
    T& operator[](int index) { return items[index]; }
    const T& operator[](int index) const { return items[index]; }
    int GetCount() const { return static_cast<int>(items.size()); }
    void Clear() { items.clear(); }
    bool IsEmpty() const { return items.empty(); }
    T* Begin() { return items.data(); }
    T* End() { return items.data() + items.size(); }
    const T* Begin() const { return items.data(); }
    const T* End() const { return items.data() + items.size(); }
};

template<typename T>
class Vector {
private:
    std::vector<T> items;

public:
    void Add(const T& item) { items.push_back(item); }
    T& operator[](int index) { return items[index]; }
    const T& operator[](int index) const { return items[index]; }
    int GetCount() const { return static_cast<int>(items.size()); }
    void Clear() { items.clear(); }
    bool IsEmpty() const { return items.empty(); }
    T* Begin() { return items.data(); }
    T* End() { return items.data() + items.size(); }
    const T* Begin() const { return items.data(); }
    const T* End() const { return items.data() + items.size(); }
};

template<typename T>
class Ptr {
private:
    std::shared_ptr<T> ptr;

public:
    Ptr() = default;
    explicit Ptr(T* p) : ptr(p) {}
    explicit Ptr(std::shared_ptr<T> p) : ptr(p) {}
    
    T* operator->() const { return ptr.get(); }
    T& operator*() const { return *ptr; }
    T* Get() const { return ptr.get(); }
    bool operator!() const { return !ptr; }
    operator bool() const { return ptr != nullptr; }
    
    void operator=(std::shared_ptr<T> p) { ptr = p; }
    void operator=(T* p) { ptr.reset(p); }
};

// U++ template for reference-counted objects
template<typename T>
class Pte {
public:
    virtual ~Pte() = default;
    // In real U++, this would have complex reference counting
};

// Style base class for U++ styling system
template<typename T>
class ChStyle {
public:
    // Base styling functionality would go here
    virtual ~ChStyle() = default;
};

// String-like types
using WString = std::wstring;  // Wide string for Unicode text

// Paint rectangle for backgrounds
class PaintRect {
private:
    std::string data;  // Simplified representation

public:
    PaintRect() = default;
    PaintRect(const std::string& d) : data(d) {}
    // More functionality would be implemented based on actual U++ requirements
};

// Event system for callbacks - forward declaration
template<typename... Args>
class Event;

// Specialization for event with no arguments
template<>
class Event<> {
private:
    std::vector<std::function<void()>> callbacks;

public:
    Event& operator<<(std::function<void()> cb) {
        callbacks.push_back(cb);
        return *this;
    }
    
    void operator()() {
        for (auto& cb : callbacks) {
            cb();
        }
    }
    
    template<typename T>
    Event& operator<<(void (T::*method)()) {
        // Handle member function callbacks
        return *this;
    }
};

// Callback types
template<typename... Args>
using Callback = std::function<void(Args...)>;

template<typename T>
using Callback1 = std::function<void(T)>;

// Function wrapper
template<typename R, typename... Args>
class Function {
private:
    std::function<R(Args...)> func;

public:
    Function() = default;
    Function(const std::function<R(Args...)>& f) : func(f) {}
    Function(R(*f)(Args...)) : func(f) {}
    
    R operator()(Args... args) const {
        return func(args...);
    }
    
    operator bool() const {
        return bool(func);
    }
};

// ValueArray - array of values
class ValueArray {
private:
    std::vector<Value> values;

public:
    void Add(const Value& v) { values.push_back(v); }
    Value& operator[](int i) { return values[i]; }
    const Value& operator[](int i) const { return values[i]; }
    int GetCount() const { return static_cast<int>(values.size()); }
    void Clear() { values.clear(); }
};

// Uuid type
class Uuid {
private:
    std::string uuid_str;

public:
    Uuid() = default;
    Uuid(const std::string& str) : uuid_str(str) {}

    static Uuid Create() {
        // Simplified UUID generation (in real implementation would be more complex)
        return Uuid("00000000-0000-0000-0000-000000000000");
    }
};

// Moveable base class for U++ move semantics
template<typename T>
class Moveable {
public:
    Moveable() = default;
    Moveable(T&& other) = default;
    Moveable& operator=(T&& other) = default;
};

// Serialization Stream class
class Stream {
public:
    virtual ~Stream() = default;
    virtual bool IsEof() = 0;
    virtual dword GetCount() = 0;
    virtual void  Put(int c) = 0;
    virtual int   Get() = 0;
    virtual void  Put(const void *data, dword size) = 0;
    virtual void  Get(void *data, dword size) = 0;
    
    bool IsStoring() const { return is_storing; }
    bool IsLoading() const { return !is_storing; }
    
protected:
    bool is_storing = false;
};

// Json serialization IO class  
class JsonIO {
public:
    virtual ~JsonIO() = default;
};

// XML serialization IO class
class XmlIO {
public:
    virtual ~XmlIO() = default;
};

}

#endif