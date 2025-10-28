#pragma once
#ifndef _Core_Mt_h_
#define _Core_Mt_h_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <functional>
#include "Core.h"

// This header should work with MULTITHREADED defined, but since stdsrc is single-threaded,
// we'll implement it in a single-threaded safe way

#define flagST // Indicate that we are in single-threaded mode

#ifdef DEPRECATED
#define thread__ thread_local
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
class Function<Res(ArgTypes...)>; // Forward declaration

class Thread : NoCopy {
	std::thread thread_obj;
	bool running = false;
	int stack_size = 0;

public:
	bool       Run(Function<void ()> cb, bool noshutdown = false);
	bool       RunNice(Function<void ()> cb, bool noshutdown = false);
	bool       RunCritical(Function<void ()> cb, bool noshutdown = false);

	void       Detach();
	int        Wait();

	bool       IsOpen() const     { return running; }

	typedef std::thread::native_handle_type Handle;
	typedef std::thread::id Id;

	Id          GetId() const                  { return thread_obj.get_id(); }

	Handle      GetHandle() const              { return thread_obj.native_handle(); }
	
	bool        Priority(int percent); // 0 = lowest, 100 = normal
	void        StackSize(int bytes)           { stack_size = bytes; }
	
	void        Nice()                         { Priority(25); }
	void        Critical()                     { Priority(150); }

	static void Start(Function<void ()> cb, bool noshutdown = false) {
		// In single-threaded mode, execute immediately
		cb();
	}
	static void StartNice(Function<void ()> cb, bool noshutdown = false) {
		Start(cb, noshutdown);
	}
	static void StartCritical(Function<void ()> cb, bool noshutdown = false) {
		Start(cb, noshutdown);
	}

	static void Sleep(int ms) {
		// In single-threaded mode, this could be a stub or use std::this_thread::sleep_for
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	static bool IsST() { return true; } // We are single-threaded
	static bool IsMain() { return true; } // In single-threaded mode, we're always the main thread
	static bool IsUpp() { return true; } 
	static int  GetCount() { return 0; }
	static void BeginShutdownThreads() {}
	static void AtShutdown(void (*shutdownfn)()) {}
	static void TryShutdownThreads() {}
	static void EndShutdownThreads() {}
	static void ShutdownThreads() {}
	static bool IsShutdownThreads() { return false; }
	static void (*AtExit(void (*exitfn)()))() { return nullptr; }

	static void Exit() {}
	
	static void DumpDiagnostics() {}

	static Handle GetCurrentHandle() { return std::this_thread::get_id(); }
	static inline Id GetCurrentId()  { return std::this_thread::get_id(); }

	Thread() = default;
	~Thread() = default;

private:
	void operator=(const Thread&) = delete;
	Thread(const Thread&) = delete;
};

#ifdef _DEBUG
inline void AssertST() { ASSERT(Thread::IsST()); }
#else
inline void AssertST() {}
#endif

class Semaphore : NoCopy {
	std::mutex mtx;
	std::condition_variable cv;
	unsigned long count;

public:
	bool       Wait(int timeout_ms = -1) {
		std::unique_lock<std::mutex> lock(mtx);
		if (timeout_ms < 0) {
			// Wait indefinitely
			cv.wait(lock, [this] { return count > 0; });
			--count;
			return true;
		} else {
			// Wait with timeout
			if (cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
			                [this] { return count > 0; })) {
				--count;
				return true;
			}
			return false;
		}
	}
	
	void       Release() {
		std::lock_guard<std::mutex> lock(mtx);
		++count;
		cv.notify_one();
	}
	
	void       Release(int n) {
		std::lock_guard<std::mutex> lock(mtx);
		count += n;
		cv.notify_all();
	}

	Semaphore() : count(0) {}
	~Semaphore() = default;
};

struct MtInspector;

class Mutex : NoCopy {
protected:
	mutable std::mutex mtx;
#ifdef flagPROFILEMT
	MtInspector        *mti;
#endif

