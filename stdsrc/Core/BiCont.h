#pragma once
#ifndef _Core_BiCont_h_
#define _Core_BiCont_h_

#include <vector>
#include <memory>
#include "Core.h"

template <class T>
class BiVector : MoveableAndDeepCopyOption< BiVector<T> > {
protected:
    std::vector<T> vector;
    int start = 0;
    int items = 0;

public:
    int      GetCount() const        { return items; }
    bool     IsEmpty() const         { return items == 0; }
    void     Clear()                 { vector.clear(); start = 0; items = 0; }

    T&       AddHead()               { 
        if (items == 0) {
            vector.emplace_back();
            start = 0;
            items = 1;
            return vector[0];
        } else {
            // Need to insert at the beginning for proper indexing
            vector.insert(vector.begin(), T{});
            start = 0;
            items++;
            return vector[0];
        }
    }
    
    T&       AddTail()               { 
        if (items == 0) {
            vector.emplace_back();
            start = 0;
            items = 1;
            return vector[0];
        } else {
            vector.emplace_back();
            items++;
            return vector[items - 1];
        }
    }
    
    template <class... Args>
    T&       CreateHead(Args&&... args) { 
        if (items == 0) {
            vector.emplace_back(std::forward<Args>(args)...);
            start = 0;
            items = 1;
            return vector[0];
        } else {
            vector.insert(vector.begin(), T(std::forward<Args>(args)...));
            start = 0;
            items++;
            return vector[0];
        }
    }
    
    template <class... Args>
    T&       CreateTail(Args&&... args) { 
        vector.emplace_back(std::forward<Args>(args)...);
        items++;
        return vector[items - 1];
    }
    
    void     AddHead(const T& x)     { 
        if (items == 0) {
            vector.push_back(x);
            start = 0;
            items = 1;
        } else {
            vector.insert(vector.begin(), x);
            start = 0;
            items++;
        }
    }
    
    void     AddTail(const T& x)     { 
        vector.push_back(x);
        items++;
    }
    
    void     AddHead(T&& x)          { 
        if (items == 0) {
            vector.emplace_back(std::move(x));
            start = 0;
            items = 1;
        } else {
            vector.insert(vector.begin(), std::move(x));
            start = 0;
            items++;
        }
    }
    
    void     AddTail(T&& x)          { 
        vector.emplace_back(std::move(x));
        items++;
    }
    
    T&       Head()                  { 
        ASSERT(items > 0); 
        return vector[start]; 
    }
    
    T&       Tail()                  { 
        ASSERT(items > 0); 
        return vector[start + items - 1]; 
    }
    
    const T& Head() const            { 
        ASSERT(items > 0); 
        return vector[start]; 
    }
    
    const T& Tail() const            { 
        ASSERT(items > 0); 
        return vector[start + items - 1]; 
    }
    
    void     DropHead()              { 
        if (items > 0) {
            vector.erase(vector.begin() + start);
            items--;
            if (start >= vector.size() && items > 0) start = 0;  // Adjust if needed
        }
    }
    
    void     DropTail()              { 
        if (items > 0) {
            vector.pop_back();
            items--;
        }
    }
    
    T        PopHead()               { 
        T x = Head(); 
        DropHead(); 
        return x; 
    }
    
    T        PopTail()               { 
        T x = Tail(); 
        DropTail(); 
        return x; 
    }
    
    void     DropHead(int n)         { 
        for (int i = 0; i < n && items > 0; i++) {
            DropHead();
        }
    }
    
    void     DropTail(int n)         { 
        for (int i = 0; i < n && items > 0; i++) {
            DropTail();
        }
    }
    
    const T& operator[](int i) const { 
        ASSERT(i >= 0 && i < items); 
        return vector[start + i]; 
    }
    
    T&       operator[](int i)       { 
        ASSERT(i >= 0 && i < items); 
        return vector[start + i]; 
    }
    
    void     Shrink()                { 
        if (items == 0) {
            vector.clear();
        } else {
            // Remove unused elements at the beginning
            if (start > 0) {
                vector.erase(vector.begin(), vector.begin() + start);
                start = 0;
            }
            // Remove unused elements at the end
            if (items < vector.size()) {
                vector.resize(items);
            }
        }
    }
    
    void     Reserve(int n)          { 
        vector.reserve(n); 
    }
    
