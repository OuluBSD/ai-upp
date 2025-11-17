# U++ to STL Mapping: Core Package Utilities

This document provides comprehensive mapping between U++ Core package utility types and their STL equivalents.

## 1. Tuple ↔ std::tuple

### U++ Declaration
```cpp
template <int N, typename... T>
struct TupleN;

template <int I>
struct IndexI__ {};

template <typename V, typename T, typename I>
const V& GetFromTuple(const T& t, const I&);

template <typename... T>
struct Tuple;

template <typename A>
struct TupleN<1, A> {
    A a;
    using T1 = A;

    bool  operator==(const TupleN& x) const;        // Equality comparison
    int   Compare(const TupleN& x) const;           // Comparison
    void  ToHash(CombineHash& h) const;             // Hash combination
    void  ToString(String& r) const;                // String representation
    void  Serialize(Stream& s);                     // Serialization
    int   GetCount() const;                         // Get element count
    Value Get(int i) const;                         // Get element by index
    void  Set(int i, const Value& v);               // Set element by index
    TupleN(const A& a);                             // Constructor
    TupleN();                                       // Default constructor
    template <typename AA> operator Tuple<AA>();    // Type conversion
};

#define TUPLE_N_METHODS(M, I) \
    bool operator==(const TupleN& x) const { return Base::operator==(x) && M == x.M; } \
    int  Compare(const TupleN& x) const { int q = Base::Compare(x); return q ? q : SgnCompare(M, x.M); } \
    void ToHash(CombineHash& h) const { Base::ToHash(h); h << M; } \
    void ToString(String& r) const { Base::ToString(r); r << ", " << M; } \
    void Serialize(Stream& s) { Base::Serialize(s); s % M; } \
    int   GetCount() const { return I + 1; } \
    Value Get(int i) const { if(i == I) return M; return Base(i); } \
    void  Set(int i, const Value& v) { if(i == I) M = v; else Base::Set(i, v); } \
    TupleN() {}

// Inherited implementations for 2, 3, 4 elements...
template <typename A, typename B>
struct TupleN<2, A, B> : public TupleN<1, A> {
    typedef TupleN<1, A> Base;
    B b;
    using T2 = B;
    TUPLE_N_METHODS(b, 1);
    TupleN(const A& a, const B& b);
    template <typename AA, typename BB> operator Tuple<AA, BB>();
};

template <typename A, typename B, typename C>
struct TupleN<3, A, B, C> : public TupleN<2, A, B> {
    typedef TupleN<2, A, B> Base;
    C c;
    using T3 = C;
    TUPLE_N_METHODS(c, 2);
    TupleN(const A& a, const B& b, const C& c);
    template <typename AA, typename BB, typename CC> operator Tuple<AA, BB, CC>();
};

template <typename A, typename B, typename C, typename D>
struct TupleN<4, A, B, C, D> : public TupleN<3, A, B, C> {
    typedef TupleN<3, A, B, C> Base;
    D d;
    using T4 = D;
    TUPLE_N_METHODS(d, 3);
    TupleN(const A& a, const B& b, const C& c, const D& d);
    template <typename AA, typename BB, typename CC, typename DD> operator Tuple<AA, BB, CC, DD>();
};

#define GET_FROM_TUPLE(M, I) \
template <typename T> \
auto GetFromTuple(const T& t, const IndexI__<I>&) -> decltype(t.M)& \
{ return const_cast<T&>(t).M; } \
template <typename T> \
auto GetFromTupleByType(const T& t, decltype(t.M)*, const IndexI__<I>* = NULL) -> decltype(t.M)& \
{ return const_cast<T&>(t).M; }

GET_FROM_TUPLE(a, 0)
GET_FROM_TUPLE(b, 1)
GET_FROM_TUPLE(c, 2)
GET_FROM_TUPLE(d, 3)

template <typename... Args>
struct Tuple : public TupleN<sizeof...(Args), Args...> {
private:
    typedef TupleN<sizeof...(Args), Args...> Base;

public:
    template <int I>
    const auto& Get() const { return GetFromTuple(*this, IndexI__<I>()); }
    template <int I>
    auto& Get() { return GetFromTuple(*this, IndexI__<I>()); }

    template <int I> // std compatibility & C++17 structured binding support
    const auto& get() const { return GetFromTuple(*this, IndexI__<I>()); }
    template <int I> // std compatibility & C++17 structured binding support
    auto& get() { return GetFromTuple(*this, IndexI__<I>()); }

    template <typename T> const T& Get() const { return GetFromTupleByType(*this, (T*)NULL); }
    template <typename T> T& Get() { return GetFromTupleByType(*this, (T*)NULL); }

    int  GetCount() const;                          // Get element count
    bool operator==(const Tuple& x) const;         // Equality comparison
    bool operator!=(const Tuple& x) const;         // Inequality comparison
    int  Compare(const Tuple& x) const;            // Comparison
    bool operator<=(const Tuple& x) const;         // Less or equal
    bool operator>=(const Tuple& x) const;         // Greater or equal
    bool operator<(const Tuple& x) const;          // Less than
    bool operator>(const Tuple& x) const;          // Greater than
    hash_t GetHashValue() const;                   // Hash value
    void Serialize(Stream& s);                     // Serialization
    void Jsonize(JsonIO& j);                       // JSON conversion
    String ToString() const;                       // String representation
    Value Get(int i) const;                        // Get element by index
    void  Set(int i, const Value& v);              // Set element by index
    ValueArray GetArray() const;                   // Get as ValueArray
    void  SetArray(const ValueArray& va);          // Set from ValueArray
    Tuple();                                       // Default constructor
    Tuple(const Args... args);                     // Constructor with args
};

template <typename T, typename U>
inline T *FindTuple(T *x, int n, const U& key);   // Find in array of tuples

// Tie utility functions for structured binding
template <typename A, typename B>
struct Tie2 { A& a; B& b; void operator=(const Tuple<A, B>& s); Tie2(A& a, B& b); };
template <typename A, typename B> Tie2<A, B> Tie(A& a, B& b);

template <typename A, typename B, typename C>
struct Tie3 { A& a; B& b; C& c; void operator=(const Tuple<A, B, C>& s); Tie3(A& a, B& b, C& c); };
template <typename A, typename B, typename C> Tie3<A, B, C> Tie(A& a, B& b, C& c);

template <typename A, typename B, typename C, typename D>
struct Tie4 { A& a; B& b; C& c; D& d; void operator=(const Tuple<A, B, C, D>& s); Tie4(A& a, B& b, C& c, D& d); };
template <typename A, typename B, typename C, typename D> Tie4<A, B, C, D> Tie(A& a, B& b, C& c, D& d);

// Type aliases for backward compatibility
template <typename A, typename B> using Tuple2 = Tuple<A, B>;
template <typename A, typename B, typename C> using Tuple3 = Tuple<A, B, C>;
template <typename A, typename B, typename C, typename D> using Tuple4 = Tuple<A, B, C, D>;
```

