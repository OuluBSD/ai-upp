#include "DebuggersCore.h"

namespace Upp {

DbgBacktrace GetLldbBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript)
{
	DbgBacktrace bt;
	
	Vector<String> lldb_args;
	lldb_args.Add("--batch");
	
	// Set working directory inside LLDB if specified
	// LLDB uses "settings set target.run-args" or "process launch -w"
	// But in batch mode, we can just run -o "settings set target.run-args ..."
	// Actually, "process launch -w <cwd>" is best.
	// But wait! If we do "run" or "process launch", let's see.
	// By default, "-o run" will launch.
	// If we specify working directory, we can run:
	// -o "settings set target.process.working-dir <cwd>"
	if (!cwd.IsEmpty()) {
		lldb_args.Add("-o");
		lldb_args.Add("settings set target.process.working-dir " + cwd);
	}
	
	lldb_args.Add("-o");
	lldb_args.Add("run");
	lldb_args.Add("-o");
	lldb_args.Add("quit");
	lldb_args.Add("-k");
	lldb_args.Add("bt");
	lldb_args.Add("-k");
	lldb_args.Add("quit");
	lldb_args.Add("-f");
	lldb_args.Add(exe);
	lldb_args.Add("--");
	for (int i = 0; i < args.GetCount(); i++) {
		lldb_args.Add(args[i]);
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
	if (!lp.Start("lldb", lldb_args, env_block.IsEmpty() ? NULL : ~env_block, cwd.IsEmpty() ? NULL : ~cwd)) {
		bt.error = "Failed to launch LLDB";
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
		if (line.StartsWith("* ")) {
			line = TrimBoth(line.Mid(2));
		}
		if (line.StartsWith("frame #")) {
			int colon = line.Find(':');
			if (colon < 0) continue;
			
			String remainder = TrimBoth(line.Mid(colon + 1));
			DbgFrame frame;
			
			if (remainder.StartsWith("0x")) {
				int space = remainder.Find(' ');
				if (space >= 0) {
					frame.address = remainder.Left(space);
					remainder = TrimBoth(remainder.Mid(space));
				}
			}
			
			int at_pos = remainder.Find(" at ");
			String func_part = at_pos >= 0 ? TrimBoth(remainder.Left(at_pos)) : remainder;
			remainder = at_pos >= 0 ? TrimBoth(remainder.Mid(at_pos + 4)) : "";
			
			int backtick = func_part.Find('`');
			if (backtick >= 0) {
				frame.function = func_part.Mid(backtick + 1);
			} else {
				frame.function = func_part;
			}
			
			if (at_pos >= 0) {
				Vector<String> parts = Split(remainder, ':');
				if (parts.GetCount() >= 2) {
					frame.file = parts[0];
					frame.line = atoi(parts[1]);
				} else {
					frame.file = remainder;
				}
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
