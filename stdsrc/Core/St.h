#pragma once
#ifndef _Core_St_h_
#define _Core_St_h_

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Core.h"

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
	bool       Run(Function<void ()> cb, bool noshutdown = false) {
		// In single-threaded mode, just execute immediately
		cb();
		return true;
	}
	bool       RunNice(Function<void ()> cb, bool noshutdown = false) { 
		return Run(cb, noshutdown); 
	}
	bool       RunCritical(Function<void ()> cb, bool noshutdown = false) { 
		return Run(cb, noshutdown); 
	}

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

	static void Start(Function<void ()> cb, bool noshutdown = false) {
		cb(); // Execute immediately in single-threaded mode
	}
	static void StartNice(Function<void ()> cb, bool noshutdown = false) {
		Start(cb, noshutdown);
	}
	static void StartCritical(Function<void ()> cb, bool noshutdown = false) {
		Start(cb, noshutdown);
	}

	static void Sleep(int ms) {
		// In single-threaded mode, this might be a no-op or use std::this_thread::sleep_for
		// For now, using a placeholder
	}

	static bool IsST() { return true; }
	static bool IsMain() { return true; }
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

	static Handle GetCurrentHandle()          { return 0; }
	static inline Id GetCurrentId()           { return 0; }

	Thread() {}
	~Thread() {}

private:
	void operator=(const Thread&);
	Thread(const Thread&);
};

inline void AssertST() {}

class Semaphore : NoCopy {
private:
	int count;
	mutable std::mutex mtx;
	std::condition_variable cv;

public:
	bool Wait(int timeout_ms = -1) {
		std::unique_lock<std::mutex> lock(mtx);
		if (count > 0) {
			count--;
			return true;
		}
		return false; // In single-threaded model, this is essentially a no-op
	}
	
	void Release() {
		std::lock_guard<std::mutex> lock(mtx);
		count++;
		cv.notify_one();
	}
	
	void Release(int n) {
		std::lock_guard<std::mutex> lock(mtx);
		count += n;
		cv.notify_all();
	}

	Semaphore() : count(0) {}
	~Semaphore() {}
};

struct MtInspector;

class Mutex : NoCopy {
protected:
	// In single-threaded mode, the mutex does nothing
	MtInspector        *mti;

	Mutex(int)         {}

	friend class ConditionVariable;

public:
	bool  TryEnter() { return true; }
	void  Leave()    { }

	void  Enter()    { }

	Mutex() : mti(nullptr) { }
	~Mutex()                     { }

	class Lock;
};

class RWMutex : NoCopy {
	// In single-threaded mode, the mutex does nothing
public:
	void EnterWrite() { }
	void LeaveWrite() { }

	void EnterRead()  { }
	void LeaveRead()  { }

	RWMutex() {}
	~RWMutex() {}
};

class ConditionVariable {
	// In single-threaded mode, condition variable does nothing
public:
	bool Wait(Mutex& m, int timeout_ms = -1) { 
		return true; 
	}
	void Signal()     { }
	void Broadcast()  { }
	
	ConditionVariable() {}
	~ConditionVariable() {}
};

typedef bool OnceFlag;

#define ONCELOCK_(o_b_) \
for(static int o_b_ = 0; o_b_ < 1; o_b_++)

#define ONCELOCK \
for(static int ___ = 0; ___ < 1; ___++)

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
	
	void Initialize() { 
		primitive = new(buffer) Primitive; 
	}

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
	mutable bool               dirty;

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

	LazyUpdate() : dirty(true) {}
};

inline bool IsMainThread() { return true; }

struct SpinLock : Moveable<SpinLock> {
	// In single-threaded mode, no actual locking needed
	bool locked = false;

	bool TryEnter() { 
		if (locked) return false;
		locked = true;
		return true;
	}
	void Leave()    { locked = false; }
	
	void Enter()    { locked = true; }
	
	void Wait() {
		// In single-threaded mode, there's nothing to wait for
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

#define INTERLOCKED

struct H_l_ : Mutex::Lock {
	bool b;
	H_l_(Mutex& cs) : Mutex::Lock(cs) { b = true; }
};

#define INTERLOCKED_(cs)

#ifdef DEPRECATED
typedef Mutex CriticalSection;
typedef StaticMutex StaticCriticalSection;
#endif

#define auxthread_t void *
#define auxthread__

bool StartAuxThread(auxthread_t (auxthread__ *fn)(void *ptr), void *ptr);

#endif

#endif

#endif