	friend class ConditionVariable;

public:
#ifdef flagPROFILEMT
	bool  TryEnter() { return mtx.try_lock(); }
	void  Enter()    { 
		if(!mtx.try_lock()) { 
			// In single-threaded mode, this won't happen, but keeping for compatibility
			mtx.lock(); 
		}
	}
	void  Set(MtInspector& m) { mti = &m; }
#else
	bool  TryEnter() { return mtx.try_lock(); }
	void  Enter()    { mtx.lock(); }
#endif
	void  Leave()    { mtx.unlock(); }

	Mutex() {
#ifdef flagPROFILEMT
		mti = MtInspector::Dumi();
#endif
	}
	~Mutex() = default;

	class Lock;
};

class RWMutex : NoCopy {
	mutable std::shared_mutex rwmtx;

public:
	void EnterWrite()  { rwmtx.lock(); }
	void LeaveWrite()  { rwmtx.unlock(); }
	void EnterRead()   { rwmtx.lock_shared(); }
	void LeaveRead()   { rwmtx.unlock_shared(); }

	RWMutex() = default;
	~RWMutex() = default;

	class ReadLock;
	class WriteLock;
};

class ConditionVariable {
	std::condition_variable_any cv;
	
public:
	bool Wait(Mutex& m, int timeout_ms = -1) {
		if (timeout_ms < 0) {
			cv.wait(m.mtx, [] { return true; });
			return true;
		} else {
			return cv.wait_for(m.mtx, std::chrono::milliseconds(timeout_ms), 
			                   [] { return true; });
		}
	}

	void Signal()        { cv.notify_one(); }
	void Broadcast()     { cv.notify_all(); }
	
	ConditionVariable() = default;
	~ConditionVariable() = default;
};

typedef std::atomic<bool> OnceFlag;

#define ONCELOCK_(o_b_) \
for(static ::Upp::Mutex o_ss_; !atomic_load_explicit(&o_b_, std::memory_order_acquire);) \
	for(::Upp::Mutex::Lock o_ss_lock__(o_ss_); !atomic_load_explicit(&o_b_, std::memory_order_acquire); \
	    atomic_store_explicit(&o_b_, true, std::memory_order_release))

#define ONCELOCK \
for(static std::atomic<bool> o_b_; !atomic_load_explicit(&o_b_, std::memory_order_acquire);) ONCELOCK_(o_b_)

class Mutex::Lock : NoCopy {
	Mutex& s;

public:
	Lock(Mutex& s) : s(s) { s.Enter(); }
	~Lock()               { s.Leave(); }
};

class RWMutex::ReadLock : NoCopy {
	RWMutex& s;

public:
	ReadLock(RWMutex& s) : s(s) { s.EnterRead(); }
	~ReadLock()                 { s.LeaveRead(); }
};

class RWMutex::WriteLock : NoCopy {
	RWMutex& s;

public:
	WriteLock(RWMutex& s) : s(s) { s.EnterWrite(); }
	~WriteLock()                 { s.LeaveWrite(); }
};

template <class Primitive>
class StaticPrimitive_ : NoCopy {
	alignas(Primitive) byte buffer[sizeof(Primitive)];
	Primitive *primitive = nullptr;
	OnceFlag   once{false};
	
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
#ifdef flagPROFILEMT
	void Set(MtInspector& mti) { Get().Set(mti); }
#endif
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
	mutable std::atomic<bool>  dirty{true};

public:
	void Invalidate() { dirty = true; }
	bool BeginUpdate() const { 
		if (dirty.load()) {
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
			// Add a small delay to prevent busy waiting
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

// For auxthread compatibility in single-threaded mode
#define auxthread_t void *
#define auxthread__

bool StartAuxThread(auxthread_t (auxthread__ *fn)(void *ptr), void *ptr) {
	// In single-threaded mode, execute immediately
	if (fn) {
		fn(ptr);
		return true;
	}
	return false;
}

#endif