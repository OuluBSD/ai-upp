#include "MainCtrl.h"
#include "UwpUtils.h"

#ifdef flagGUI

#define FUNCTION_NAME UPP_FUNCTION_NAME << "(): "

#ifdef DYNAMIC_LIBCLANG
bool TryLoadLibClang()
{
#ifdef PLATFORM_MACOS
	if(LoadLibClang(TrimBoth(Sys("xcode-select -p")) + "/usr/lib"))
		return true;
	if(LoadLibClang("/Library/Developer/CommandLineTools/usr/lib"))
		return true;
	if(LoadLibClang("/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib"))
		return true;
#endif
#ifdef FLATPAK
	if(LoadLibClang("/app/lib"))
		return true;
#endif
	// in Mint 21.1, clang installed is 14 but llvm defaults to 15
	for(String s : Split(Sys("clang --version"), [](int c)->int { return !IsDigit(c); })) {
		int n = Atoi(s);
		if(n >= 5 && n < 5000) {
			if(LoadLibClang("/usr/lib/llvm-" + AsString(n) + "/lib"))
				return true;
			if(LoadLibClang("/usr/lib/llvm/" + AsString(n) + "/lib64"))
				return true;
			if(LoadLibClang("/usr/lib/llvm/" + AsString(n) + "/lib32"))
				return true;
			break;
		}
	}

	String libdir = TrimBoth(Sys("llvm-config --libdir"));
	int q = FindIndex(CommandLine(), "--clangdir");
	if(q >= 0 && q + 1 < CommandLine().GetCount()) {
		libdir = CommandLine()[q + 1];
		CommandLineRemove(q, 2);
	}
	if(LoadLibClang(libdir))
		return true;
	if(LoadLibClang("/usr/lib64"))
		return true;
	if(LoadLibClang("/usr/lib"))
		return true;
	for(int i = 200; i >= 10; i--) {
		if(LoadLibClang("/usr/lib/llvm-" + AsString(i) + "/lib"))
			return true;
		if(LoadLibClang("/usr/lib/llvm/" + AsString(i) + "/lib64"))
			return true;
		if(LoadLibClang("/usr/lib/llvm/" + AsString(i) + "/lib32"))
			return true;
	}
	return false;
}
#endif

void StartEditorMode(const Vector<String>& args, Ide& ide, bool& clset)
{
	if(args.IsEmpty() || clset)
		return;

	bool editor = false;
	for(int i = 0; i < args.GetCount(); i++) {
		if(*args[i] != '-') {
			String file_path = NormalizePath(args[i]);

			Logd() << FUNCTION_NAME << "Opening file \"" << file_path << "\".";

			ide.EditFile(file_path);
			ide.FileSelected();
			editor = true;
		}
	}

	if(editor) {
		clset = true;
		Vector<String> dir = Split(LoadFile(GetHomeDirFile("usc.path")), ';');
		for(int i = 0; i < dir.GetCount(); i++)
			ide.UscProcessDirDeep(dir[i]);
		ide.EditorMode();
	}
}

#undef FUNCTION_NAME

#endif

#ifdef PLATFORM_WIN32

bool HandleUwpPrelaunchCommandLine(const Vector<String>& arg, Ide& ide, bool clset,
                                   bool uwp_test_mode, bool uwp_prelaunch_mode)
{
	if(!(uwp_test_mode || uwp_prelaunch_mode))
		return false;

	// UWP test/prelaunch mode: build with UWP method, enable debug, launch, and exit
	// Write output to file since GUI apps don't have console
	String logPath = GetHomeDirFile(uwp_prelaunch_mode ? "uwp_prelaunch.log" : "uwp_test.log");
	FileOut log(logPath);

	auto Log = [&](const char* s) {
		log << s;
		log.Flush();
	};

	Log(uwp_prelaunch_mode ? "UWP Prelaunch Mode\n" : "UWP Test Mode\n");
	Log("clset: "); Log(clset ? "true" : "false"); Log("\n");
	Log("Assembly: "); Log(GetVarsName()); Log("\n");
	Log("Package: "); Log(ide.GetMain()); Log("\n");

	if(!clset) {
		Log("ERROR: Assembly/package not set. Make sure .var files exist in IDE directory.\n");
		SetExitCode(1);
		return true;
	}

	ide.SetMethod("UWP");
	ide.mainconfigparam = "UWP DX12 UWP_INTERNAL DEBUG DEBUG_FULL";
	ide.targetmode = 0; // debug

	Log("Building...\n");
	SetTheIde(&ide);

	// Capture all PutConsole output to the log file
	ide.console_capture = &log;

	if(ide.Build()) {
		Log("Build succeeded.\n");
		Log("Target: "); Log(ide.target); Log("\n");

		if(uwp_prelaunch_mode) {
			// Prelaunch mode: enable debugging, launch, and prepare for debugger attach
			Log("Enabling UWP prelaunch debugging...\n");
			if(EnableUwpPrelaunch(ide.target, String())) {
				Log("Prelaunch debugging enabled.\n");

				// Launch the app with debug flag
				DWORD pid = 0;
				Log("Launching UWP app with debug...\n");
				if(LaunchUwpApp(ide.target, String(), true, pid)) {
					Log("UWP app launched. PID: "); Log(AsString(pid)); Log("\n");
					if(pid > 0) {
						Log("Process is ready for debugger attach.\n");
						// TODO: Could auto-attach debugger here
					}
				}
				else {
					Log("UWP app launch failed.\n");
					SetExitCode(1);
				}
			}
			else {
				Log("Failed to enable prelaunch debugging.\n");
				SetExitCode(1);
			}
		}
		else {
			// Normal test mode
			Log("Attempting to launch UWP app...\n");
			ide.ExecuteBinary();
			Log("ExecuteBinary returned.\n");
		}
	}
	else {
		Log("Build failed.\n");
		SetExitCode(1);
	}

	ide.console_capture = nullptr;
	Log("Done.\n");
	log.Close();
	return true;
}

#endif