### STL Equivalent
```cpp
#include <tuple>
template< class... Types >
class std::tuple {
public:
    // Various constructors
    constexpr tuple();  // Default constructor (only if all types are default-constructible)
    template< class... UTypes > constexpr tuple(UTypes&&... args);  // Constructor with args
    template< class... UTypes > constexpr tuple(const std::tuple<UTypes...>& other);  // Copy constructor
    template< class... UTypes > constexpr tuple(std::tuple<UTypes...>&& other);  // Move constructor
    // ... other constructors

    // Assignment operators
    template< class... UTypes > constexpr tuple& operator=(const std::tuple<UTypes...>& other);
    template< class... UTypes > constexpr tuple& operator=(std::tuple<UTypes...>&& other);

    // No public members - elements accessed via std::get
};

namespace std {
    template< size_t I, class... Types >
    constexpr tuple_element_t<I, tuple<Types...>>& get(tuple<Types...>& t) noexcept;  // Get by index
    
    template< size_t I, class... Types >
    constexpr tuple_element_t<I, tuple<Types...>>&& get(tuple<Types...>&& t) noexcept;  // Get by index (rvalue)
    
    template< size_t I, class... Types >
    constexpr const tuple_element_t<I, tuple<Types...>>& get(const tuple<Types...>& t) noexcept;  // Get by index (const)
    
    template< class T, class... Types >
    constexpr T& get(tuple<Types...>& t) noexcept;  // Get by type
    
    template< class T, class... Types >
    constexpr T&& get(tuple<Types...>&& t) noexcept;  // Get by type (rvalue)
    
    template< class T, class... Types >
    constexpr const T& get(const tuple<Types...>& t) noexcept;  // Get by type (const)
}

template< class... Types >
constexpr bool operator==(const tuple<Types...>& lhs, const tuple<Types...>& rhs);  // Equality
template< class... Types >
constexpr bool operator!=(const tuple<Types...>& lhs, const tuple<Types...>& rhs);  // Inequality
template< class... Types >
constexpr bool operator<(const tuple<Types...>& lhs, const tuple<Types...>& rhs);   // Less than
template< class... Types >
constexpr bool operator<=(const tuple<Types...>& lhs, const tuple<Types...>& rhs);  // Less or equal
template< class... Types >
constexpr bool operator>(const tuple<Types...>& lhs, const tuple<Types...>& rhs);   // Greater than
template< class... Types >
constexpr bool operator>=(const tuple<Types...>& lhs, const tuple<Types...>& rhs);  // Greater or equal

template< class... Types >
std::tuple<Types&...> tie(Types&... args);  // Create tuple of lvalue references
template< class... Types >
std::tuple<Types&&...> forward_as_tuple(Types&&... args);  // Create tuple of rvalue references

// Helper variable template
template< class T >
constexpr size_t tuple_size_v = std::tuple_size<T>::value;

// Structured bindings support
template<size_t I, class... Types>
constexpr auto& get(std::tuple<Types...>& t) noexcept;  // For structured bindings
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Tuple | std::tuple | ✓ Complete | |
| Tuple<A, B> | std::tuple<A, B> | ✓ Complete | |
| Tuple<A, B, C> | std::tuple<A, B, C> | ✓ Complete | |
| Tuple<A, B, C, D> | std::tuple<A, B, C, D> | ✓ Complete | |
| Tuple::Get<I>() | std::get<I>(tuple) | ✓ Complete | |
| Tuple::Get<I>() const | std::get<I>(const tuple) | ✓ Complete | |
| Tuple::get<I>() | std::get<I>(tuple) | ✓ Complete | For C++17 structured binding compatibility |
| Tuple(const Args... args) | std::tuple(Args... args) | ✓ Complete | |
| std compatibility methods | std::tuple methods | ✓ Complete | |
| Tie(a, b) | std::tie(a, b) | ✓ Complete | |
| Tie(a, b, c) | std::tie(a, b, c) | ✓ Complete | |
| Tie(a, b, c, d) | std::tie(a, b, c, d) | ✓ Complete | |
| Tuple::GetCount() | std::tuple_size<Tuple>::value or std::tuple_size_v<Tuple> (C++17) | ✓ Complete | |

### Conversion Notes
- U++ Tuple provides direct member access (a, b, c, d) while std::tuple only provides access through std::get
- Both support comparison operators with similar semantics
- U++ Tuple::Get<I>() maps directly to std::get<I>(tuple)
- U++ Tuple::get<I>() provides C++17 structured binding compatibility
- U++ Tie utility maps directly to std::tie for structured binding
- U++ Tuple supports serialization and JSON conversion which std::tuple doesn't have by default
- U++ Tuple::GetCount() can be replaced with std::tuple_size_v (C++17) or std::tuple_size::value

## 2. Optional ↔ std::optional (C++17)

### U++ Declaration
```cpp
// In U++ this is defined as an alias to std::optional
template <class T> using Optional = std::optional<T>;

