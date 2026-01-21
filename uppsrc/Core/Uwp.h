#ifndef _Core_Uwp_h_
#define _Core_Uwp_h_

#ifdef flagUWP

#include <winapifamily.h>
#include <windows.h>
#include <shlobj.h>

#undef GetVersion
inline unsigned long GetVersion() { return 0x0A000000; }

#undef GetVersionEx
inline int GetVersionEx(void*) { return 0; }

#undef timeGetTime
inline unsigned long timeGetTime() { return GetTickCount(); }

#undef GetUserDefaultLCID
inline unsigned long GetUserDefaultLCID() { return 0x0409; }

#undef ShellExecuteW
inline void* ShellExecuteW(void*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, int) { return (void*)33; }

#undef CommandLineToArgvW
inline wchar_t** CommandLineToArgvW(const wchar_t*, int*) { return (wchar_t**)0; }

#undef GetActiveWindow
inline void* GetActiveWindow() { return (void*)0; }

#undef MessageBox
inline int MessageBox(void*, const char*, const char*, unsigned int) { return 0; }

#undef MessageBeep
inline int MessageBeep(unsigned int) { return 1; }

#undef GetUserNameW
inline int GetUserNameW(wchar_t*, unsigned long*) { return 0; }

#undef GetUserNameA
inline int GetUserNameA(char*, unsigned long*) { return 0; }

#undef SHGetFolderPathW
inline long SHGetFolderPathW(void*, int, void*, unsigned long, wchar_t*) { return -1; }

#undef IsBadReadPtr
inline int IsBadReadPtr(const void*, uintptr_t) { return 0; }

#undef SetLocalTime
inline int SetLocalTime(const void*) { return 0; }

#undef GetFileSize
inline unsigned long GetFileSize(void*, unsigned long*) { return 0; }

#undef GetFileSizeEx
inline int GetFileSizeEx(void*, void*) { return 0; }

#undef GetWindowsDirectoryW
inline unsigned int GetWindowsDirectoryW(wchar_t*, unsigned int) { return 0; }

#undef GetSystemDirectoryW
inline unsigned int GetSystemDirectoryW(wchar_t*, unsigned int) { return 0; }

#undef GetModuleHandle
inline void* GetModuleHandle(const char*) { return (void*)0; }

#undef LoadLibrary
inline void* LoadLibrary(const char*) { return (void*)0; }

#undef RegOpenKeyEx
inline long RegOpenKeyEx(void*, const char*, unsigned long, unsigned long, void**) { return 1; }

#undef RegQueryValueEx
inline long RegQueryValueEx(void*, const char*, unsigned long*, unsigned long*, unsigned char*, unsigned long*) { return 1; }

#undef RegCloseKey
inline long RegCloseKey(void*) { return 0; }

#undef RegCreateKeyEx
inline long RegCreateKeyEx(void*, const char*, unsigned long, char*, unsigned long, unsigned long, void*, void**, unsigned long*) { return 1; }

#undef RegSetValueEx
inline long RegSetValueEx(void*, const char*, unsigned long, unsigned long, const unsigned char*, unsigned long) { return 1; }

#undef RegEnumKeyEx
inline long RegEnumKeyEx(void*, unsigned long, char*, unsigned long*, unsigned long*, char*, unsigned long*, void*) { return 1; }

#undef RegDeleteKey
inline long RegDeleteKey(void*, const char*) { return 1; }

#undef CreateFileW
inline void* CreateFileW(const wchar_t*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*) { return (void*)-1; }

#undef MoveFileW
inline int MoveFileW(const wchar_t*, const wchar_t*) { return 0; }

#endif

#endif
