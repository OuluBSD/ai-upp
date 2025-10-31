#pragma once
#ifndef _Core_Other_h_
#define _Core_Other_h_

#include <type_traits>
#include <utility>
#include <memory>
#include <functional>
#include "Core.h"

// Other.h - miscellaneous utility functions and classes for stdsrc

// Type traits for detecting various properties
template<typename T>
struct IsPointerLike {
    static constexpr bool value = std::is_pointer_v<T> || 
                                 std::is_same_v<T, std::nullptr_t>;
};

template<typename T>
struct IsContainer {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>().begin(), 
        std::declval<U>().end(), 
        std::true_type{}
    );
    
    template<typename>
    static std::false_type test(...);
    
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// Utility functions for working with containers
template<typename Container>
auto Size(const Container& container) -> decltype(container.size()) {
    return container.size();
}

template<typename T, size_t N>
constexpr size_t Size(T (&array)[N]) {
    return N;
}

template<typename Container>
bool IsEmpty(const Container& container) {
    if constexpr (requires { container.empty(); }) {
        return container.empty();
    } else {
        return Size(container) == 0;
    }
}

template<typename Container>
auto Begin(Container& container) -> decltype(container.begin()) {
    return container.begin();
}

template<typename Container>
auto End(Container& container) -> decltype(container.end()) {
    return container.end();
}

template<typename Container>
auto Begin(const Container& container) -> decltype(container.begin()) {
    return container.begin();
}

template<typename Container>
auto End(const Container& container) -> decltype(container.end()) {
    return container.end();
}

// Safe pointer operations
template<typename T>
T* SafePtr(T* ptr) {
    return ptr ? ptr : nullptr;
}

template<typename T>
const T* SafePtr(const T* ptr) {
    return ptr ? ptr : nullptr;
}

template<typename T>
T& SafeRef(T* ptr, T& default_ref) {
    return ptr ? *ptr : default_ref;
}

template<typename T>
const T& SafeRef(const T* ptr, const T& default_ref) {
    return ptr ? *ptr : default_ref;
}

// Null object pattern implementation
template<typename T>
class NullObject {
public:
    static const T& Instance() {
        static T null_instance;
        return null_instance;
    }
};

// Optional-like wrapper
template<typename T>
class Maybe {
private:
    std::unique_ptr<T> value;
    
public:
    Maybe() : value(nullptr) {}
    
    Maybe(const T& val) : value(std::make_unique<T>(val)) {}
    
    Maybe(T&& val) : value(std::make_unique<T>(std::move(val))) {}
    
    Maybe(std::nullptr_t) : value(nullptr) {}
    
    Maybe(const Maybe& other) {
        if (other.value) {
            value = std::make_unique<T>(*other.value);
        }
    }
    
    Maybe(Maybe&& other) noexcept : value(std::move(other.value)) {}
    
    Maybe& operator=(const T& val) {
        value = std::make_unique<T>(val);
        return *this;
    }
    
    Maybe& operator=(T&& val) {
        value = std::make_unique<T>(std::move(val));
        return *this;
    }
    
    Maybe& operator=(std::nullptr_t) {
        value.reset();
        return *this;
    }
    
    Maybe& operator=(const Maybe& other) {
        if (this != &other) {
            if (other.value) {
                value = std::make_unique<T>(*other.value);
            } else {
                value.reset();
            }
        }
        return *this;
    }
    
    Maybe& operator=(Maybe&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }
    
    const T& operator*() const { return *value; }
    T& operator*() { return *value; }
    
    const T* operator->() const { return value.get(); }
    T* operator->() { return value.get(); }
    
    const T& Value() const { 
        if (!value) {
            throw std::runtime_error("Accessing null Maybe");
        }
        return *value;
    }
    
    T& Value() { 
        if (!value) {
            throw std::runtime_error("Accessing null Maybe");
        }
        return *value;
    }
    
    const T& ValueOr(const T& default_value) const {
        return value ? *value : default_value;
    }
    
    T& ValueOr(T& default_value) {
        return value ? *value : default_value;
    }
    
    bool HasValue() const { return value != nullptr; }
    bool IsNull() const { return value == nullptr; }
    
    void Reset() { value.reset(); }
    
    explicit operator bool() const { return HasValue(); }
    
    bool operator==(const Maybe& other) const {
        if (IsNull() && other.IsNull()) return true;
        if (IsNull() || other.IsNull()) return false;
        return *value == *other.value;
    }
    
    bool operator!=(const Maybe& other) const {
        return !(*this == other);
    }
    
