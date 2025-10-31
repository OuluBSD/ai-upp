#pragma once
#ifndef _Core_St_h_
#define _Core_St_h_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <functional>
#include "Core.h"

// Single-threaded implementations of threading primitives
#if SINGLETHREADED
#ifdef DEPRECATED
#define thread__
#endif

#ifdef flagPROFILEMT
class Mutex;
class StaticMutex;

struct MtInspector {
	const char *name;
	int   number;
	int   locked;
	int   blocked;
	
	static MtInspector *Dumi();

	MtInspector(const char *s, int n = -1) { name = s; number = n; locked = blocked = 0; }
	~MtInspector();
};

#define PROFILEMT(mutex) \
	{ static MtInspector MK__s(__FILE__, __LINE__); mutex.Set(MK__s); }

#define PROFILEMT_(mutex, id) \
	{ static MtInspector MK__s(id); mutex.Set(MK__s); }

#else

#define PROFILEMT(mutex)
#define PROFILEMT_(mutex, id)

#endif

template<typename Res, typename... ArgTypes>
class Function<Res(ArgTypes...)>;

class Thread : NoCopy {
	int stack_size = 0;

public:
	bool       Run(Function<void ()> cb, bool noshutdown = false) { cb(); return true; }
	bool       RunNice(Function<void ()> cb, bool noshutdown = false) { cb(); return true; }
	bool       RunCritical(Function<void ()> cb, bool noshutdown = false) { cb(); return true; }

	void       Detach() {}
	int        Wait() { return 0; }

	bool       IsOpen() const     { return false; }

	typedef int Handle;
	typedef int Id;

	Id          GetId() const                  { return 0; }

	Handle      GetHandle() const              { return 0; }
	
	bool        Priority(int percent) { return true; } // 0 = lowest, 100 = normal
	void        StackSize(int bytes)           { stack_size = bytes; }
	
	void        Nice()                         { Priority(25); }
	void        Critical()                     { Priority(150); }

	static void Start(Function<void ()> cb, bool noshutdown = false) { cb(); }
	static void StartNice(Function<void ()> cb, bool noshutdown = false) { Start(cb, noshutdown); }
	static void StartCritical(Function<void ()> cb, bool noshutdown = false) { Start(cb, noshutdown); }

	static void Sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

	static bool IsST() { return true; }
	static bool IsMain() { return true; }
	static bool IsUpp() { return true; }
	static int  GetCount() { return 1; }
	static void BeginShutdownThreads() {}
	static void AtShutdown(void (*shutdownfn)()) {}
	static void TryShutdownThreads() {}
	static void EndShutdownThreads() {}
	static void ShutdownThreads() {}
	static bool IsShutdownThreads() { return false; }
	static void (*AtExit(void (*exitfn)()))() { return nullptr; }

	static void Exit() {}
	
	static void DumpDiagnostics() {}

	static Handle GetCurrentHandle()          { return 0; }
	static inline Id GetCurrentId()           { return 0; }

	Thread() = default;
	~Thread() = default;

private:
	void operator=(const Thread&) = delete;
	Thread(const Thread&) = delete;
};

inline void AssertST() {}

class Semaphore : NoCopy {
private:
    std::atomic<int> count;
    std::mutex mtx;
    std::condition_variable cv;

public:
	bool       Wait(int timeout_ms = -1) {
        std::unique_lock<std::mutex> lock(mtx);
        if (timeout_ms < 0) {
            cv.wait(lock, [this] { return count.load() > 0; });
        } else {
            cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                       [this] { return count.load() > 0; });
        }
        if (count.load() > 0) {
            count.fetch_sub(1);
            return true;
        }
        return false;
    }
	
	void       Release() {
        count.fetch_add(1);
        cv.notify_one();
    }
	
	void       Release(int n) {
        count.fetch_add(n);
        cv.notify_all();
    }

    Semaphore() : count(0) {}
	~Semaphore() = default;
};

struct MtInspector;


class Mutex : NoCopy {
protected:
    std::mutex mtx;
	MtInspector        *mti = nullptr;

	Mutex(int)         {}

	friend class ConditionVariable;

public:
	bool  TryEnter() { return mtx.try_lock(); }
	void  Leave()    { mtx.unlock(); }

	void  Enter()    { mtx.lock(); }

	Mutex()          { }
	~Mutex()         { }

	class Lock;
};

class RWMutex : NoCopy {
private:
    std::shared_mutex rwmtx;
	
public:
	void EnterWrite()  { rwmtx.lock(); }
	void LeaveWrite()  { rwmtx.unlock(); }

	void EnterRead()   { rwmtx.lock_shared(); }
	void LeaveRead()   { rwmtx.unlock_shared(); }