// U++ also provides utility functions
template <class T> std::optional<T> MakeOptional(const T& o) {return std::make_optional(o);}
```

### STL Equivalent
```cpp
#include <optional>
template< class T >
class std::optional {
public:
    using value_type = T;

    constexpr optional() noexcept;                  // Default constructor (empty)
    constexpr optional(std::nullopt_t) noexcept;    // Constructor from nullopt
    optional(const optional& other);                // Copy constructor
    optional(optional&& other) noexcept(see below); // Move constructor
    template< class U > optional(U&& value);        // Constructor from value
    template< class U > explicit optional(std::in_place_t, U&&... args); // In-place construction
    // ... other constructors

    ~optional();                                   // Destructor

    optional& operator=(std::nullopt_t) noexcept;  // Assignment from nullopt
    optional& operator=(const optional& other);    // Copy assignment
    optional& operator=(optional&& other) noexcept; // Move assignment
    template< class U > optional& operator=(U&& value); // Assignment from value
    template< class... Args > T& emplace(Args&&... args); // In-place construction

    constexpr const T* operator->() const;         // Pointer access (const)
    constexpr T* operator->();                     // Pointer access
    constexpr const T& operator*() const&;         // Dereference (const lvalue)
    constexpr T& operator*() &;                    // Dereference (lvalue)
    constexpr T&& operator*() &&;                  // Dereference (rvalue)
    constexpr const T&& operator*() const&&;       // Dereference (const rvalue)

