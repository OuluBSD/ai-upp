#include "EonApiEditor.h"

#ifdef flagMAIN
using namespace Upp;

CONSOLE_APP_MAIN {
	// Get root path from cmdline or use default
	String root_path;
	if (CommandLine().GetCount() > 0) {
		root_path = CommandLine()[0];
	}
	else {
		#ifdef flagWIN32
		root_path = "E:\\active\\sblo\\Dev\\ai-upp";
		if (!DirectoryExists(root_path))
			root_path = GetHomeDirectory() + "\\Dev\\ai-upp";
		#else
		root_path = AppendFileName(GetHomeDirectory(), "Dev/ai-upp");
		#endif

		if (!DirectoryExists(root_path)) {
			LOG("ERROR: Root directory not found. Please specify path as first argument.");
			LOG("  Usage: EonApiEditor <root_path>");
			LOG("  Example: EonApiEditor ~/Dev/ai-upp");
			SetExitCode(1);
			return;
		}
	}

	LOG("Using root path: " << root_path);

	InterfaceBuilder ib;
	ib.SetRootPath(root_path);

	ib.AddAudio();
	ib.AddHal();
	ib.AddScreen();
	ib.AddVolumetric();
	ib.AddCamera();
	ib.AddHolographic();
	ib.AddSynth();
	ib.AddEffect();
	ib.AddMidiHw();
	ib.AddAudioFileOut();

	ib.Headers();

	ib.Generate(true);

	LOG("Generation complete!");
}


#endif
