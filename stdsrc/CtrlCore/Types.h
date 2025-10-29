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

// Keyboard key code type
using dword = unsigned long long;

// Keyboard key combination constants
const dword K_SHIFT = 0x0100ULL;
const dword K_CTRL = 0x0200ULL;
const dword K_ALT = 0x0400ULL;
const dword K_KEYUP = 0x4000000ULL;

// Standard key codes
const dword K_A = 0x41ULL;
const dword K_B = 0x42ULL;
const dword K_C = 0x43ULL;
const dword K_D = 0x44ULL;
const dword K_E = 0x45ULL;
const dword K_F = 0x46ULL;
const dword K_G = 0x47ULL;
const dword K_H = 0x48ULL;
const dword K_I = 0x49ULL;
const dword K_J = 0x4AULL;
const dword K_K = 0x4BULL;
const dword K_L = 0x4CULL;
const dword K_M = 0x4DULL;
const dword K_N = 0x4EULL;
const dword K_O = 0x4FULL;
const dword K_P = 0x50ULL;
const dword K_Q = 0x51ULL;
const dword K_R = 0x52ULL;
const dword K_S = 0x53ULL;
const dword K_T = 0x54ULL;
const dword K_U = 0x55ULL;
const dword K_V = 0x56ULL;
const dword K_W = 0x57ULL;
const dword K_X = 0x58ULL;
const dword K_Y = 0x59ULL;
const dword K_Z = 0x5AULL;

const dword K_0 = 0x30ULL;
const dword K_1 = 0x31ULL;
const dword K_2 = 0x32ULL;
const dword K_3 = 0x33ULL;
const dword K_4 = 0x34ULL;
const dword K_5 = 0x35ULL;
const dword K_6 = 0x36ULL;
const dword K_7 = 0x37ULL;
const dword K_8 = 0x38ULL;
const dword K_9 = 0x39ULL;

const dword K_F1 = 0x7000ULL;
const dword K_F2 = 0x7001ULL;
const dword K_F3 = 0x7002ULL;
const dword K_F4 = 0x7003ULL;
const dword K_F5 = 0x7004ULL;
const dword K_F6 = 0x7005ULL;
const dword K_F7 = 0x7006ULL;
const dword K_F8 = 0x7007ULL;
const dword K_F9 = 0x7008ULL;
const dword K_F10 = 0x7009ULL;
const dword K_F11 = 0x700AULL;
const dword K_F12 = 0x700BULL;

const dword K_LEFT = 0x7020ULL;
const dword K_UP = 0x7021ULL;
const dword K_RIGHT = 0x7022ULL;
const dword K_DOWN = 0x7023ULL;
const dword K_HOME = 0x7024ULL;
const dword K_END = 0x7025ULL;
const dword K_PGUP = 0x7026ULL;
const dword K_PGDN = 0x7027ULL;

const dword K_INSERT = 0x7030ULL;
const dword K_DELETE = 0x7FULL;
const dword K_BACKSPACE = 0x08ULL;
const dword K_TAB = 0x09ULL;
const dword K_RETURN = 0x0DULL;
const dword K_ENTER = 0x0DULL;
const dword K_ESCAPE = 0x1BULL;
const dword K_SPACE = 0x20ULL;

const dword K_CTRL_A = K_CTRL | K_A;
const dword K_CTRL_C = K_CTRL | K_C;
const dword K_CTRL_V = K_CTRL | K_V;
const dword K_CTRL_X = K_CTRL | K_X;
const dword K_CTRL_Z = K_CTRL | K_Z;
const dword K_CTRL_Y = K_CTRL | K_Y;

// Common key combination utility functions
inline dword MakeCtrlKey(dword key) { return K_CTRL | key; }
inline dword MakeAltKey(dword key) { return K_ALT | key; }
inline dword MakeShiftKey(dword key) { return K_SHIFT | key; }
inline bool IsCtrlPressed(dword key) { return (key & K_CTRL) != 0; }
inline bool IsAltPressed(dword key) { return (key & K_ALT) != 0; }
inline bool IsShiftPressed(dword key) { return (key & K_SHIFT) != 0; }
inline bool IsKeyUp(dword key) { return (key & K_KEYUP) != 0; }
inline dword GetBaseKey(dword key) { return key & ~(K_CTRL | K_ALT | K_SHIFT | K_KEYUP); }

// Key description utility
std::string GetKeyDesc(dword key);

// Timer utility functions
void SetTimeCallback(int delay_ms, const Event<>& cb, void *id = nullptr);
void KillTimeCallback(void *id = nullptr);
bool ExistsTimeCallback(void *id);
dword GetTimeClick();
void Sleep(int milliseconds);

// Timer callback wrapper functions
template<typename Func>
void SetTimeCallback(int delay_ms, Func&& func, void *id = nullptr) {
    SetTimeCallback(delay_ms, Event<>() << func, id);
}

template<typename Func>
void KillSetTimeCallback(int delay_ms, Func&& func, void *id = nullptr) {
    KillTimeCallback(id);
    SetTimeCallback(delay_ms, Event<>() << func, id);
}

// Time utilities
inline dword GetTickCount() {
    return GetTimeClick();
}

inline dword GetTime() {
    return GetTimeClick();
}

inline void Delay(int milliseconds) {
    Sleep(milliseconds);
}

// Timer class for RAII-style timer management
class Timer {
private:
    void* id;
    bool active;
    
public:
    Timer() : id(nullptr), active(false) {}
    
    ~Timer() {
        if (active) {
            KillTimeCallback(id);
        }
    }
    
    void Start(int delay_ms, std::function<void()> callback) {
        if (active) {
            KillTimeCallback(id);
        }
        SetTimeCallback(delay_ms, Event<>() << callback, id);
        active = true;
    }
    
    void Stop() {
        if (active) {
            KillTimeCallback(id);
            active = false;
        }
    }
    
    bool IsActive() const {
        return active;
    }
    
    void Restart(int delay_ms, std::function<void()> callback) {
        Start(delay_ms, callback);
    }
    
    // One-shot timer
    static void Once(int delay_ms, std::function<void()> callback) {
        SetTimeCallback(delay_ms, Event<>() << callback);
    }
    
    // Repeating timer
    static void Repeat(int interval_ms, std::function<void()> callback) {
        SetTimeCallback(-interval_ms, Event<>() << callback); // Negative means repeat
    }
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