	RWMutex() = default;
	~RWMutex() = default;
};

class ConditionVariable {
private:
    std::condition_variable cv;
	
public:
	bool Wait(Mutex& m, int timeout_ms = -1) {
        if (timeout_ms < 0) {
            cv.wait(m.mtx);
            return true;
        } else {
            return cv.wait_for(m.mtx, std::chrono::milliseconds(timeout_ms)) 
                   == std::cv_status::no_timeout;
        }
    }
	
	void Signal()        { cv.notify_one(); }
	void Broadcast()     { cv.notify_all(); }
	
	ConditionVariable() = default;
	~ConditionVariable() = default;
};


typedef bool OnceFlag;

#define ONCELOCK_(o_b_) \
for(static bool o_b_ = false; !o_b_; o_b_ = true)

#define ONCELOCK \
for(static bool ___ = false; !___; ___ = true)


class Mutex::Lock : NoCopy {
	Mutex& s;

public:
	Lock(Mutex& s) : s(s) { s.Enter(); }
	~Lock()               { s.Leave(); }
};

template <class Primitive>
class StaticPrimitive_ : NoCopy {
    alignas(Primitive) byte buffer[sizeof(Primitive)];
    Primitive *primitive = nullptr;
    OnceFlag   once = false;
	
	void Initialize() { primitive = new(buffer) Primitive; }

public:
	Primitive& Get()  { 
        ONCELOCK_(once) Initialize(); 
        return *primitive; 
    }
};

class StaticMutex : StaticPrimitive_<Mutex> {
public:
	operator Mutex&()          { return Get(); }
	bool TryEnter()            { return Get().TryEnter();}
	void Enter()               { Get().Enter();}
	void Leave()               { Get().Leave(); }
};

class StaticSemaphore : StaticPrimitive_<Semaphore> {
public:
	operator Semaphore&()        { return Get(); }
	void Wait()                  { Get().Wait(); }
	void Release()               { Get().Release(); }
};

class StaticRWMutex : StaticPrimitive_<RWMutex> {
public:
	operator RWMutex&()  { return Get(); }
	void EnterRead()     { Get().EnterRead();}
	void LeaveRead()     { Get().LeaveRead(); }
	void EnterWrite()    { Get().EnterWrite();}
	void LeaveWrite()    { Get().LeaveWrite(); }
};

class StaticConditionVariable : StaticPrimitive_<ConditionVariable> {
public:
	operator ConditionVariable&() { return Get(); }
	void Wait(Mutex& m)  { Get().Wait(m); }
	void Signal()        { Get().Signal(); }
	void Broadcast()     { Get().Broadcast(); }
};

class LazyUpdate {
	mutable Mutex              mutex;
	mutable bool               dirty = true;

public:
	void Invalidate() { dirty = true; }
	bool BeginUpdate() const { 
        if (dirty) {
            dirty = false;
            return true;
        }
        return false;
    }
	void EndUpdate() const {}

	LazyUpdate() = default;
};

inline bool IsMainThread() { 
    static std::thread::id main_id = std::this_thread::get_id();
    return std::this_thread::get_id() == main_id;
}

struct SpinLock : Moveable<SpinLock> {
    std::atomic<bool> locked{false};

	bool TryEnter() { 
        bool expected = false;
        return locked.compare_exchange_strong(expected, true);
    }
	void Leave()    { locked.store(false); }

	void Enter()    { 
        bool expected = false;
        while (!locked.compare_exchange_weak(expected, true)) {
            expected = false;
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }
	
	void Wait() {
        while (locked.load()) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }
	
	class Lock;

	SpinLock() = default;
};

class SpinLock::Lock : NoCopy {
	SpinLock& s;

public:
	Lock(SpinLock& s) : s(s) { s.Enter(); }
	~Lock()                  { s.Leave(); }
};

#define INTERLOCKED \
for(bool i_b_ = true; i_b_;) \
	for(static UPP::Mutex i_ss_; i_b_;) \
		for(UPP::Mutex::Lock i_ss_lock__(i_ss_); i_b_; i_b_ = false)

struct H_l_ : Mutex::Lock {
	bool b;
	H_l_(Mutex& cs) : Mutex::Lock(cs) { b = true; }
};

#define INTERLOCKED_(cs) \
for(UPP::H_l_ i_ss_lock__(cs); i_ss_lock__.b; i_ss_lock__.b = false)

#ifdef DEPRECATED
typedef Mutex CriticalSection;
typedef StaticMutex StaticCriticalSection;
#endif

#define auxthread_t void *
#define auxthread__

bool StartAuxThread(auxthread_t (auxthread__ *fn)(void *ptr), void *ptr) {
    // Execute immediately in single-threaded mode
    fn(ptr);
    return true;
}

#endif

#endif