# U++ to STL Mapping: Core Package Smart Pointers

This document provides comprehensive mapping between U++ Core package smart pointer types and their STL equivalents.

## 1. One ↔ std::unique_ptr

### U++ Declaration
```cpp
template <class T>
class One : MoveableAndDeepCopyOption< One<T> > {
    mutable T  *ptr;  // Pointer to managed object

public:
    void        Attach(T *data);                    // Attach raw pointer
    T          *Detach();                           // Detach pointer (relinquish ownership)
    void        Clear();                            // Clear and delete object
    void        operator=(T *data);                 // Assignment from raw pointer
    template <class TT> void operator=(One<TT>&& d); // Move assignment
    const T    *operator->() const;                 // Pointer access (const)
    T          *operator->();                       // Pointer access
    const T    *operator~() const;                  // Get raw pointer (const)
    T          *operator~();                        // Get raw pointer
    const T    *Get() const;                        // Get raw pointer (const)
    T          *Get();                              // Get raw pointer
    const T&    operator*() const;                  // Dereference (const)
    T&          operator*();                        // Dereference
    template <class TT, class... Args> TT& Create(Args&&... args); // Create object in-place
    template <class TT> TT& Create();               // Create object default
    template <class TT> bool Is() const;            // Check dynamic type
    bool        IsEmpty() const;                    // Check if empty
    operator bool() const;                          // Boolean conversion
    String ToString() const;                        // String representation
    One();                                         // Default constructor
    One(T *newt);                                  // Constructor from raw pointer
    template <class TT> One(One<TT>&& p);          // Move constructor
    One(const One<T>& p, int);                     // Deep copy constructor
    One(const One<T>& p) = delete;                 // No copy constructor
    void operator=(const One<T>& p) = delete;       // No copy assignment
};
```

### STL Equivalent
```cpp
template <class T, class Deleter = std::default_delete<T>>
class std::unique_ptr {
    T* ptr;  // Pointer to managed object
    Deleter deleter;  // Deleter function object

public:
    T& operator*() const;                           // Dereference
    T* operator->() const;                          // Pointer access
    T* get() const;                                 // Get raw pointer
    T* release();                                   // Release ownership
    void reset(T* p = nullptr);                     // Reset with new pointer
    void swap(std::unique_ptr<T, Deleter>& other);  // Swap with another
    explicit operator bool() const;                 // Boolean conversion
    template< class U, class... Args > std::unique_ptr<U> make_unique(Args&&... args); // Create (helper function)
    std::unique_ptr();                              // Default constructor
    explicit std::unique_ptr(T* p);                 // Constructor from raw pointer
    std::unique_ptr(std::unique_ptr&& r);           // Move constructor
    template< class U > std::unique_ptr(std::unique_ptr<U, D>&& r); // Move from compatible ptr
    std::unique_ptr& operator=(std::unique_ptr&& r); // Move assignment
    template< class U > std::unique_ptr& operator=(std::unique_ptr<U, D>&& r); // Move assignment
    ~std::unique_ptr();                             // Destructor (deletes object)
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| One | std::unique_ptr | ✓ Complete | |
| One::Attach(T *data) | std::unique_ptr::reset(T *p) | ✓ Complete | |
| One::Detach() | std::unique_ptr::release() | ✓ Complete | |
| One::Clear() | std::unique_ptr::reset() | ✓ Complete | |
| One::operator=(T *data) | std::unique_ptr::reset(T *p) | ✓ Complete | |
| One::operator->() | std::unique_ptr::operator->() | ✓ Complete | |
| One::operator~() | std::unique_ptr::get() | ✓ Complete | |
| One::Get() | std::unique_ptr::get() | ✓ Complete | |
| One::operator*() | std::unique_ptr::operator*() | ✓ Complete | |
| One::IsEmpty() | !std::unique_ptr or std::unique_ptr::get() == nullptr | ✓ Complete | |
| One::operator bool() | std::unique_ptr::operator bool() | ✓ Complete | |
| One(T *newt) | std::unique_ptr::unique_ptr(T *p) | ✓ Complete | |
| One(One<T>&& p) | std::unique_ptr::unique_ptr(std::unique_ptr&& r) | ✓ Complete | |
| One::operator=(One<TT>&& d) | std::unique_ptr::operator=(std::unique_ptr&& r) | ✓ Complete | |
| One::Create(args...) | std::unique_ptr with std::make_unique | ✓ Complete | Requires std::make_unique |

### Conversion Notes
- U++ One<T> maps directly to std::unique_ptr<T>
- One provides automatic deep copy semantics via its template parameter, which std::unique_ptr doesn't have built-in
- One::Attach() corresponds to std::unique_ptr::reset()
- One::Detach() corresponds to std::unique_ptr::release()
- One::Clear() is equivalent to calling reset() with nullptr
- One doesn't allow copy construction/assignment (deleted), just like std::unique_ptr
- One::Create() is similar to using std::make_unique()

## 2. Ptr ↔ std::shared_ptr or raw pointer

### U++ Declaration
```cpp
template <class T>
class Ptr : public PtrBase, Moveable< Ptr<T> > {
    PteBase::Prec *prec;  // Reference counting structure

public:
    using Type = T;
    T       *operator->() const;                    // Pointer access
    T       *operator~() const;                     // Get raw pointer
    operator T*() const;                            // Implicit conversion to raw pointer
    Ptr& operator=(T *ptr);                         // Assignment from raw pointer
    Ptr& operator=(const Ptr& ptr);                 // Assignment from another Ptr
    Ptr();                                         // Default constructor
    Ptr(T *ptr);                                   // Constructor from raw pointer
    Ptr(const Ptr& ptr);                           // Copy constructor
    String ToString() const;                        // String representation
    friend bool operator==(const Ptr& a, const T *b); // Equality with raw pointer
    friend bool operator==(const T *a, const Ptr& b); // Equality with raw pointer
    friend bool operator==(const Ptr& a, const Ptr& b); // Equality with another Ptr
    friend bool operator!=(const Ptr& a, const T *b); // Inequality with raw pointer
    friend bool operator!=(const T *a, const Ptr& b); // Inequality with raw pointer
    friend bool operator!=(const Ptr& a, const Ptr& b); // Inequality with another Ptr
};

