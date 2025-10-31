#pragma once
#ifndef _Core_Win32Util_h_
#define _Core_Win32Util_h_

#ifdef PLATFORM_WIN32

#include <winreg.h>
#include <string>
#include "Core.h"

#ifdef PLATFORM_WINCE
inline bool IsWinNT()    { return false; }
inline bool IsWinXP()    { return false; }
inline bool IsWin2K()    { return false; }
inline bool IsWinVista() { return false; }
inline bool IsWin7()     { return false; }
#else
inline bool IsWinNT() { return GetVersion() < 0x80000000; }
bool IsWin2K();
bool IsWinXP();
bool IsWinVista();
bool IsWin7();
bool IsWin11();
#endif

HINSTANCE AppGetHandle();
void      AppSetHandle(HINSTANCE dll_instance);

String AsString(const wchar_t *buffer);
String AsString(const wchar_t *buffer, int count);
String AsString(const wchar_t *buffer, const wchar_t *end);

String GetWinRegString(const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE, dword wow = 0);
int    GetWinRegInt(const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE, dword wow = 0);
bool   SetWinRegString(const String& string, const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE, dword wow = 0);
bool   SetWinRegExpandString(const String& string, const char *value, const char *path, HKEY base_key, dword wow = 0);
bool   SetWinRegInt(int data, const char *value, const char *path, HKEY base_key = HKEY_LOCAL_MACHINE, dword wow = 0);
void   DeleteWinReg(const String& key, HKEY base = HKEY_LOCAL_MACHINE, dword wow = 0);

void  *GetDllFn(const char *dll, const char *fn);

template <class T>
void   DllFn(T& x, const char *dll, const char *fn)
{
	x = (T)GetDllFn(dll, fn);
}

bool Win32CreateProcess(const char *command, const char *envptr, STARTUPINFOW& si, PROCESS_INFORMATION& pi, const char *cd);

#ifndef PLATFORM_WINCE
String GetSystemDirectory();
String GetWindowsDirectory();
#endif
String GetModuleFileName(HINSTANCE instance = AppGetHandle());

#ifdef DEPRECATED
class SyncObject {
protected:
	HANDLE     handle;

public:
	bool       Wait(int time_ms);
	bool       Wait();

	HANDLE     GetHandle() const { return handle; }

	SyncObject();
	~SyncObject();
};

class Win32Event : public SyncObject {
public:
	void       Set();

	Win32Event();
};
#endif

#else // PLATFORM_WIN32

// Non-Windows implementations - provide minimal stubs for cross-platform compatibility
inline bool IsWinNT() { return false; }
inline bool IsWinXP() { return false; }
inline bool IsWin2K() { return false; }
inline bool IsWinVista() { return false; }
inline bool IsWin7() { return false; }
inline bool IsWin11() { return false; }

inline HINSTANCE AppGetHandle() { return nullptr; }
inline void      AppSetHandle(HINSTANCE dll_instance) {}

inline String AsString(const wchar_t *buffer) { return String(); }
inline String AsString(const wchar_t *buffer, int count) { return String(); }
inline String AsString(const wchar_t *buffer, const wchar_t *end) { return String(); }

inline String GetWinRegString(const char *value, const char *path, void* base_key = nullptr, dword wow = 0) { return String(); }
inline int    GetWinRegInt(const char *value, const char *path, void* base_key = nullptr, dword wow = 0) { return 0; }
inline bool   SetWinRegString(const String& string, const char *value, const char *path, void* base_key = nullptr, dword wow = 0) { return false; }
inline bool   SetWinRegExpandString(const String& string, const char *value, const char *path, void* base_key, dword wow = 0) { return false; }
inline bool   SetWinRegInt(int data, const char *value, const char *path, void* base_key = nullptr, dword wow = 0) { return false; }
inline void   DeleteWinReg(const String& key, void* base = nullptr, dword wow = 0) {}

inline void  *GetDllFn(const char *dll, const char *fn) { return nullptr; }

template <class T>
void   DllFn(T& x, const char *dll, const char *fn)
{
	x = (T)GetDllFn(dll, fn);
}

inline bool Win32CreateProcess(const char *command, const char *envptr, void* si, void* pi, const char *cd) { return false; }

inline String GetSystemDirectory() { return String(); }
inline String GetWindowsDirectory() { return String(); }
inline String GetModuleFileName(void* instance = nullptr) { return String(); }

#ifdef DEPRECATED
class SyncObject {
protected:
	void*     handle;

public:
	bool       Wait(int time_ms) { return false; }
	bool       Wait() { return false; }

	void*     GetHandle() const { return handle; }

	SyncObject() {}
	~SyncObject() {}
};

class Win32Event : public SyncObject {
public:
	void       Set() {}

	Win32Event() {}
};
#endif

#endif // PLATFORM_WIN32

#endif