#include "DebuggersCore.h"

namespace Upp {

DbgBacktrace GetGdbBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript)
{
	DbgBacktrace bt;
	
	Vector<String> gdb_args;
	gdb_args.Add("-batch");
	
	// Set working directory inside GDB if specified
	if (!cwd.IsEmpty()) {
		gdb_args.Add("-ex");
		gdb_args.Add("cd " + cwd);
	}
	
	gdb_args.Add("-ex");
	gdb_args.Add("run");
	gdb_args.Add("-ex");
	gdb_args.Add("bt");
	gdb_args.Add("--args");
	gdb_args.Add(exe);
	for (int i = 0; i < args.GetCount(); i++) {
		gdb_args.Add(args[i]);
	}
	
	String env_block;
	for (int i = 0; i < env.GetCount(); i++) {
		env_block.Cat(env.GetKey(i));
		env_block.Cat("=");
		env_block.Cat(env[i]);
		env_block.Cat('\0');
	}
	if (!env_block.IsEmpty()) {
		env_block.Cat('\0');
	}
	
	LocalProcess lp;
	if (!lp.Start("gdb", gdb_args, env_block.IsEmpty() ? NULL : ~env_block, cwd.IsEmpty() ? NULL : ~cwd)) {
		bt.error = "Failed to launch GDB";
		return bt;
	}
	
	String out;
	while (lp.IsRunning()) {
		String chunk;
		if (lp.Read(chunk)) {
			out.Cat(chunk);
			transcript.Cat(chunk);
		}
		Sleep(10);
	}
	String chunk;
	while (lp.Read(chunk)) {
		out.Cat(chunk);
		transcript.Cat(chunk);
	}
	
	Vector<String> lines = Split(out, '\n');
	for (int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if (line.StartsWith("#")) {
			// Expected format: #0  0x000055555556fa7d in CrashSmokeCrash () at /home/sblo/Dev/ai-upp/upptst/DbgCrashSmoke/main.cpp:16
			// or: #0  CrashSmokeCrash () at ...
			int space1 = line.Find(' ');
			if (space1 < 0) continue;
			
			String num_str = line.Mid(1, space1 - 1);
			String remainder = TrimBoth(line.Mid(space1));
			
			DbgFrame frame;
			if (remainder.StartsWith("0x")) {
				int in_pos = remainder.Find(" in ");
				if (in_pos >= 0) {
					frame.address = remainder.Left(in_pos);
					remainder = TrimBoth(remainder.Mid(in_pos + 4));
				} else {
					int space2 = remainder.Find(' ');
					if (space2 >= 0) {
						frame.address = remainder.Left(space2);
						remainder = TrimBoth(remainder.Mid(space2));
					}
				}
			}
			
			int paren_open = remainder.Find('(');
			if (paren_open >= 0) {
				frame.function = TrimBoth(remainder.Left(paren_open));
				int depth = 0;
				int paren_close = -1;
				for (int j = paren_open; j < remainder.GetCount(); j++) {
					if (remainder[j] == '(') depth++;
					else if (remainder[j] == ')') {
						depth--;
						if (depth == 0) {
							paren_close = j;
							break;
						}
					}
				}
				if (paren_close >= 0) {
					remainder = TrimBoth(remainder.Mid(paren_close + 1));
				}
			} else {
				// No parens, check for "at" or "from"
				int at_pos = remainder.Find(" at ");
				int from_pos = remainder.Find(" from ");
				if (at_pos >= 0) {
					frame.function = TrimBoth(remainder.Left(at_pos));
					remainder = remainder.Mid(at_pos + 1);
				} else if (from_pos >= 0) {
					frame.function = TrimBoth(remainder.Left(from_pos));
					remainder = remainder.Mid(from_pos + 1);
				} else {
					frame.function = remainder;
					remainder = "";
				}
			}
			
			if (remainder.StartsWith("at ")) {
				String file_line = TrimBoth(remainder.Mid(3));
				int colon = file_line.ReverseFind(':');
				if (colon >= 0) {
					frame.file = TrimBoth(file_line.Left(colon));
					frame.line = atoi(~file_line.Mid(colon + 1));
				} else {
					frame.file = file_line;
				}
			} else if (remainder.StartsWith("from ")) {
				frame.file = TrimBoth(remainder.Mid(5));
			}
			
			bt.frames.Add(frame);
		}
	}
	
	if (bt.frames.IsEmpty()) {
		bt.error = "No stack frames found";
	}
	
	return bt;
}

}
