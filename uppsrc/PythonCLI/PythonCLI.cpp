#include "PythonCLI.h"
#include <termios.h>
#include <unistd.h>

NAMESPACE_UPP

String PythonCLI::ReadLine()
{
	bool is_interactive = isatty(STDIN_FILENO);
	if (!is_interactive) {
		String r;
		for(;;)	{
			int c = getchar();
			if(c < 0) return r.GetCount() ? r : String::GetVoid();
			if(c == '\n') return r;
			r.Cat(c);
		}
	}

	struct termios orig_termios;
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
		String r;
		for(;;)	{
			int c = getchar();
			if(c < 0) return r.GetCount() ? r : String::GetVoid();
			if(c == '\n') return r;
			r.Cat(c);
		}
	}

	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	String line;
	int cursor_pos = 0;
	int h_index = history.GetCount();
	String saved_line;

	auto Redraw = [&]() {
		printf("\r\x1b[K>>> %s", ~line);
		if (cursor_pos < line.GetCount())
			printf("\x1b[%dD", line.GetCount() - cursor_pos);
		fflush(stdout);
	};

	Redraw();

	for (;;)	{
		char c;
		if (read(STDIN_FILENO, &c, 1) <= 0) break;

		if (c == '\n' || c == '\r') {
			printf("\n");
			break;
		}
		else if (c == 3) { // Ctrl+C
			printf("\nKeyboardInterrupt\n");
			line = "";
			cursor_pos = 0;
			h_index = history.GetCount();
			Redraw();
		}
		else if (c == 4) { // Ctrl+D (EOF or Delete)
			if (line.IsEmpty() && cursor_pos == 0) {
				line = String::GetVoid();
				break;
			}
			if (cursor_pos < line.GetCount()) {
				line.Remove(cursor_pos, 1);
				Redraw();
			}
		}
		else if (c == 127 || c == 8) { // Backspace
			if (cursor_pos > 0) {
				line.Remove(cursor_pos - 1, 1);
				cursor_pos--;
				Redraw();
			}
		}
		else if (c == 1) { // Ctrl+A (Home)
			cursor_pos = 0;
			Redraw();
		}
		else if (c == 5) { // Ctrl+E (End)
			cursor_pos = line.GetCount();
			Redraw();
		}
		else if (c == 11) { // Ctrl+K (Kill after)
			line = line.Mid(0, cursor_pos);
			Redraw();
		}
		else if (c == 21) { // Ctrl+U (Kill before)
			line = line.Mid(cursor_pos);
			cursor_pos = 0;
			Redraw();
		}
		else if (c == 27) { // Escape
			char seq[3];
			if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
				if (seq[0] == '[') {
					if (seq[1] >= '0' && seq[1] <= '9') {
						if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') {
							switch (seq[1]) {
							case '1': case '7': // Home
								cursor_pos = 0; break;
							case '4': case '8': // End
								cursor_pos = line.GetCount(); break;
							case '3': // Delete
								if (cursor_pos < line.GetCount()) line.Remove(cursor_pos, 1);
								break;
							}
						}
					}
					else {
						switch (seq[1]) {
						case 'A': // Up
							if (h_index > 0) {
								if (h_index == history.GetCount()) saved_line = line;
								h_index--;
								line = history[h_index];
								cursor_pos = line.GetCount();
							}
							break;
						case 'B': // Down
							if (h_index < history.GetCount()) {
								h_index++;
								if (h_index == history.GetCount()) line = saved_line;
								else line = history[h_index];
								cursor_pos = line.GetCount();
							}
							break;
						case 'C': // Right
							if (cursor_pos < line.GetCount()) cursor_pos++;
							break;
						case 'D': // Left
							if (cursor_pos > 0) cursor_pos--;
							break;
						case 'H': // Home
							cursor_pos = 0; break;
						case 'F': // End
							cursor_pos = line.GetCount(); break;
						}
					}
				}
			}
			Redraw();
		}
		else if ((byte)c >= 32) {
			line.Insert(cursor_pos, (char)c);
			cursor_pos++;
			Redraw();
		}
	}

		tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
		return line;
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

	bool PythonCLI::ProcessInput(const String& input, bool is_interactive)
	{
		if(input == "quit()" || input == "exit()" || input == "quit" || input == "exit") {
			return false; // Stop processing
		}

		if(input.StartsWith("help") || input.StartsWith("copyright") ||
		   input.StartsWith("credits") || input.StartsWith("license")) {
			if(input.StartsWith("help")) {
				Cout() << "This is a simplified Uppy interpreter based on U++ ByteVM\n";
			} else if(input.StartsWith("copyright")) {
				Cout() << "Copyright (c) 2026 U++ Community, Seppo Pakonen\n";
			} else if(input.StartsWith("credits")) {
				Cout() << "Uppy interpreter developed using U++ ByteVM technology. Credits: Seppo Pakonen\n";
			} else if(input.StartsWith("license")) {
				Cout() << "BSD-style license - see LICENSE.md in U++ distribution\n";
			}
			return true; // Continue processing
		}

		try {
			// Process the input as Python code
			Tokenizer tk;
			tk.SkipComments();
			tk.SkipPythonComments();
			if(!tk.Process(input, "<stdin>")) {
				return true; 
			}
			tk.NewlineToEndStatement();
			tk.CombineTokens();

			PyCompiler compiler(tk.GetTokens());
			Vector<PyIR> ir;
			try {
				compiler.Compile(ir);
			} catch (Exc& e) {
				return true; 
			}

			vm.SetIR(ir);
			try {
				PyValue res = vm.Run();
				if (is_interactive && !res.IsNone()) {
					Cout() << res.Repr() << "\n";
				}
			} catch (Exc& e) {
				if (e.StartsWith("EXIT:")) {
					exit_code = StrInt(e.Mid(5));
					return false;
				}
				if (is_interactive) {
					Cout() << e << "\n";
				} else {
					Cout() << "Runtime error: " << e << "\n";
				}
			}
		} catch (...) {
			Cout() << "Internal error occurred while processing input\n";
			return true; // Continue processing
		}

		return true; // Continue processing
	}

	int PythonCLI::RunInteractive()
	{
		bool is_interactive = isatty(STDIN_FILENO);

		if (is_interactive) {
			PrintBanner();
		}

		String input;

		while(true) {
			input = ReadLine();

			if (input.IsVoid()) {
				if (is_interactive) printf("\n");
				break; 
			}

			if (input.IsEmpty()) {
				continue;
			}

			if (is_interactive && !input.IsEmpty() && (history.IsEmpty() || history.Top() != input)) {
				history.Add(input);
			}

			if (!ProcessInput(input, true)) {
				break; 
			}

			if (!is_interactive) {
				if (feof(stdin)) {
					break;
				}
			}
		}
		return exit_code;
	}

	int PythonCLI::RunScript(const String& filename)
	{
		String content = LoadFile(filename);

		if(content.IsEmpty()) {
			Cout() << "Error: Could not read file '" << filename << "'\n";
			return 1;
		}

		try {
			Tokenizer tk;
			tk.SkipComments();
			tk.SkipPythonComments();
			if(!tk.Process(content, filename)) {
				return 1;
			}
			tk.NewlineToEndStatement();
			tk.CombineTokens();

			PyCompiler compiler(tk.GetTokens());
			Vector<PyIR> ir;
			try {
				compiler.Compile(ir);
			} catch (Exc& e) {
				return 1;
			}

			vm.SetIR(ir);
			try {
				vm.Run();
			} catch (Exc& e) {
				if (e.StartsWith("EXIT:")) {
					exit_code = StrInt(e.Mid(5));
					return exit_code;
				}
				Cout() << "Runtime error in file '" << filename << "': " << e << "\n";
				exit_code = 1;
				return 1;
			}
		} catch (...) {
			Cout() << "Internal error occurred while processing file '" << filename << "'\n";
			exit_code = 1;
			return 1;
		}
		return exit_code;
	}

	int PythonCLI::Run()
	{
		const auto& cmds = CommandLine();
		int start_idx = 0;
		while (cmds.GetCount() > start_idx) {
			if (cmds[start_idx] == "--threading") {
				if (cmds.GetCount() > start_idx + 1) {
					if (cmds[start_idx + 1] == "scheduled") PyScheduler::Get().SetMode(PYTHREAD_SCHEDULED);
					else PyScheduler::Get().SetMode(PYTHREAD_NATIVE);
					start_idx += 2;
				} else start_idx++;
			}
			else if (cmds[start_idx] == "--sandbox") {
				PolicyKit::Get().Set(PYPERM_FILE_READ, false);
				PolicyKit::Get().Set(PYPERM_FILE_WRITE, false);
				PolicyKit::Get().Set(PYPERM_PROCESS_EXEC, false);
				start_idx++;
			}
			else break;
		}

		int res = 0;
		if (cmds.GetCount() > start_idx)
			res = RunScript(cmds[start_idx]);
		else
			res = RunInteractive();
		
		PyScheduler::Get().Run();
		return res;
	}

END_UPP_NAMESPACE