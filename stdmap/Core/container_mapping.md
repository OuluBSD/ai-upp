# U++ to STL Mapping: Core Package Containers

This document provides comprehensive mapping between U++ Core package containers and their STL equivalents.

## 1. Vector ↔ std::vector

### U++ Declaration
```cpp
template <class T>
class Vector : public MoveableAndDeepCopyOption< Vector<T> > {
    T       *vector;
    int      items;
    int      alloc;

public:
    T&       Add();                                    // Add new default item
    T&       Add(const T& x);                         // Add copy of item
    T&       Add(T&& x);                              // Add with move semantics
    template <class... Args> T& Create(Args&&... args); // Create new item in-place
    void     AddN(int n);                             // Add n default items
    const T& operator[](int i) const;                 // Access element (const)
    T&       operator[](int i);                       // Access element
    const T& Get(int i, const T& def) const;          // Access with default
    T&       Get(int i, T& def);                      // Access with default
    int      GetCount() const;                        // Get number of items
    bool     IsEmpty() const;                         // Check if empty
    void     Trim(int n);                             // Trim to n items
    void     SetCount(int n);                         // Set count (default init)
    void     SetCount(int n, const T& init);          // Set count with init value
    void     SetCountR(int n);                        // Set count (resize)
    void     SetCountR(int n, const T& init);         // Set count with init value
    void     Clear();                                 // Clear container
    T&       At(int i);                               // Access with auto-resize
    T&       At(int i, const T& x);                   // Access with auto-resize
    void     Shrink();                                // Minimize memory usage
    void     Reserve(int n);                          // Reserve capacity
    int      GetAlloc() const;                        // Get allocated capacity
    void     Set(int i, const T& x, int count);       // Set range of items
    T&       Set(int i, const T& x);                  // Set single item
    T&       Set(int i, T&& x);                       // Set with move
    void     Remove(int i, int count = 1);            // Remove elements
    void     Remove(const int *sorted_list, int n);   // Remove by index list
    void     Remove(const Vector<int>& sorted_list);  // Remove by index vector
    template <class Condition> void RemoveIf(Condition c); // Remove conditionally
    void     InsertN(int i, int count = 1);           // Insert multiple empty
    T&       Insert(int i);                          // Insert empty at position
    void     Insert(int i, const T& x, int count);    // Insert multiple copies
    T&       Insert(int i, const T& x);               // Insert single item
    T&       Insert(int i, T&& x);                    // Insert with move
    void     Insert(int i, const Vector& x);          // Insert vector
    void     Insert(int i, const Vector& x, int offset, int count); // Insert range
    void     Insert(int i, Vector&& x);               // Insert with move
    template <class Range> void InsertRange(int i, const Range& r); // Insert range
    void     Append(const Vector& x);                 // Append vector
    void     Append(const Vector& x, int o, int c);   // Append range
    void     Append(Vector&& x);                      // Append with move
    template <class Range> void AppendRange(const Range& r); // Append range
    void     InsertSplit(int i, Vector<T>& v, int from); // Split and insert
    void     Swap(int i1, int i2);                   // Swap elements
    void     Drop(int n = 1);                        // Drop last n items
    T&       Top();                                   // Access last element
    const T& Top() const;                             // Access last element (const)
    T        Pop();                                   // Remove and return last element
    operator T*();                                   // Implicit conversion to pointer
    operator const T*() const;                       // Implicit conversion to const pointer
    Vector&  operator<<(const T& x);                 // Add and return reference
    Vector&  operator<<(T&& x);                      // Add move and return reference
    
    // Standard container interface
    const T         *begin() const;                  // Iterator to beginning
    const T         *end() const;                    // Iterator to end
    T               *begin();                        // Iterator to beginning
    T               *end();                          // Iterator to end
};
```

