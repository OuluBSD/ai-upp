#ifndef _ByteVM_PyScheduler_h_
#define _ByteVM_PyScheduler_h_

#include "PyVM.h"

NAMESPACE_UPP

enum PyThreadingMode {
	PYTHREAD_NATIVE,
	PYTHREAD_SCHEDULED
};

class PyScheduler {
	Array<PyVM> vms;
	Vector<Thread> native_threads;
	PyThreadingMode mode = PYTHREAD_NATIVE;
	Mutex mutex;
	Mutex gil;
	int   active_native_threads = 0;

public:
	static PyScheduler& Get() { static PyScheduler s; return s; }
	
	void SetMode(PyThreadingMode m) { mode = m; }
	PyThreadingMode GetMode() const { return mode; }
	
	void AddVM(PyVM& vm);
	void Run();
	
	// GIL operations
	void Lock();
	void Unlock();
	bool HasLock() const;
	
	// Threading module support
	PyValue CreateThread(PyValue func, Vector<PyValue>&& args);
	
	void NativeThreadStarted() { Mutex::Lock __(mutex); active_native_threads++; }
	void NativeThreadFinished() { Mutex::Lock __(mutex); active_native_threads--; }
};

END_UPP_NAMESPACE

#endif
