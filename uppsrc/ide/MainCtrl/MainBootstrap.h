#ifndef _ide_MainCtrl_MainBootstrap_h_
#define _ide_MainCtrl_MainBootstrap_h_

struct Ide;

void StartEditorMode(const Vector<String>& args, Ide& ide, bool& clset);

#ifdef DYNAMIC_LIBCLANG
bool TryLoadLibClang();
#endif

#ifdef PLATFORM_WIN32
bool HandleUwpPrelaunchCommandLine(const Vector<String>& arg, Ide& ide, bool clset,
                                   bool uwp_test_mode, bool uwp_prelaunch_mode);
#endif

#endif
