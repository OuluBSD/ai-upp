#ifndef _ide_UwpUtils_h_
#define _ide_UwpUtils_h_

#include <ide/Core/Core.h>

#ifdef PLATFORM_WIN32

bool IsUwpApp(const String& path);
String GetUwpPackageName(const String& folder);
String GetUwpPackageFamilyName(const String& pkgName);
bool LaunchUwpApp(const String& path, const String& args, bool debug, DWORD& pid);
void StopUwpDebug(const String& path);

#endif

#endif