    bool operator==(const T& other) const {
        return HasValue() && *value == other;
    }
    
    bool operator!=(const T& other) const {
        return !(*this == other);
    }
};

// Visitor pattern implementation
template<typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

// Variant visitor helper
template<typename... Ts>
auto MakeVisitor(Ts&&... lambdas) {
    return Overload<std::decay_t<Ts>...>{std::forward<Ts>(lambdas)...};
}

// Type erasure for callable objects
class AnyCallable {
private:
    struct CallableBase {
        virtual ~CallableBase() = default;
        virtual void Call() = 0;
        virtual std::unique_ptr<CallableBase> Clone() const = 0;
    };
    
    template<typename F>
    struct CallableImpl : CallableBase {
        F func;
        
        CallableImpl(F f) : func(std::move(f)) {}
        
        void Call() override {
            func();
        }
        
        std::unique_ptr<CallableBase> Clone() const override {
            return std::make_unique<CallableImpl<F>>(func);
        }
    };
    
    std::unique_ptr<CallableBase> callable;
    
public:
    AnyCallable() = default;
    
    template<typename F>
    AnyCallable(F f) : callable(std::make_unique<CallableImpl<F>>(std::move(f))) {}
    
    void operator()() {
        if (callable) {
            callable->Call();
        }
    }
    
    AnyCallable(const AnyCallable& other) 
        : callable(other.callable ? other.callable->Clone() : nullptr) {}
    
    AnyCallable(AnyCallable&& other) noexcept 
        : callable(std::move(other.callable)) {}
    
    AnyCallable& operator=(const AnyCallable& other) {
        if (this != &other) {
            callable = other.callable ? other.callable->Clone() : nullptr;
        }
        return *this;
    }
    
    AnyCallable& operator=(AnyCallable&& other) noexcept {
        if (this != &other) {
            callable = std::move(other.callable);
        }
        return *this;
    }
};

// Scope guard for automatic cleanup
template<typename F>
class ScopeGuard {
private:
    F cleanup_func;
    bool active;
    
public:
    ScopeGuard(F f) : cleanup_func(std::move(f)), active(true) {}
    
    ~ScopeGuard() {
        if (active) {
            cleanup_func();
        }
    }
    
    void Dismiss() { active = false; }
    void Activate() { active = true; }
    
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    
    ScopeGuard(ScopeGuard&& other) noexcept 
        : cleanup_func(std::move(other.cleanup_func)), active(other.active) {
        other.active = false;
    }
    
    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            if (active) {
                cleanup_func();
            }
            cleanup_func = std::move(other.cleanup_func);
            active = other.active;
            other.active = false;
        }
        return *this;
    }
};

template<typename F>
auto MakeScopeGuard(F f) {
    return ScopeGuard<F>(std::move(f));
}

// Resource wrapper with automatic cleanup
template<typename T, typename CleanupFunc>
class AutoResource {
private:
    T resource;
    CleanupFunc cleanup;
    bool owned;
    
public:
    AutoResource(T res, CleanupFunc cf) 
        : resource(res), cleanup(cf), owned(true) {}
    
    AutoResource(const AutoResource&) = delete;
    AutoResource& operator=(const AutoResource&) = delete;
    
    AutoResource(AutoResource&& other) noexcept 
        : resource(other.resource), cleanup(std::move(other.cleanup)), owned(other.owned) {
        other.owned = false;
    }
    
    AutoResource& operator=(AutoResource&& other) noexcept {
        if (this != &other) {
            if (owned) {
                cleanup(resource);
            }
            resource = other.resource;
            cleanup = std::move(other.cleanup);
            owned = other.owned;
            other.owned = false;
        }
        return *this;
    }
    
    ~AutoResource() {
        if (owned) {
            cleanup(resource);
        }
    }
    
    T Get() const { return resource; }
    T operator*() const { return resource; }
    T* operator->() { return &resource; }
    const T* operator->() const { return &resource; }
    
    void Release() { owned = false; }
    bool IsOwned() const { return owned; }
};

template<typename T, typename CleanupFunc>
auto MakeAutoResource(T resource, CleanupFunc cleanup) {
    return AutoResource<T, CleanupFunc>(resource, std::move(cleanup));
}

// Function traits for introspection
template<typename T>
struct FunctionTraits;