### STL Equivalent
```cpp
template <class T, class Allocator = std::allocator<T>>
class std::vector {
    T*        data;
    size_t    size;
    size_t    capacity;
    Allocator alloc;

public:
    void       push_back(const T& x);                // Add copy of item
    void       push_back(T&& x);                     // Add with move semantics
    template <class... Args> reference emplace_back(Args&&... args); // Create at end
    void       resize(size_t n);                     // Resize with default init
    void       resize(size_t n, const value_type& val); // Resize with value init
    size_type  size() const;                         // Get number of items
    bool       empty() const;                        // Check if empty
    void       clear();                              // Clear container
    reference  at(size_t i);                        // Access element with bounds check
    const_reference at(size_t i) const;              // Access element with bounds check
    reference  operator[](size_t i);                 // Access element (no bounds check)
    const_reference operator[](size_t i) const;       // Access element (no bounds check)
    void       reserve(size_t n);                    // Reserve capacity
    size_t     capacity() const;                     // Get allocated capacity
    iterator   erase(const_iterator pos);            // Remove element
    iterator   erase(const_iterator first, const_iterator last); // Remove range
    iterator   insert(const_iterator pos, const T& x); // Insert element
    iterator   insert(const_iterator pos, T&& x);    // Insert with move
    iterator   insert(const_iterator pos, size_type count, const T& value); // Insert count copies
    template< class InputIt > iterator insert(const_iterator pos, InputIt first, InputIt last); // Insert range
    template< class... Args > reference emplace(const_iterator pos, Args&&... args); // Emplace at position
    void       pop_back();                           // Remove last element
    reference  back();                               // Access last element
    const_reference back() const;                     // Access last element (const)
    iterator   begin();                              // Iterator to beginning
    iterator   end();                                // Iterator to end
    const_iterator begin() const;                     // Iterator to beginning
    const_iterator end() const;                       // Iterator to end
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Vector | std::vector | ✓ Complete | |
| Vector::Add() | std::vector::emplace_back() | ✓ Complete | U++ version returns reference to added element |
| Vector::Add(const T&) | std::vector::push_back(const T&) | ✓ Complete | |
| Vector::Add(T&&) | std::vector::push_back(T&&) | ✓ Complete | |
| Vector::AddN(n) | std::vector::resize(size + n) | ✓ Complete | |
| Vector::operator[] | std::vector::operator[] | ✓ Complete | |
| Vector::GetCount() | std::vector::size() | ✓ Complete | |
| Vector::IsEmpty() | std::vector::empty() | ✓ Complete | |
| Vector::Trim(n) | std::vector::resize(n) | ✓ Complete | |
| Vector::SetCount(n) | std::vector::resize(n) | ✓ Complete | |
| Vector::SetCount(n, const T&) | std::vector::resize(n, const T&) | ✓ Complete | |
| Vector::Clear() | std::vector::clear() | ✓ Complete | |
| Vector::At(i) | std::vector::resize() + back() or emplace_back() | ✓ Complete | Requires different approach |
| Vector::Reserve(n) | std::vector::reserve(n) | ✓ Complete | |
| Vector::GetAlloc() | std::vector::capacity() | ✓ Complete | |
| Vector::Remove(i, count) | std::vector::erase() | ✓ Complete | |
| Vector::Insert(i, const T&) | std::vector::insert() | ✓ Complete | |
| Vector::Append(x) | std::vector::insert(end(), ...) | ✓ Complete | |
| Vector::Top() | std::vector::back() | ✓ Complete | |
| Vector::Pop() | std::vector::pop_back() | ✓ Complete | |
| Vector::begin() | std::vector::begin() | ✓ Complete | |
| Vector::end() | std::vector::end() | ✓ Complete | |

### Conversion Notes
- U++ Vector::GetCount() corresponds to STL std::vector::size()
- U++ Vector::Add() corresponds to STL std::vector::emplace_back() (returns reference)
- U++ Vector::Add(const T&) corresponds to STL std::vector::push_back(const T&)
- U++ Vector::Top() corresponds to STL std::vector::back()
- U++ Vector::Pop() corresponds to STL std::vector::pop_back() but returns the removed element

## 2. Array ↔ std::vector of unique_ptr

### U++ Declaration
```cpp
template <class T>
class Array : public MoveableAndDeepCopyOption< Array<T> > {
protected:
    Vector<PointerType> vector;  // Vector of pointers

public:
    T&       Add();                                    // Add new default item
    T&       Add(const T& x);                         // Add copy of item
    T&       Add(T&& x);                              // Add with move semantics
    T&       Add(T *newt);                            // Add raw pointer
    T&       Add(One<T>&& one);                       // Add from One container
    template<class... Args> T& Create(Args&&... args); // Create new item in-place
    const T& operator[](int i) const;                 // Access element (const)
    T&       operator[](int i);                       // Access element
    int      GetCount() const;                        // Get number of items
    bool     IsEmpty() const;                         // Check if empty
    void     Trim(int n);                             // Trim to n items
    void     SetCount(int n);                         // Set count (default init)
    void     SetCount(int n, const T& init);          // Set count with init value
    void     SetCountR(int n);                        // Set count (resize)
    void     SetCountR(int n, const T& init);         // Set count with init value
    void     Clear();                                 // Clear container
    T&       At(int i);                               // Access with auto-resize
    T&       At(int i, const T& x);                   // Access with auto-resize
    void     Shrink();                                // Minimize memory usage
    void     Reserve(int xtra);                       // Reserve capacity
    int      GetAlloc() const;                        // Get allocated capacity
    void     Remove(int i, int count = 1);            // Remove elements
    void     Insert(int i, const T& x, int count);    // Insert multiple copies
    T&       Insert(int i, const T& x);               // Insert single item
    T&       Insert(int i, T&& x);                    // Insert with move
    void     Insert(int i, const Array& x);           // Insert array
    void     Insert(int i, const Array& x, int offset, int count); // Insert range
    void     Append(const Array& x);                  // Append array
    T&       Set(int i, T *newt);                     // Set with raw pointer
    T&       Insert(int i, T *newt);                  // Insert with raw pointer
    T       *Detach(int i);                           // Detach pointer at index
    T       *Swap(int i, T *newt);                    // Swap with new pointer
    T       *PopDetach();                             // Detach last element
    void     Swap(Array& b);                          // Swap with another array
};
```

### STL Equivalent
```cpp
template <class T, class Allocator = std::allocator<T>>
class std::vector<std::unique_ptr<T>> {
    std::unique_ptr<T>* data;
    size_t    size;
    size_t    capacity;
    Allocator alloc;

public:
    // Similar to std::vector but with unique_ptr elements
    std::unique_ptr<T>&       operator[](size_t i);
    const std::unique_ptr<T>& operator[](size_t i) const;
    size_t                    size() const;
    bool                      empty() const;
    void                      clear();
    // ... other methods adapted for unique_ptr elements
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Array | std::vector<std::unique_ptr<T>> | ✓ Complete | Array manages pointers to objects |
| Array::Add() | std::vector::emplace_back(std::make_unique<T>()) | ✓ Complete | |
| Array::Add(const T&) | std::vector::emplace_back(std::make_unique<T>(const T&)) | ✓ Complete | |
| Array::Add(T *newt) | std::vector::emplace_back(std::unique_ptr<T>(newt)) | ✓ Complete | |
| Array::operator[] | std::vector::operator[] | ✓ Complete | |
| Array::GetCount() | std::vector::size() | ✓ Complete | |
| Array::Clear() | std::vector::clear() | ✓ Complete | |
| Array::Remove(i, count) | std::vector::erase() | ✓ Complete | |
| Array::Detach(i) | std::vector::at(i).release() | ✓ Complete | Manually handle removal |
| Array::PopDetach() | std::vector::back().release() + pop_back() | ✓ Complete | |

### Conversion Notes
- U++ Array manages heap-allocated objects through pointers
- STL equivalent is std::vector<std::unique_ptr<T>>
- Array::Add(T *newt) can be replaced with std::vector::emplace_back(std::unique_ptr<T>(newt))
- Array::Detach() is similar to calling release() on the unique_ptr at that index
- Array::PopDetach() is similar to calling release() on the back element and then pop_back()

## 3. Index ↔ std::unordered_set or std::set

### U++ Declaration
```cpp
template <class T>
class Index : MoveableAndDeepCopyOption<Index<T>>, IndexCommon {
    Vector<T> key;

public:
    void        Add(const T& k);                      // Add key
    void        Add(T&& k);                           // Add key with move
    Index&      operator<<(const T& x);               // Add and return reference
    Index&      operator<<(T&& x);                    // Add move and return reference
    int         Find(const T& k) const;               // Find key index
    int         FindNext(int i) const;                // Find next occurrence
    int         FindLast(const T& k) const;           // Find last occurrence
    int         FindPrev(int i) const;                // Find previous occurrence
    int         FindAdd(const T& k);                  // Find or add key
    int         FindAdd(T&& k);                       // Find or add key with move
    int         Put(const T& k);                      // Put key (add if not exists)
    int         Put(T&& k);                           // Put key with move
    int         FindPut(const T& k, bool& p);         // Find or put with flag
    int         FindPut(T&& k, bool& p);              // Find or put with move
    void        Unlink(int i);                        // Unlink element
    int         UnlinkKey(const T& k);                // Unlink key
    bool        IsUnlinked(int i) const;              // Check if unlinked
    bool        HasUnlinked() const;                  // Check if has unlinked
    void        Sweep();                              // Clean up unlinked
    void        Set(int i, const T& k);               // Set key at index
    void        Set(int i, T&& k);                    // Set key with move
    const T&    operator[](int i) const;              // Access by index
    int         GetCount() const;                     // Get count
    bool        IsEmpty() const;                      // Check if empty
    void        Clear();                              // Clear container
    void        Trim(int n = 0);                      // Trim to n elements
    void        Drop(int n = 1);                      // Drop last n elements
    const T&    Top() const;                          // Access top element
    T           Pop();                                // Pop and return element
    void        Reserve(int n);                       // Reserve capacity
    void        Shrink();                             // Shrink to fit
    int         GetAlloc() const;                     // Get allocated capacity
    Vector<T>        PickKeys();                      // Extract keys vector
    const Vector<T>& GetKeys() const;                 // Get keys reference
};
```

### STL Equivalent
```cpp
template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<Key>>
class std::unordered_set {
    // Hash table implementation
public:
    std::pair<iterator, bool> insert(const value_type& x);  // Insert element
    std::pair<iterator, bool> insert(value_type&& x);       // Insert with move
    iterator       find(const Key& k) const;                // Find element
    bool           contains(const Key& k) const;            // Check if contains (C++20)
    size_t         size() const;                            // Size of container
    bool           empty() const;                           // Check if empty
    void           clear();                                 // Clear container
    iterator       erase(const_iterator pos);               // Remove element
    size_t         erase(const Key& k);                    // Remove by key
    // ... other methods
};

template <class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
class std::set {
    // Ordered tree implementation
public:
    std::pair<iterator, bool> insert(const value_type& x);  // Insert element
    std::pair<iterator, bool> insert(value_type&& x);       // Insert with move
    iterator       find(const Key& k) const;                // Find element
    size_t         count(const Key& k) const;               // Count occurrences
    size_t         size() const;                            // Size of container
    bool           empty() const;                           // Check if empty
    void           clear();                                 // Clear container
    // ... other methods
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Index | std::unordered_set or std::set | ✓ Complete | Depends on ordering needs |
| Index::Add(const T&) | std::unordered_set::insert(const T&) | ✓ Complete | |
| Index::Add(T&&) | std::unordered_set::insert(T&&) | ✓ Complete | |
| Index::Find(const T&) | std::unordered_set::find(const T&) | ✓ Complete | Returns iterator, not int |
| Index::Find(const T&) | std::unordered_set::count(const T&) > 0 | ✓ Complete | For boolean check |
| Index::FindAdd(const T&) | std::unordered_set::insert(const T&) | ✓ Complete | Returns pair<iterator, bool> |
| Index::GetCount() | std::unordered_set::size() | ✓ Complete | |
| Index::IsEmpty() | std::unordered_set::empty() | ✓ Complete | |
| Index::Clear() | std::unordered_set::clear() | ✓ Complete | |
| Index::operator[](int) | Not directly available | ⚠️ Complex | Need custom wrapper |

### Conversion Notes
- U++ Index provides both hash-based and ordered implementations, depending on the underlying implementation
- Most direct equivalent is std::unordered_set for fast lookup with index-based access
- Index::Find() returns an integer index, while std::unordered_set::find() returns an iterator
- For index-based access functionality, a custom wrapper around std::unordered_set or a combination of std::unordered_set and std::vector might be needed

## 4. Map ↔ std::map or std::unordered_map

### U++ Declaration
```cpp
template <class K, class T, class V>
class AMap {
protected:
    Index<K> key;  // Index of keys
    V        value; // Value container (Vector<T> or Array<T>)

public:
    T&       Add(const K& k, const T& x);              // Add key-value pair
    T&       Add(const K& k, T&& x);                  // Add with move
    T&       Add(const K& k);                         // Add with default value
    int      Find(const K& k) const;                  // Find value index
    int      FindNext(int i) const;                   // Find next occurrence
    int      FindLast(const K& k) const;              // Find last occurrence
    int      FindPrev(int i) const;                   // Find previous occurrence
    int      FindAdd(const K& k);                     // Find or add key
    int      FindAdd(const K& k, const T& init);      // Find or add with init
    T&       Put(const K& k);                         // Put or create
    int      Put(const K& k, const T& x);             // Put value
    int      Put(const K& k, T&& x);                  // Put with move
    int      PutDefault(const K& k);                  // Put with default
    int      FindPut(const K& k);                     // Find or put
    int      FindPut(const K& k, const T& init);      // Find or put with init
    T&       Get(const K& k);                         // Get value
    const T& Get(const K& k) const;                   // Get value (const)
    const T& Get(const K& k, const T& d) const;       // Get with default
    T&       GetAdd(const K& k);                      // Get or add
    T&       GetAdd(const K& k, const T& x);          // Get or add with value
    T&       GetAdd(const K& k, T&& x);               // Get or add with move
    T*       FindPtr(const K& k);                     // Find pointer
    const T* FindPtr(const K& k) const;               // Find pointer (const)
    void     Unlink(int i);                           // Unlink element
    int      UnlinkKey(const K& k);                   // Unlink by key
    bool     IsUnlinked(int i) const;                 // Check if unlinked
    void     Sweep();                                 // Clean up unlinked
    bool     HasUnlinked() const;                     // Check has unlinked
    const T& operator[](int i) const;                 // Access by index
    T&       operator[](int i);                       // Access by index
    int      GetCount() const;                        // Get count
    bool     IsEmpty() const;                         // Check if empty
    void     Clear();                                 // Clear container
    void     Shrink();                                // Shrink to fit
    void     Reserve(int xtra);                       // Reserve capacity
    int      GetAlloc() const;                        // Get allocated capacity
    const K& GetKey(int i) const;                     // Get key by index
    void     Remove(const int *sl, int n);            // Remove by index list
    void     Remove(const Vector<int>& sl);           // Remove by index vector
    template <typename P> void RemoveIf(P p);         // Remove if condition
};
```

### STL Equivalent
```cpp
template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
class std::map {
public:
    std::pair<iterator, bool> insert(const value_type& x);  // Insert pair
    std::pair<iterator, bool> insert(value_type&& x);       // Insert with move
    iterator       find(const Key& k);                      // Find element
    T&             at(const Key& k);                        // Access by key (with exception)
    const T&       at(const Key& k) const;                  // Access by key (const)
    T&             operator[](const Key& k);                // Access or create
    T&             operator[](Key&& k);                     // Access or create with move key
    size_t         size() const;                            // Size
    bool           empty() const;                           // Check if empty
    void           clear();                                 // Clear
    iterator       erase(const_iterator pos);               // Erase element
    size_t         erase(const Key& k);                    // Erase by key
    // ... other methods
};

template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
class std::unordered_map {
public:
    std::pair<iterator, bool> insert(const value_type& x);  // Insert pair
    std::pair<iterator, bool> insert(value_type&& x);       // Insert with move
    iterator       find(const Key& k);                      // Find element
    T&             at(const Key& k);                        // Access by key (with exception)
    const T&       at(const Key& k) const;                  // Access by key (const)
    T&             operator[](const Key& k);                // Access or create
    T&             operator[](Key&& k);                     // Access or create with move key
    size_t         size() const;                            // Size
    bool           empty() const;                           // Check if empty
    void           clear();                                 // Clear
    iterator       erase(const_iterator pos);               // Erase element
    size_t         erase(const Key& k);                    // Erase by key
    // ... other methods
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| AMap | std::map or std::unordered_map | ✓ Complete | |
| VectorMap | std::map or std::unordered_map | ✓ Complete | AMap with Vector<T> as value container |
| ArrayMap | std::map or std::unordered_map with unique_ptr<T> | ✓ Complete | AMap with Array<T> as value container |
| AMap::Add(k, v) | std::map::insert(std::make_pair(k, v)) | ✓ Complete | |
| AMap::Find(k) | std::map::find(k) != end() | ✓ Complete | Returns iterator, not int |
| AMap::Find(k) | std::map::count(k) > 0 | ✓ Complete | For boolean check |
| AMap::Get(k) | std::map::at(k) | ✓ Complete | |
| AMap::Get(k, default) | Manual implementation | ⚠️ Complex | Need manual code |
| AMap::operator[](k) | std::map::operator[](k) | ✓ Complete | |
| AMap::GetCount() | std::map::size() | ✓ Complete | |
| AMap::IsEmpty() | std::map::empty() | ✓ Complete | |
| AMap::Clear() | std::map::clear() | ✓ Complete | |
| AMap::Remove(key) | std::map::erase(key) | ✓ Complete | |
| AMap::GetKey(i) | Not directly available | ⚠️ Complex | Need to iterate or maintain parallel containers |

### Conversion Notes
- U++ Map implementations offer index-based access to elements, which std::map/std::unordered_map doesn't provide directly
- For direct equivalence, use std::map or std::unordered_map for key-value access
- For index-based access, maintain parallel containers or iterate to find the nth element
- VectorMap::GetKey() can be replaced by iterating to the nth element and getting its key
- AMap::Get(const K&, const T&) has no direct equivalent in STL; requires manual implementation

## 5. BiVector ↔ std::deque

### U++ Declaration
```cpp
template <class T>
class BiVector : MoveableAndDeepCopyOption< BiVector<T> > {
public:
    T&       AddHead();                                // Add at head
    T&       AddHead(const T& x);                     // Add at head with value
    T&       AddHead(T&& x);                          // Add at head with move
    void     AddHeadN(int n);                         // Add n at head
    T&       AddTail();                                // Add at tail
    T&       AddTail(const T& x);                     // Add at tail with value
    T&       AddTail(T&& x);                          // Add at tail with move
    void     AddTailN(int n);                         // Add n at tail
    T&       Add() { return AddTail(); }              // Add at tail (alias)
    T&       Add(const T& x) { return AddTail(x); }   // Add at tail (alias)
    T&       Add(T&& x) { return AddTail(pick(x)); }  // Add at tail (alias)
    T&       Insert(int i, const T& x);               // Insert at position
    T&       Insert(int i, T&& x);                    // Insert with move
    void     Remove(int i, int count = 1);            // Remove elements
    void     RemoveHead(int n = 1);                   // Remove from head
    void     RemoveTail(int n = 1);                   // Remove from tail
    const T& operator[](int i) const;                 // Access by index
    T&       operator[](int i);                       // Access by index
    const T& Head() const;                             // Access head
    T&       Head();                                   // Access head
    const T& Tail() const;                             // Access tail
    T&       Tail();                                   // Access tail
    int      GetCount() const;                        // Get count
    bool     IsEmpty() const;                         // Check if empty
    void     Clear();                                 // Clear
    void     Shrink();                                // Shrink to fit
    void     SetCount(int n);                         // Set count
    void     SetCount(int n, const T& init);          // Set count with init
    T&       At(int i);                               // Access with auto-resize
    T&       At(int i, const T& x);                   // Access with auto-resize
    void     TrimHead(int n);                         // Trim from head
    void     TrimTail(int n);                         // Trim from tail
};
```

### STL Equivalent
```cpp
template <class T, class Allocator = std::allocator<T>>
class std::deque {
public:
    void push_front(const T& x);                      // Add at front
    void push_front(T&& x);                           // Add at front with move
    void push_back(const T& x);                       // Add at back
    void push_back(T&& x);                            // Add at back with move
    iterator insert(const_iterator pos, const T& x);  // Insert at position
    iterator insert(const_iterator pos, T&& x);       // Insert with move
    iterator erase(const_iterator pos);               // Erase element
    iterator erase(const_iterator first, const_iterator last); // Erase range
    reference front();                                // Access first element
    reference back();                                 // Access last element
    const_reference front() const;                    // Access first element (const)
    const_reference back() const;                     // Access last element (const)
    size_t size() const;                             // Size
    bool empty() const;                              // Check if empty
    void clear();                                    // Clear
    reference operator[](size_t n);                  // Access element
    const_reference operator[](size_t n) const;       // Access element (const)
    // ... other methods
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| BiVector | std::deque | ✓ Complete | |
| BiVector::AddHead() | std::deque::push_front() | ✓ Complete | |
| BiVector::AddHead(const T&) | std::deque::push_front(const T&) | ✓ Complete | |
| BiVector::AddHead(T&&) | std::deque::push_front(T&&) | ✓ Complete | |
| BiVector::AddTail() | std::deque::push_back() | ✓ Complete | |
| BiVector::AddTail(const T&) | std::deque::push_back(const T&) | ✓ Complete | |
| BiVector::AddTail(T&&) | std::deque::push_back(T&&) | ✓ Complete | |
| BiVector::Head() | std::deque::front() | ✓ Complete | |
| BiVector::Tail() | std::deque::back() | ✓ Complete | |
| BiVector::RemoveHead(n) | std::deque::pop_front() called n times | ⚠️ Complex | For n=1, direct mapping |
| BiVector::RemoveTail(n) | std::deque::pop_back() called n times | ⚠️ Complex | For n=1, direct mapping |
| BiVector::operator[] | std::deque::operator[] | ✓ Complete | |
| BiVector::GetCount() | std::deque::size() | ✓ Complete | |

### Conversion Notes
- BiVector::AddHead() corresponds to std::deque::push_front()
- BiVector::AddTail() corresponds to std::deque::push_back()
- BiVector::Head() corresponds to std::deque::front()
- BiVector::Tail() corresponds to std::deque::back()
- BiVector::RemoveHead(n) and RemoveTail(n) for n>1 need loops or custom implementations
- BiVector supports efficient insertion/removal at both ends like std::deque

## 6. InVector ↔ std::vector (indirect)

### U++ Declaration
```cpp
template <class T>
class InVector : public MoveableAndDeepCopyOption< InVector<T> > {
public:
    T&       Insert(int i);                           // Insert empty at position
    T&       Insert(int i, const T& x);               // Insert at position
    void     InsertN(int i, int count);               // Insert multiple empty
    void     Remove(int i, int count = 1);            // Remove elements
    T&       Add();                                   // Add at end
    T&       Add(const T& x);                         // Add at end with value
    void     AddN(int n);                             // Add n at end
    int      GetCount() const;                        // Get count
    bool     IsEmpty() const;                         // Check if empty
    void     Trim(int n);                             // Trim to n elements
    void     SetCount(int n);                         // Set count
    void     Clear();                                 // Clear
    T&       At(int i);                               // Access with auto-resize
    void     Shrink();                                // Shrink to fit
    void     Set(int i, const T& x, int count);       // Set range
    T&       Set(int i, const T& x);                  // Set single item
    void     Swap(int i1, int i2);                   // Swap elements
    void     Drop(int n = 1);                        // Drop last n elements
    T&       Top();                                   // Access last element
    const T& Top() const;                             // Access last element (const)
    T        Pop();                                   // Remove and return last element
    const T& operator[](int i) const;                 // Access by index
    T&       operator[](int i);                       // Access by index
};
```

### STL Equivalent
```cpp
template <class T, class Allocator = std::allocator<T>>
class std::vector {
    // Same as Vector mapping
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| InVector | std::vector | ✓ Complete | Implementation differs but interface similar |
| InVector::Insert(int, const T&) | std::vector::insert(iterator, const T&) | ✓ Complete | |
| InVector::Remove(i, count) | std::vector::erase() | ✓ Complete | |
| InVector::Add(const T&) | std::vector::push_back(const T&) | ✓ Complete | |

### Conversion Notes
- InVector has a more complex internal implementation (chunked storage) for better performance when inserting/removing in the middle
- Interface is very similar to Vector and can be mapped to std::vector
- Performance characteristics differ: InVector is optimized for middle-element operations

## Summary of Container Mappings

| U++ Container | STL Equivalent | Notes |
|---------------|----------------|-------|
| Vector | std::vector | Direct mapping for most operations |
| Array | std::vector<std::unique_ptr<T>> | Manages heap-allocated objects |
| Index | std::unordered_set or std::set | For key-based lookup without values |
| Map | std::map or std::unordered_map | Key-value pairs |
| BiVector | std::deque | Efficient operations at both ends |
| InVector | std::vector | Similar interface, different implementation |