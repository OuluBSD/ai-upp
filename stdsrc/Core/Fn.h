#pragma once
#ifndef _Core_Fn_h_
#define _Core_Fn_h_

#include <functional>
#include "Core.h"

template <class T>
using Fn = std::function<T>;

// Helper to create function from lambda or function pointer
template <class T>
Fn<T> MakeFn(T&& fn) {
    return std::forward<T>(fn);
}

// Specific function types commonly used
using VoidFn = Fn<void()>;
using BoolFn = Fn<bool()>;

// Function composition utilities
template <class F, class G>
auto Compose(F&& f, G&& g) {
    return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&&... args) {
        return f(g(std::forward<decltype(args)>(args)...));
    };
}

#endif