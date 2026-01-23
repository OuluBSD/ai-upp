#ifndef _ide_UwpUtils_h_
#define _ide_UwpUtils_h_

#include <ide/Core/Core.h>

#ifdef PLATFORM_WIN32

bool IsUwpApp(const String& path);
String GetUwpPackageName(const String& folder);
String GetUwpPackageFamilyName(const String& pkgName);
String GetUwpPackageFullName(const String& pkgName);
bool LaunchUwpApp(const String& path, const String& args, bool debug, DWORD& pid);
bool EnableUwpPrelaunch(const String& path, const String& debuggerPath = String());
bool LaunchUwpAppDirect(const String& path, DWORD& pid);
// For debugging: creates process suspended, returns handles. Caller must ResumeThread and close handles.
bool LaunchUwpAppForDebug(const String& path, DWORD& pid, HANDLE& hProcess, HANDLE& hThread);
void StopUwpDebug(const String& path);

#endif

#endif