template<typename R, typename... Args>
struct FunctionTraits<R(Args...)> {
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

template<typename R, typename C, typename... Args>
struct FunctionTraits<R(C::*)(Args...)> {
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

template<typename R, typename C, typename... Args>
struct FunctionTraits<R(C::*)(Args...) const> {
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

template<typename F>
struct FunctionTraits : FunctionTraits<decltype(&F::operator())> {};

// Type list utilities
template<typename... Ts>
struct TypeList {};

template<typename T, typename TL>
struct Prepend;

template<typename T, typename... Ts>
struct Prepend<T, TypeList<Ts...>> {
    using type = TypeList<T, Ts...>;
};

template<typename TL>
struct Size;

template<typename... Ts>
struct Size<TypeList<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

// Concatenate two type lists
template<typename TL1, typename TL2>
struct Concat;

template<typename... Ts1, typename... Ts2>
struct Concat<TypeList<Ts1...>, TypeList<Ts2...>> {
    using type = TypeList<Ts1..., Ts2...>;
};

// Find the first type in a type list that satisfies a predicate
template<template<typename> typename Predicate, typename TL>
struct FindIf;

template<template<typename> typename Predicate, typename T, typename... Ts>
struct FindIf<Predicate, TypeList<T, Ts...>> {
    using type = std::conditional_t<
        Predicate<T>::value,
        T,
        typename FindIf<Predicate, TypeList<Ts...>>::type
    >;
};

template<template<typename> typename Predicate>
struct FindIf<Predicate, TypeList<>> {
    using type = void; // No matching type found
};

// Check if any type in the list satisfies the predicate
template<template<typename> typename Predicate, typename TL>
struct AnyOf;

template<template<typename> typename Predicate, typename T, typename... Ts>
struct AnyOf<Predicate, TypeList<T, Ts...>> {
    static constexpr bool value = Predicate<T>::value || AnyOf<Predicate, TypeList<Ts...>>::value;
};

template<template<typename> typename Predicate>
struct AnyOf<Predicate, TypeList<>> {
    static constexpr bool value = false;
};

// Filter types in a type list based on a predicate
template<template<typename> typename Predicate, typename TL>
struct Filter;

template<template<typename> typename Predicate, typename T, typename... Ts>
struct Filter<Predicate, TypeList<T, Ts...>> {
    using type = typename std::conditional_t<
        Predicate<T>::value,
        typename Prepend<T, typename Filter<Predicate, TypeList<Ts...>>::type>::type,
        typename Filter<Predicate, TypeList<Ts...>>::type
    >;
};

template<template<typename> typename Predicate>
struct Filter<Predicate, TypeList<>> {
    using type = TypeList<>;
};

// Transform each type in a type list using a metafunction
template<template<typename> typename MetaFunction, typename TL>
struct Transform;

template<template<typename> typename MetaFunction, typename T, typename... Ts>
struct Transform<MetaFunction, TypeList<T, Ts...>> {
    using type = typename Prepend<typename MetaFunction<T>::type, 
                                   typename Transform<MetaFunction, TypeList<Ts...>>::type>::type;
};

template<template<typename> typename MetaFunction>
struct Transform<MetaFunction, TypeList<>> {
    using type = TypeList<>;
};

// Utilities for working with variant types
template<typename... Ts>
class Variant {
private:
    using Storage = std::aligned_union_t<0, Ts...>;
    Storage storage;
    std::size_t index;
    
    template<typename T>
    static constexpr std::size_t TypeIndex() {
        constexpr std::array<bool, sizeof...(Ts)> matches = { std::is_same_v<T, Ts>... };
        for (std::size_t i = 0; i < sizeof...(Ts); ++i) {
            if (matches[i]) return i;
        }
        return static_cast<std::size_t>(-1);
    }
    
public:
    Variant() : index(static_cast<std::size_t>(-1)) {}
    
    template<typename T>
    Variant(T&& value) : index(TypeIndex<std::decay_t<T>>()) {
        static_assert(TypeIndex<std::decay_t<T>>() != static_cast<std::size_t>(-1), "Type not in variant");
        new(&storage) std::decay_t<T>(std::forward<T>(value));
    }
    
    ~Variant() {
        if (index != static_cast<std::size_t>(-1)) {
            Destroy();
        }
    }
    
    template<typename T>
    bool Is() const {
        return index == TypeIndex<T>();
    }
    
    template<typename T>
    T& Get() {
        if (!Is<T>()) {
            throw std::bad_cast();
        }
        return *reinterpret_cast<T*>(&storage);
    }
    
    template<typename T>
    const T& Get() const {
        if (!Is<T>()) {
            throw std::bad_cast();
        }
        return *reinterpret_cast<const T*>(&storage);
    }
    
    template<typename T>
    T* GetIf() {
        if (!Is<T>()) {
            return nullptr;
        }
        return reinterpret_cast<T*>(&storage);
    }
    
