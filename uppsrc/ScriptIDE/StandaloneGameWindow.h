#ifndef _ScriptIDE_StandaloneGameWindow_h_
#define _ScriptIDE_StandaloneGameWindow_h_

NAMESPACE_UPP

void OpenStandaloneGameWindow(const String& path, RunMode mode = RunMode::Run);
void RunStandaloneGameWindow(const String& path, RunMode mode = RunMode::Run);
int  GetOpenStandaloneGameWindowCount();
int  GetRunningStandaloneGameWindowCount();
void CloseAllStandaloneGameWindows();

END_UPP_NAMESPACE

#endif