    int      GetAlloc() const        { return vector.capacity(); }

    template <class B> bool operator==(const B& b) const { 
        return IsEqualRange(*this, b); 
    }
    template <class B> bool operator!=(const B& b) const { 
        return !operator==(b); 
    }
    template <class B> int  Compare(const B& b) const    { 
        return CompareRanges(*this, b); 
    }
    template <class B> bool operator<=(const B& x) const { 
        return Compare(x) <= 0; 
    }
    template <class B> bool operator>=(const B& x) const { 
        return Compare(x) >= 0; 
    }
    template <class B> bool operator<(const B& x) const  { 
        return Compare(x) < 0; 
    }
    template <class B> bool operator>(const B& x) const  { 
        return Compare(x) > 0; 
    }

    BiVector(const BiVector& src, int)          { 
        vector = src.vector;
        start = src.start;
        items = src.items;
    }
    
    BiVector(BiVector&& src)                    { 
        vector = std::move(src.vector);
        start = src.start;
        items = src.items;
        src.start = 0;
        src.items = 0;
    }
    
    void operator=(BiVector&& src)              { 
        if(this != &src) { 
            vector = std::move(src.vector);
            start = src.start;
            items = src.items;
            src.start = 0;
            src.items = 0;
        } 
    }
    
    BiVector()                                  { }
    ~BiVector()                                 { } 

    BiVector(std::initializer_list<T> init) {
        for (const auto& item : init) {
            AddTail(item);
        }
    }

    typedef ConstIIterator<BiVector> ConstIterator;
    typedef IIterator<BiVector>      Iterator;

    ConstIterator    begin() const              { return ConstIterator(*this, 0); }
    ConstIterator    end() const                { return ConstIterator(*this, GetCount()); }
    Iterator         begin()                    { return Iterator(*this, 0); }
    Iterator         end()                      { return Iterator(*this, GetCount()); }

    friend void Swap(BiVector& a, BiVector& b)  { 
        UPP::Swap(a.vector, b.vector);
        UPP::Swap(a.start, b.start);
        UPP::Swap(a.items, b.items);
    }

#ifdef DEPRECATED
    ConstIterator    GetIter(int pos) const     { return ConstIterator(*this, pos); }
    Iterator         GetIter(int pos)           { return Iterator(*this, pos); }
    typedef T        ValueType;
#endif
};

template <class T>
class BiArray : MoveableAndDeepCopyOption< BiArray<T> > {
protected:
    BiVector<std::unique_ptr<T>> bv;

public:
    int      GetCount() const              { return bv.GetCount(); }
    bool     IsEmpty() const               { return GetCount() == 0; }
    void     Clear()                       { bv.Clear(); }

    T&       AddHead()                     { 
        auto ptr = std::make_unique<T>(); 
        T* raw_ptr = ptr.get();
        bv.AddHead(std::move(ptr));
        return *raw_ptr;
    }
    
    T&       AddTail()                     { 
        auto ptr = std::make_unique<T>(); 
        T* raw_ptr = ptr.get();
        bv.AddTail(std::move(ptr));
        return *raw_ptr;
    }
    
    void     AddHead(const T& x)           { 
        auto ptr = std::make_unique<T>(x); 
        bv.AddHead(std::move(ptr));
    }
    
    void     AddTail(const T& x)           { 
        auto ptr = std::make_unique<T>(x); 
        bv.AddTail(std::move(ptr));
    }
    
    T&       AddHead(T *newt)              { 
        bv.AddHead(std::unique_ptr<T>(newt)); 
        return *newt; 
    }
    
    T&       AddTail(T *newt)              { 
        bv.AddTail(std::unique_ptr<T>(newt)); 
        return *newt; 
    }
    
