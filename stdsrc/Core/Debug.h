#pragma once
#ifndef _Core_Debug_h_
#define _Core_Debug_h_

#include "Core.h"
#include <cstdarg>
#include <cstdio>
#include <map>
#include <vector>
#include <mutex>

namespace Upp {

// Logging functions
void __LOGF__(const char *fmt, ...);
String GetTypeName(const char *s);

#ifdef _DEBUG
#define  LOG(x)    do { Upp::__LOGF__ x; } while(false)
#define  DLOG(x)   LOG(x)
#else
#define  LOG(x)    do {} while(false)
#define  DLOG(x)   do {} while(false)
#endif

#define  RLOG(x)   LOG(x) // Raw log (unconditional)

// Timing functionality
class TimingInspector {
	static bool       active;
	Mutex             mutex;
	const char       *name;
	dword             all_count;
	dword             call_count;
	dword             total_time;
	dword             min_time;
	dword             max_time;
	int               max_nesting;

public:
	void  Add(dword time, int nesting);
	String Dump();
	
	static void  Clear()                      { active = false; }
	static void  Enable()                     { active = true; }
	static bool  IsEnabled()                  { return active; }

	TimingInspector(const char *name);
	~TimingInspector();
	
	friend class TimingInspectorRoutine;
};

class TimingInspectorRoutine {
	static thread_local int nesting;
	TimingInspector& m;
	dword            tm;
	
public:
	TimingInspectorRoutine(TimingInspector& m);
	~TimingInspectorRoutine();
	
	static int GetNesting() { return nesting; }
};

#define  TIMING(x)       Upp::TimingInspector __timing__(x)
#define  LTIMING(x)      // In production builds, this is empty

#define  TIME(x)         for(Upp::TimingInspector __timing__(x);; __timing__.Add(Upp::tmGetTime(), Upp::TimingInspectorRoutine::nesting), Upp::DoTimeLoopBreak())

inline bool& DoTimeLoopBreak() { static bool b; return b = true; }

// Hit counting functionality
class HitCountInspector {
	String  name;
	mutable int64 hitcount;
	
public:
	void  operator++() const                    { hitcount++; }
	void  operator++(int) const                 { hitcount++; }
	
	~HitCountInspector();
	
	HitCountInspector(const char *name);
};

#define  HITALOOP(x)     for(Upp::HitCountInspector x(#x); Upp::DoTimeLoopBreak(); )

// Debug assertion functionality
#ifdef _DEBUG

#define  ASSERT(x)               do { if(!(x)) { Upp::Panic(#x); } } while(0)
#define  ASSERT_(x, y)           do { if(!(x)) { Upp::Panic(y); } } while(0)
#define  ASSERTDBG(x)            ASSERT(x)

#else

#define  ASSERT(x)               do {} while(0)
#define  ASSERT_(x, y)           do {} while(0)
#define  ASSERTDBG(x)            do {} while(0)

#endif

// Panic function
void Panic(const char *text);

// Hex dump functionality
void HexDump(Stream& s, const void *ptr, int size, int maxsize = 10000);
void HexDumpData(Stream& s, const void *ptr, int size, bool adr = true, int maxsize = 10000);
void LogHex(const String& s);
void LogHex(const WString& s);
void LogHex(uint64 i);
void LogHex(void *p);

// Memory debugging
void SetMagic(byte *t, int count);
void CheckMagic(byte *t, int count);

// Etalon testing
void CheckLogEtalon(const char *etalon_path);
void CheckLogEtalon();

// C++ name demangling
String CppDemangle(const char* name);

// Crash dump handling (Windows)
#if defined(PLATFORM_WIN32) && !defined(PLATFORM_WINCE)
void InstallCrashDump(const char *info = NULL);
void SetCrashFileName(const char *cfile);
#endif

}

#endif