#include "PythonCLI.h"
#include <cstdio>
#include <unistd.h>

using namespace Upp;

static String ReadStdInLine()
{
	String r;
	for(;;) {
		int c = getchar();
		if(c < 0) // EOF
			return r.GetCount() ? r : String::GetVoid();
		if(c == '\n') // End of line
			return r;
		r.Cat(c);
	}
}

void PythonCLI::PrintBanner()
{
	Cout() << "Uppy 0.1v (based on U++ Python implementation)\n";
	Cout() << "Type \"help\", \"copyright\", \"credits\" or \"license\" for more information.\n";
}

void PythonCLI::PrintPrompt()
{
	Cout() << ">>> ";
}

bool PythonCLI::ProcessInput(const String& input)
{
	if(input == "quit()" || input == "exit()" || input == "quit" || input == "exit") {
		return false; // Stop processing
	}

	if(input.StartsWith("help") || input.StartsWith("copyright") ||
	   input.StartsWith("credits") || input.StartsWith("license")) {
		if(input.StartsWith("help")) {
			Cout() << "This is a simplified Uppy interpreter based on U++ ByteVM\n";
		} else if(input.StartsWith("copyright")) {
			Cout() << "Copyright (c) 2026 U++ Community\n";
		} else if(input.StartsWith("credits")) {
			Cout() << "Uppy interpreter developed using U++ ByteVM technology\n";
		} else if(input.StartsWith("license")) {
			Cout() << "BSD-style license - see LICENSE.md in U++ distribution\n";
		}
		return true; // Continue processing
	}

	try {
		// Process the input as Python code
		Tokenizer tk;
		tk.SkipPythonComments();
		if(!tk.Process(input, "<stdin>")) {
			Cout() << "SyntaxError: invalid syntax\n";
			return true; // Continue processing
		}
		tk.CombineTokens();
		tk.NewlineToEndStatement();

		PyCompiler compiler(tk.GetTokens());
		Vector<PyIR> ir;
		try {
			compiler.Compile(ir);
		} catch (Exc& e) {
			Cout() << "Compilation error: " << e << "\n";
			return true; // Continue processing
		}

		PyVM vm;
		vm.SetIR(ir);
		try {
			vm.Run();
		} catch (Exc& e) {
			Cout() << "Runtime error: " << e << "\n";
		}
	} catch (...) {
		Cout() << "Internal error occurred while processing input\n";
		return true; // Continue processing
	}

	return true; // Continue processing
}

void PythonCLI::RunInteractive()
{
	// Check if stdin is connected to a terminal (interactive mode)
	// or if input is being piped redirected (non-interactive mode)
	bool is_interactive = isatty(STDIN_FILENO);

	if (is_interactive) {
		PrintBanner();
	}

	String input;

	while(true) {
		if (is_interactive) {
			PrintPrompt();
		}

		// Using our custom function to read a single line from stdin
		input = ReadStdInLine();

		// If we reach EOF
		if (input.IsEmpty() && feof(stdin)) {
			break; // Exit the loop
		}

		// Process the input
		if (!ProcessInput(input)) {
			break; // User wants to quit
		}

		// If not interactive (piped input), don't continue the loop after EOF
		if (!is_interactive) {
			if (feof(stdin)) {
				break;
			}
		}
	}
}

void PythonCLI::RunScript(const String& filename)
{
	String content = LoadFile(filename);

	if(content.IsEmpty()) {
		Cout() << "Error: Could not read file '" << filename << "'\n";
		return;
	}

	try {
		// Process the file content using the same logic as ProcessInput but for files
		Tokenizer tk;
		tk.SkipPythonComments();
		if(!tk.Process(content, filename)) {
			Cout() << "SyntaxError: invalid syntax in file '" << filename << "'\n";
			return;
		}
		tk.CombineTokens();
		tk.NewlineToEndStatement();

		PyCompiler compiler(tk.GetTokens());
		Vector<PyIR> ir;
		try {
			compiler.Compile(ir);
		} catch (Exc& e) {
			Cout() << "Compilation error in file '" << filename << "': " << e << "\n";
			return;
		}

		PyVM vm;
		vm.SetIR(ir);
		try {
			vm.Run();
		} catch (Exc& e) {
			Cout() << "Runtime error in file '" << filename << "': " << e << "\n";
		}
	} catch (...) {
		Cout() << "Internal error occurred while processing file '" << filename << "'\n";
	}
}

void PythonCLI::Run()
{
	RunInteractive();
}