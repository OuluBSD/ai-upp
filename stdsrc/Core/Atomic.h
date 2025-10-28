#pragma once
#ifndef _Core_Atomic_h_
#define _Core_Atomic_h_

#include <atomic>

typedef std::atomic<int> Atomic;

inline int  AtomicInc(volatile Atomic& t)             { return ++t; }
inline int  AtomicDec(volatile Atomic& t)             { return --t; }

#endif