class PteBase {
protected:
    struct Prec {
        PteBase *ptr;  // Pointer to actual object
        Atomic   n;    // Reference counter
        bool     panic = false;
    };
    volatile Prec  *prec;
    Prec           *PtrAdd();  // Add reference
    static void     PtrRelease(Prec *prec);  // Release reference
    // ... other methods
};

class PtrBase {
protected:
    PteBase::Prec *prec;
    void Set(PteBase *p);
    void Release();
    void Assign(PteBase *p);
public:
    ~PtrBase();
    void PanicRelease(bool b=true);  // Set panic mode
};
```

### STL Equivalent
```cpp
template< class T, class Deleter = std::default_delete<T>, class Alloc = std::allocator<T> >
class std::shared_ptr {
    T* ptr;  // Pointer to managed object
    std::shared_ptr_control_block* control_block;  // Shared control block with reference counter

public:
    T& operator*() const;                           // Dereference
    T* operator->() const;                          // Pointer access
    T* get() const;                                 // Get raw pointer
    explicit operator bool() const;                 // Boolean conversion
    bool unique() const;                            // Check if unique owner
    long use_count() const;                         // Get reference count
    void reset();                                   // Release ownership
    void reset(T* ptr);                             // Release and take ownership
    template< class U > void reset(U* ptr);        // Release and take ownership
    void swap(std::shared_ptr<T>& r);               // Swap with another
    std::shared_ptr();                              // Default constructor (null)
    explicit std::shared_ptr(T* p);                 // Constructor from raw pointer
    template< class U > std::shared_ptr(const std::shared_ptr<U>& r, T* ptr); // Aliasing constructor
    std::shared_ptr(const std::shared_ptr& r);      // Copy constructor
    template< class U > std::shared_ptr(const std::shared_ptr<U>& r); // Copy from compatible ptr
    std::shared_ptr(std::shared_ptr&& r);           // Move constructor
    template< class U > std::shared_ptr(std::shared_ptr<U>&& r); // Move from compatible ptr
    template< class U > std::shared_ptr& operator=(const std::shared_ptr<U>& r); // Copy assignment
    template< class U > std::shared_ptr& operator=(std::shared_ptr<U>&& r); // Move assignment
    ~std::shared_ptr();                             // Destructor (decrements counter)
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Ptr | std::shared_ptr | ✓ Complete | Closest equivalent, though implementation differs |
| Ptr::operator->() | std::shared_ptr::operator->() | ✓ Complete | |
| Ptr::operator~() | std::shared_ptr::get() | ✓ Complete | |
| Ptr::operator T*() | std::shared_ptr::get() or implicit conversion | ✓ Complete | |
| Ptr(T *ptr) | std::shared_ptr::shared_ptr(T *p) | ✓ Complete | |
| Ptr(const Ptr& ptr) | std::shared_ptr::shared_ptr(const shared_ptr& r) | ✓ Complete | |
| Ptr::operator=(T *ptr) | std::shared_ptr::reset(T *p) or assignment | ⚠️ Complex | Needs special handling |
| Ptr::operator=(const Ptr& ptr) | std::shared_ptr::operator=(const shared_ptr& r) | ✓ Complete | |
| operator==(const Ptr&, const T*) | operator==(const shared_ptr&, const T*) | ✓ Complete | |
| operator==(const Ptr&, const Ptr&) | operator==(const shared_ptr&, const shared_ptr&) | ✓ Complete | |

### Conversion Notes
- U++ Ptr<T> provides shared ownership with reference counting, similar to std::shared_ptr<T>
- However, the internal implementation differs: U++ Ptr uses a separate reference counting structure (PteBase::Prec) while std::shared_ptr uses a control block
- Both support copy construction and assignment, transferring shared ownership
- U++ Ptr has a "panic" mode mechanism that std::shared_ptr doesn't have
- U++ Ptr allows assignment from raw pointer which resets the reference, while std::shared_ptr assignment doesn't change ownership of the original pointer
- For direct replacement, use std::shared_ptr with std::make_shared when possible

## 3. Pick with Move Semantics ↔ std::move

### U++ Declaration
```cpp
// In context of containers and values, 'pick' enables move semantics
template <class T>
T&& pick(T& t) { return (T&&) t; }  // Cast to rvalue reference for move

// Many U++ types implement move semantics using pick:
class String {
    String(String&& s) { String0::Pick0(pick(s)); }  // Move constructor using pick
    void operator=(String&& s) { if(this != &s) { Free(); Pick0(pick(s)); } }  // Move assignment
    // ... other methods
};
```

### STL Equivalent
```cpp
#include <utility>
template< class T >
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept;  // Standard move utility

// Standard types implement move semantics naturally:
class std::string {
    std::string(std::string&& other) noexcept;  // Move constructor
    std::string& operator=(std::string&& other) noexcept;  // Move assignment
    // ... other methods
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| pick(x) | std::move(x) | ✓ Complete | |
| T&& (in function parameters) | T&& (in function parameters) | ✓ Complete | Same syntax |
| Move constructor | Move constructor | ✓ Complete | Same concept, similar implementation |
| Move assignment | Move assignment | ✓ Complete | Same concept, similar implementation |

### Conversion Notes
- U++'s `pick()` function directly corresponds to std::move() in STL
- Both serve the purpose of enabling move semantics by casting to rvalue reference
- Move semantics behave similarly in both systems for transferring ownership efficiently
- The syntax for move constructors and move assignment operators is identical between U++ and STL

## Summary of Smart Pointer Mappings

| U++ Smart Pointer | STL Equivalent | Notes |
|-------------------|----------------|-------|
| One<T> | std::unique_ptr<T> | Exclusive ownership, move-only semantics |
| Ptr<T> | std::shared_ptr<T> | Shared ownership with reference counting |
| pick() | std::move() | Utility for enabling move semantics |