#ifndef _PythonCLI_PythonCLI_h_
#define _PythonCLI_PythonCLI_h_

#include <Core/Core.h>
#include <Core/TextParsing/TextParsing.h>
#include <ByteVM/ByteVM.h>

NAMESPACE_UPP

// PythonCLI package for Uppy interpreter

class PythonCLI {
public:
	void Run();
	void RunInteractive();
	void RunScript(const String& filename);

private:
	Vector<String> history;
	PyVM vm;

	void PrintBanner();
	void PrintPrompt();
	bool ProcessInput(const String& input, bool is_interactive);
	String ReadLine();
};

END_UPP_NAMESPACE

#endif