    constexpr explicit operator bool() const noexcept; // Boolean conversion
    constexpr bool has_value() const noexcept;     // Check if has value
    constexpr T& value();                          // Get value with exception if empty
    constexpr const T& value() const;              // Get value with exception if empty
    template< class U > constexpr T value_or(U&& default_value) const&; // Get value or default
    template< class U > constexpr T value_or(U&& default_value) &&;     // Get value or default (rvalue)
    // ... other methods
};

namespace std {
    constexpr struct nullopt_t { explicit nullopt_t(int) {} } nullopt{0}; // Null state indicator
}
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Optional<T> | std::optional<T> | ✓ Complete | U++ defines this as an alias |
| Optional() | std::optional<T>() | ✓ Complete | |
| Optional(value) | std::optional<T>(value) | ✓ Complete | |
| Optional(std::nullopt) | std::optional<T>(std::nullopt) | ✓ Complete | |
| optional.has_value() | optional.has_value() | ✓ Complete | |
| optional.operator bool() | optional.operator bool() | ✓ Complete | |
| optional.value() | optional.value() | ✓ Complete | |
| optional.value_or(default) | optional.value_or(default) | ✓ Complete | |
| *optional | optional.operator*() | ✓ Complete | |
| optional-> | optional.operator->() | ✓ Complete | |
| MakeOptional(value) | std::make_optional(value) | ✓ Complete | |

### Conversion Notes
- U++ Optional is defined as an alias to std::optional, making them identical
- All functionality is directly equivalent
- U++ provides MakeOptional as a wrapper for std::make_optional
- The std::nullopt sentinel is available in both systems
- Exception behavior is the same when accessing empty optionals

## 3. Value ↔ std::any or variant-based system