    template <class... Args>
    T& CreateHead(Args&&... args)          { 
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...); 
        T* raw_ptr = ptr.get();
        bv.AddHead(std::move(ptr));
        return *raw_ptr;
    }
    
    template <class... Args>
    T& CreateTail(Args&&... args)          { 
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...); 
        T* raw_ptr = ptr.get();
        bv.AddTail(std::move(ptr));
        return *raw_ptr;
    }
    
    template <class TT, class... Args>
    TT& CreateHead(Args&&... args)         { 
        auto ptr = std::make_unique<TT>(std::forward<Args>(args)...); 
        TT* raw_ptr = ptr.get();
        bv.AddHead(std::move(ptr));
        return *raw_ptr;
    }
    
    template <class TT, class... Args>
    TT& CreateTail(Args&&... args)         { 
        auto ptr = std::make_unique<TT>(std::forward<Args>(args)...); 
        TT* raw_ptr = ptr.get();
        bv.AddTail(std::move(ptr));
        return *raw_ptr;
    }
    
    T&       AddHead(One<T>&& one)         { 
        ASSERT(one); 
        auto ptr = std::make_unique<T>(one.Detach());
        T* raw_ptr = ptr.get();
        bv.AddHead(std::move(ptr));
        return *raw_ptr;
    }
    
    T&       AddTail(One<T>&& one)         { 
        ASSERT(one); 
        auto ptr = std::make_unique<T>(one.Detach());
        T* raw_ptr = ptr.get();
        bv.AddTail(std::move(ptr));
        return *raw_ptr;
    }
    
    T&       Head()                        { 
        return *bv.Head().get(); 
    }
    
    T&       Tail()                        { 
        return *bv.Tail().get(); 
    }
    
    const T& Head() const                  { 
        return *bv.Head().get(); 
    }
    
    const T& Tail() const                  { 
        return *bv.Tail().get(); 
    }
    
    void     DropHead()                    { 
        bv.Head().reset(); // Delete the object
        bv.DropHead();
    }
    
    void     DropTail()                    { 
        bv.Tail().reset(); // Delete the object
        bv.DropTail();
    }
    
    T       *DetachHead()                  { 
        std::unique_ptr<T> ptr = std::move(bv.Head());
        T* raw_ptr = ptr.get();
        bv.DropHead();
        return raw_ptr;
    }
    
    T       *DetachTail()                  { 
        std::unique_ptr<T> ptr = std::move(bv.Tail());
        T* raw_ptr = ptr.get();
        bv.DropTail();
        return raw_ptr;
    }

    T&       operator[](int i)             { 
        return *bv[i].get(); 
    }
    
    const T& operator[](int i) const       { 
        return *bv[i].get(); 
    }

    void     Shrink()                      { bv.Shrink(); }
    void     Reserve(int n)                { bv.Reserve(n); }
    int      GetAlloc() const              { return bv.GetAlloc(); }

    template <class B> bool operator==(const B& b) const { 
        return IsEqualRange(*this, b); 
    }
    template <class B> bool operator!=(const B& b) const { 
        return !operator==(b); 
    }
    template <class B> int  Compare(const B& b) const    { 
        return CompareRanges(*this, b); 
    }
    template <class B> bool operator<=(const B& x) const { 
        return Compare(x) <= 0; 
    }
    template <class B> bool operator>=(const B& x) const { 
        return Compare(x) >= 0; 
    }
    template <class B> bool operator<(const B& x) const  { 
        return Compare(x) < 0; 
    }
    template <class B> bool operator>(const B& x) const  { 
        return Compare(x) > 0; 
    }

    BiArray(const BiArray& v, int)           { 
        for (int i = 0; i < v.GetCount(); i++) {
            AddTail(new T(v[i]));
        }
    }

    BiArray(BiArray&& src) : bv(pick(src.bv)){}
    
    void operator=(BiArray&& src)            { 
        if(this != &src) { 
            bv = pick(src.bv); 
        } 
    }
    
    BiArray()                                {}
    ~BiArray()                               { }

    BiArray(std::initializer_list<T> init) {
        for (const auto& item : init) {
            AddTail(new T(item));
        }
    }

    typedef ConstIIterator<BiArray> ConstIterator;
    typedef IIterator<BiArray>      Iterator;

    ConstIterator    begin() const              { return ConstIterator(*this, 0); }
    ConstIterator    end() const                { return ConstIterator(*this, GetCount()); }
    Iterator         begin()                    { return Iterator(*this, 0); }
    Iterator         end()                      { return Iterator(*this, GetCount()); }

    friend void Swap(BiArray& a, BiArray& b)    { UPP::Swap(a.bv, b.bv); }

#ifdef DEPRECATED
    ConstIterator    GetIter(int pos) const     { return ConstIterator(*this, pos); }
    Iterator         GetIter(int pos)           { return Iterator(*this, pos); }
    typedef T        ValueType;
#endif
};

#endif