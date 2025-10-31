#pragma once
#ifndef _Core_Recycler_h_
#define _Core_Recycler_h_

#include <vector>
#include <mutex>
#include <memory>
#include <new>
#include "Core.h"

template <class T, bool keep_as_constructed=false>
class RecyclerPool {
    std::vector<T*> pool;
    std::mutex lock;
    bool destructed = false;
    
public:
    typedef RecyclerPool CLASSNAME;
    RecyclerPool() {}
    ~RecyclerPool() {Clear(); destructed = true;}
    
    void Clear() {
        std::lock_guard<std::mutex> guard(lock);
        for (T* o : pool) {
            if (keep_as_constructed) {
                o->~T();
            }
            std::free(o);
        }
        pool.clear();
    }
    void Reserve(int i) {
        std::lock_guard<std::mutex> guard(lock);
        pool.reserve(i);
    }
    
    template <typename... Args>
    T* New(const Args&... args) {
        T* o = nullptr;
        {
            std::lock_guard<std::mutex> guard(lock);
            if (!pool.empty()) {
                o = pool.back();
                pool.pop_back();
            }
        }
        
        if (!o) {
            o = static_cast<T*>(std::malloc(sizeof(T)));
            if (!o) {
                throw std::bad_alloc();
            }
            new (o) T(args...);
        } else {
            if (!keep_as_constructed) {
                new (o) T(args...);
            }
        }
        return o;
    }
    
    void Return(T* o) {
        if (destructed) {
            o->~T();
            std::free(o);
            return;
        }
        if (!keep_as_constructed) {
            o->~T();
        }
        
        std::lock_guard<std::mutex> guard(lock);
        pool.push_back(o);
    }
    
    static RecyclerPool& StaticPool() {
        static RecyclerPool pool;
        return pool;
    }
};

template <class T, bool keep_as_constructed=false>
class Recycler {
    
public:
    using Pool = RecyclerPool<T, keep_as_constructed>;
    
private:
    Pool* pool = nullptr;
    std::unique_ptr<T> o;
    
public:
    Recycler() {}
    Recycler(Pool& pool) : pool(&pool) {
        o = std::unique_ptr<T>(pool.New());
    }
    ~Recycler() {Clear();}
    
    void Clear() {
        if (o && pool) {
            pool->Return(o.release());
        }
    }
    void Create(Pool& pool) {
        Clear();
        this->pool = &pool;
        o = std::unique_ptr<T>(pool.New());
    }
    T& Get() {
        ASSERT(o.get() != nullptr);
        return *o;
    }
    
    T& operator*() {return Get();}
    T* operator->() {return &Get();}
};

// Forward declaration
template <class T> struct RecyclerRefBase;

// Base class for reference counting
struct RefBase {
    mutable std::atomic<int> refcount{1};
    void* obj = nullptr;
    
    virtual ~RefBase() = default;
    virtual void Clear() = 0;
    virtual void Delete() = 0;
};

template <class T>
struct RecyclerRefBase : RefBase {
    using Pool = RecyclerPool<RecyclerRefBase<T>>;
    
    ~RecyclerRefBase() {Clear();}
    void Clear() override {
        if (!RefBase::obj) return;
        T::Pool::StaticPool().Return(static_cast<T*>(RefBase::obj));
        RefBase::obj = nullptr;
    }
    void Delete() override {
        Pool::StaticPool().Return(this);
    }
    void SetObj(T* o) {
        RefBase::obj = o;
    }
};

template <class T, class Ref = RecyclerRefBase<T>>
class Shared {
    Ref* ref = nullptr;
    
    void Retain() {
        if (ref) {
            ref->refcount.fetch_add(1);
        }
    }
    
    void Release() {
        if (ref && ref->refcount.fetch_sub(1) == 1) {
            ref->Delete();
        }
    }
    
public:
    Shared() = default;
    
    Shared(T* ptr) {
        if (ptr) {
            ref = new Ref();
            ref->SetObj(ptr);
        }
    }
    
    ~Shared() {
        Release();
    }
    
    Shared(const Shared& s) : ref(s.ref) {
        Retain();
    }
    
    Shared& operator=(const Shared& s) {
        if (this != &s) {
            Release();
            ref = s.ref;
            Retain();
        }
        return *this;
    }
    
    Shared(Shared&& s) noexcept : ref(s.ref) {
        s.ref = nullptr;
    }
    
    Shared& operator=(Shared&& s) noexcept {
        if (this != &s) {
            Release();
            ref = s.ref;
            s.ref = nullptr;
        }
        return *this;
    }
    
    T* operator->() const {
        return ref ? static_cast<T*>(ref->obj) : nullptr;
    }
    
    T& operator*() const {
        return *static_cast<T*>(ref->obj);
    }
    
    T* Get() const {
        return ref ? static_cast<T*>(ref->obj) : nullptr;
    }
    
    bool IsNull() const {
        return !ref || !ref->obj;
    }
    
    operator bool() const {
        return !IsNull();
    }
};

template <class T> using SharedRecycler = Shared<T, RecyclerRefBase<T>>;

#endif