### U++ Declaration
```cpp
class Value : Moveable<Value> {
public:
    class Void {  // Base class for value types
    protected:
        Atomic  refcount;                          // Reference counter

    public:
        void               Retain();               // Increment ref count
        void               Release();              // Decrement ref count
        int                GetRefCount() const;    // Get reference count
        virtual bool       IsNull() const;         // Check if null
        virtual void       Serialize(Stream& s);   // Serialization
        virtual void       Xmlize(XmlIO& xio);     // XML conversion
        virtual void       Jsonize(JsonIO& jio);   // JSON conversion
        virtual hash_t     GetHashValue() const;   // Hash value
        virtual bool       IsEqual(const Void *p); // Equality check
        virtual bool       IsPolyEqual(const Value& v); // Polymorphic equality
        virtual String     AsString() const;       // String representation
        virtual int        Compare(const Void *p); // Comparison
        virtual int        PolyCompare(const Value& p); // Polymorphic comparison
        Void();                                    // Constructor
        virtual ~Void();                           // Destructor
    };

    struct Sval {  // Operations for small value types
        bool       (*IsNull)(const void *p);
        void       (*Serialize)(void *p, Stream& s);
        void       (*Xmlize)(void *p, XmlIO& xio);
        void       (*Jsonize)(void *p, JsonIO& jio);
        hash_t     (*GetHashValue)(const void *p);
        bool       (*IsEqual)(const void *p1, const void *p2);
        bool       (*IsPolyEqual)(const void *p, const Value& v);
        String     (*AsString)(const void *p);
        int        (*Compare)(const void *p1, const void *p2);
        int        (*PolyCompare)(const void *p1, const Value& p2);
    };

    // Predefined value type constants
    static const dword INT_V     = 1;
    static const dword DOUBLE_V  = 2;
    static const dword STRING_V  = 3;
    static const dword DATE_V    = 4;
    static const dword TIME_V    = 5;
    static const dword ERROR_V   = 6;
    static const dword VALUE_V   = 7;
    static const dword WSTRING_V = 8;
    static const dword VALUEARRAY_V = 9;
    static const dword INT64_V   = 10;
    static const dword BOOL_V    = 11;
    static const dword VALUEMAP_V = 12;
    static const dword FLOAT_V   = 13;
    static const dword UNKNOWN_V = (dword)0xffffffff;
    static const dword VOID_V    = 0;

    template <class T>
    static void Register(const char *name = NULL);  // Register custom type
    template <class T>
    static void SvoRegister(const char *name = NULL); // Register small value type

    dword    GetType() const;                       // Get type identifier
    bool     IsError() const;                      // Check if error
    bool     IsVoid() const;                       // Check if void
    bool     IsNull() const;                       // Check if null
    template <class T> bool     Is() const;        // Check if specific type
    template <class T> const T& To() const;        // Cast to specific type
    template <class T> const T& Get() const;       // Get as specific type

    // Type conversions
    operator String() const;
    operator WString() const;
    operator Date() const;
    operator Time() const;
    operator double() const;
    operator float() const;
    operator int() const;
    operator int64() const;
    operator bool() const;
    std::string  ToStd() const;
    std::wstring ToWStd() const;

    // Constructors for basic types
    Value(const String& s);
    Value(const WString& s);
    Value(const char *s);
    Value(int i);
    Value(int64 i);
    Value(double d);
    Value(float d);
    Value(bool b);
    Value(Date d);
    Value(Time t);
    Value(const Nuller&);  // Null value
    Value(const std::string& s);
    Value(const std::wstring& s);

    // Container operations (when Value represents ValueArray or ValueMap)
    int   GetCount() const;
    const Value& operator[](int i) const;
    const Value& operator[](const String& key) const;
    const Value& operator[](const char *key) const;
    const Value& operator[](const Id& key) const;
    Value& At(int i);
    Value& operator()(int i);               // Access or create at index
    void   Add(const Value& src);
    template <typename T> Value& operator<<(const T& src);
    Value& GetAdd(const Value& key);       // Access or create at key
    Value& operator()(const String& key);  // Access or create at key
    Value& operator()(const char *key);    // Access or create at key
    Value& operator()(const Id& key);      // Access or create at key

    // Basic Value operations
    bool operator==(const Value& v) const;
    bool operator!=(const Value& v) const;
    int  Compare(const Value& v) const;
    bool operator<=(const Value& x) const;
    bool operator>=(const Value& x) const;
    bool operator<(const Value& x) const;
    bool operator>(const Value& x) const;
    String ToString() const;
    String operator ~() const;
    String GetTypeName() const;
    void  Serialize(Stream& s);
    void  Xmlize(XmlIO& xio);
    void  Jsonize(JsonIO& jio);
    hash_t GetHashValue() const;
    Value& operator=(const Value& v);
    Value(const Value& v);
    Value();  // Default constructor (void value)
    ~Value(); // Destructor
};
```

### STL Equivalent
```cpp
// For type-erased values, std::any is the closest equivalent:
#include <any>
class std::any {
public:
    constexpr any() noexcept;                       // Default constructor (empty)
    any(const any& other);                          // Copy constructor
    any(any&& other) noexcept;                      // Move constructor
    template<typename ValueType> any(ValueType&& value); // Constructor from value
    // ... constructors

    ~any();                                        // Destructor

    any& operator=(const any& rhs);                // Copy assignment
    any& operator=(any&& rhs) noexcept;            // Move assignment
    template<typename ValueType> any& operator=(ValueType&& rhs); // Assignment from value
    void reset() noexcept;                          // Reset to empty
    void swap(any& rhs) noexcept;                   // Swap

    bool has_value() const noexcept;                // Check if non-empty
    const std::type_info& type() const noexcept;    // Get type info

    friend void swap(any& x, any& y) noexcept;      // Swap function
};

namespace std {
    template<typename ValueType> 
    ValueType any_cast(const any& operand);         // Cast to value (const)
    template<typename ValueType>
    ValueType any_cast(any& operand);               // Cast to value
    template<typename ValueType>
    ValueType any_cast(any&& operand);              // Cast to value (rvalue)
    template<typename ValueType>
    const ValueType* any_cast(const any* operand) noexcept; // Cast to pointer (const)
    template<typename ValueType>
    ValueType* any_cast(any* operand) noexcept;     // Cast to pointer
}

// For sum types, std::variant can be used:
#include <variant>
template< class... Types >
class std::variant {
public:
    // Construction and assignment
    // Visit operations
    // Get operations
    // Type checking
    // ... all variant functionality
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Value | std::any or std::variant | ⚠️ Complex | Exact equivalent depends on usage |
| Value::Is<T>() | std::any_cast<T*>(&any_value) != nullptr or std::holds_alternative<T>(variant) | ⚠️ Complex | Usage varies |
| Value::Get<T>() | std::any_cast<T>(any_value) or std::get<T>(variant) | ⚠️ Complex | |
| Value(int i) | std::any(int i) or std::variant containing int | ⚠️ Complex | |
| Value::GetType() | std::any::type() or std::variant::index() | ⚠️ Complex | Depends on implementation |
| operator==(const Value&, const Value&) | Manual implementation | ⚠️ Complex | Need custom visitor for variant |
| Value::ToString() | Manual implementation | ⚠️ Complex | Need custom visitor for variant |
| Value::Serialize() | Manual implementation | ⚠️ Complex | Need custom visitor for variant |

### Conversion Notes
- U++ Value is a complex type that can hold many different types with built-in serialization
- Closest STL equivalents are std::any (for type-erased values) or std::variant (for sum types with known options)
- std::any provides type-erased storage but lacks built-in comparison, serialization, and string conversion
- std::variant provides sum type functionality but requires knowing all possible types at compile time
- For complete feature parity, a custom variant-based implementation would be needed
- Value::operator[] and Value::GetAdd functionality requires a custom container for holding Value objects

## 4. Function ↔ std::function

### U++ Declaration
```cpp
template<typename Res, typename... ArgTypes>
class Function<Res(ArgTypes...)> : Moveable<Function<Res(ArgTypes...)>> {
    struct WrapperBase {
        Atomic  refcount;                          // Reference counter
        virtual Res Execute(ArgTypes... args) = 0; // Execute function
        WrapperBase();                             // Constructor
        virtual ~WrapperBase();                    // Destructor
    };