    template<typename T>
    const T* GetIf() const {
        if (!Is<T>()) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(&storage);
    }
    
    std::size_t Index() const { return index; }
    
    template<typename Visitor>
    auto Visit(Visitor&& visitor) -> decltype(visitor(std::declval<std::variant_alternative_t<0, std::variant<Ts...>>())) {
        return VisitImpl(std::forward<Visitor>(visitor), std::make_index_sequence<sizeof...(Ts)>{});
    }
    
private:
    void Destroy() {
        // Destroy the active object
        ([](auto* storage, std::size_t index, auto... types) {
            ((index == 0 ? (storage->template emplace<0>().~Ts(), true) : false) || ...);
        }(&storage, index, Ts{}...));
    }
    
    template<typename Visitor, std::size_t... Is>
    auto VisitImpl(Visitor&& visitor, std::index_sequence<Is...>) -> decltype(visitor(std::declval<std::variant_alternative_t<0, std::variant<Ts...>>>())) {
        using ReturnType = decltype(visitor(std::declval<std::variant_alternative_t<0, std::variant<Ts...>>>()));
        if (index == static_cast<std::size_t>(-1)) {
            throw std::runtime_error("Visiting empty variant");
        }
        
        ReturnType* result = nullptr;
        ([&] {
            if (index == Is) {
                using Type = std::variant_alternative_t<Is, std::variant<Ts...>>;
                result = new ReturnType(visitor(*reinterpret_cast<Type*>(&storage)));
                return true;
            }
            return false;
        }() || ...);
        
        if (!result) {
            throw std::runtime_error("Visit failed");
        }
        
        return *result;
    }
};

// String utilities
inline std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

inline std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

inline std::string Join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::string result;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) result += delimiter;
        result += strings[i];
    }
    
    return result;
}

inline std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

inline std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

inline std::string ToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() && str.compare(0, prefix.length(), prefix) == 0;
}

inline bool EndsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() && 
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// Mathematical utilities
template<typename T>
T Clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

template<typename T>
T Lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

template<typename T>
T SmoothStep(T edge0, T edge1, T x) {
    T t = Clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

// Hash combining utilities
template <class T>
inline void HashCombine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <typename T>
inline void HashCombine(std::size_t& seed, T&& v) {
    std::hash<std::decay_t<T>> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// Compile-time string hashing
constexpr std::size_t HashString(const char* str, std::size_t hash = 0) {
    return (*str == 0) ? hash : HashString(str + 1, hash * 1099511628211ull ^ *str);
}

// Utility for creating named constants with strong typing
template<typename Tag, typename Type>
class NamedType {
public:
    using UnderlyingType = Type;
    
    explicit NamedType(const Type& value) : value_(value) {}
    explicit NamedType(Type&& value) : value_(std::move(value)) {}
    
    const Type& Get() const { return value_; }
    Type& Get() { return value_; }
    
    bool operator==(const NamedType& other) const { return value_ == other.value_; }
    bool operator!=(const NamedType& other) const { return value_ != other.value_; }
    bool operator<(const NamedType& other) const { return value_ < other.value_; }
    bool operator<=(const NamedType& other) const { return value_ <= other.value_; }
    bool operator>(const NamedType& other) const { return value_ > other.value_; }
    bool operator>=(const NamedType& other) const { return value_ >= other.value_; }
    
    NamedType& operator+=(const Type& other) { value_ += other; return *this; }
    NamedType& operator-=(const Type& other) { value_ -= other; return *this; }
    NamedType& operator*=(const Type& other) { value_ *= other; return *this; }
    NamedType& operator/=(const Type& other) { value_ /= other; return *this; }
    
    NamedType operator+(const Type& other) const { return NamedType(value_ + other); }
    NamedType operator-(const Type& other) const { return NamedType(value_ - other); }
    NamedType operator*(const Type& other) const { return NamedType(value_ * other); }
    NamedType operator/(const Type& other) const { return NamedType(value_ / other); }
    
    std::string ToString() const {
        if constexpr (std::is_arithmetic_v<Type>) {
            return std::to_string(value_);
        } else {
            return std::string(value_);
        }
    }
    
private:
    Type value_;
};

// Strong typing helpers
#define DEFINE_NAMED_TYPE(Name, Type) \
    using Name = NamedType<struct Name##Tag, Type>;

// Example usage:
// DEFINE_NAMED_TYPE(EmailAddress, std::string)
// DEFINE_NAMED_TYPE(UserId, int64_t)

#endif