    template <class F>
    struct Wrapper : WrapperBase {
        F fn;                                      // The stored function
        virtual Res Execute(ArgTypes... args) { return fn(args...); } // Execute
        Wrapper(F&& fn) : fn(pick(fn)) {}         // Constructor with pick
    };

    WrapperBase *ptr;                              // Pointer to wrapper

public:
    Function();                                    // Default constructor
    Function(CNULLer);                             // Constructor from null
    Function(const Nuller&);                       // Constructor from null
    template <class F> Function(F fn);             // Constructor from callable
    Function(const Function& src);                 // Copy constructor
    Function& operator=(const Function& src);      // Copy assignment
    Function(Function&& src);                      // Move constructor
    Function& operator=(Function&& src);           // Move assignment
    Res operator()(ArgTypes... args) const;        // Call operator
    operator bool() const;                         // Boolean conversion
    void Clear();                                  // Clear function
    ~Function();                                   // Destructor
    friend Function Proxy(const Function& a);      // Create proxy
    friend void Swap(Function& a, Function& b);    // Swap function
};
```

### STL Equivalent
```cpp
#include <functional>
template< class Signature >
class std::function;

template< class R, class... ArgTypes >
class std::function<R(ArgTypes...)> {
public:
    using result_type = R;                         // Result type

    std::function() noexcept;                      // Default constructor (empty)
    std::function(std::nullptr_t) noexcept;        // Constructor from nullptr
    template< class F > std::function(F&& f);      // Constructor from callable
    template< class A > std::function(std::allocator_arg_t, const A& alloc); // With allocator
    template< class A > std::function(std::allocator_arg_t, const A& alloc, std::nullptr_t); // With allocator
    template< class F, class A > std::function(std::allocator_arg_t, const A& alloc, F&& f); // With allocator
    std::function(const std::function& other);     // Copy constructor
    std::function(std::function&& other);          // Move constructor
    ~std::function();                              // Destructor

    std::function& operator=(const std::function& other); // Copy assignment
    std::function& operator=(std::function&& other); // Move assignment
    std::function& operator=(std::nullptr_t);      // Assignment from nullptr
    template< class F > std::function& operator=(F&& f); // Assignment from callable
    template< class F > std::function& operator=(std::reference_wrapper<F> f); // Assignment from ref wrapper

    R operator()(ArgTypes... args) const;          // Call operator
    void swap(std::function& other) noexcept;      // Swap
    explicit operator bool() const noexcept;       // Boolean conversion
    const std::type_info& target_type() const noexcept; // Target type info
    template< class T > T* target() noexcept;      // Target access
    template< class T > const T* target() const noexcept; // Target access (const)
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Function<Res(ArgTypes...)> | std::function<Res(ArgTypes...)> | ✓ Complete | |
| Function() | std::function<Res(ArgTypes...)>() | ✓ Complete | |
| Function(callable) | std::function<Res(ArgTypes...)>(callable) | ✓ Complete | |
| Function(const Function&) | std::function<Res(ArgTypes...)>(const std::function&) | ✓ Complete | |
| Function(Function&&) | std::function<Res(ArgTypes...)>(std::function&&) | ✓ Complete | |
| Function::operator() | std::function::operator() | ✓ Complete | |
| Function::operator bool() | std::function::operator bool() | ✓ Complete | |
| Function::Clear() | std::function::operator=(nullptr) | ✓ Complete | |
| Function& operator=(const Function&) | std::function& operator=(const std::function&) | ✓ Complete | |
| Function& operator=(Function&&) | std::function& operator=(std::function&&) | ✓ Complete | |

### Conversion Notes
- U++ Function maps directly to std::function for storing and calling arbitrary callable objects
- Both provide type-erased storage of any callable that matches the signature
- Both provide move semantics and reference counting for the stored callable
- U++ Function doesn't provide target_type() or target() methods like std::function
- The behavior and performance characteristics are very similar

## 5. Callback ↔ std::function<void(...)>

### U++ Declaration
```cpp
template <class... ArgTypes>
class CallbackN : Moveable<CallbackN<ArgTypes...>> {
    typedef Function<void (ArgTypes...)> Fn;        // Use Function internally
    Fn fn;                                          // The stored function

public:
    CallbackN();                                    // Default constructor
    CallbackN(const CallbackN& src);                // Copy constructor
    CallbackN& operator=(const CallbackN& src);     // Copy assignment
    CallbackN(Fn&& src, int);                       // Constructor from Function (internal)
    template <class F> CallbackN(F src, int);       // Constructor from F (internal)
    CallbackN(CallbackN&& src);                     // Move constructor
    CallbackN& operator=(CallbackN&& src);          // Move assignment
    CallbackN(CNULLer);                             // Constructor from null
    CallbackN& operator=(CNULLer);                  // Assignment from null
    CallbackN Proxy() const;                        // Create proxy
    template <class F> CallbackN& operator<<(const F& f); // Chain function
    template <class F> CallbackN& operator<<(F&& f); // Chain function (move)
    void operator()(ArgTypes... args) const;        // Call operator
    operator Fn() const;                            // Conversion to Function
    operator bool() const;                          // Boolean conversion
    void Clear();                                   // Clear callback
    friend CallbackN Proxy(const CallbackN& a);     // Create proxy
    friend void Swap(CallbackN& a, CallbackN& b);   // Swap
};
```

### STL Equivalent
```cpp
// CallbackN<void> maps to std::function<void()>
template <>
class std::function<void()> {
    // Same as above but for void return type
};

template <typename... ArgTypes>
using Callback = std::function<void(ArgTypes...)>;
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| CallbackN<ArgTypes...> | std::function<void(ArgTypes...)> | ✓ Complete | |
| CallbackN() | std::function<void(ArgTypes...)>(); | ✓ Complete | |
| CallbackN(callable) | std::function<void(ArgTypes...)>(callable); | ✓ Complete | |
| CallbackN::operator()(args...) | std::function::operator()(args...); | ✓ Complete | |
| CallbackN::operator bool() | std::function::operator bool(); | ✓ Complete | |
| CallbackN::Clear() | std::function::operator=(nullptr); | ✓ Complete | |
| CallbackN& operator<<(F) | Manual chaining implementation | ⚠️ Complex | Need custom implementation |

### Conversion Notes
- U++ CallbackN is specifically for void-returning functions, equivalent to std::function<void(...)>
- Most functionality maps directly to std::function
- The operator<< chaining mechanism would need custom implementation in STL
- CallbackN is implemented using Function internally, which maps to std::function

## Summary of Utility Mappings

| U++ Utility Type | STL Equivalent | Notes |
|------------------|----------------|-------|
| Tuple | std::tuple | Direct mapping with similar functionality |
| Optional | std::optional | U++ defines this as an alias |
| Value | std::any or std::variant | Complex mapping depending on use case |
| Function | std::function | Direct mapping with similar functionality |
| Callback | std::function<void(...)> | Specialized case of Function |