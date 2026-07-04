#include "DebuggersCore.h"

namespace Upp {

DbgBacktrace GetJavaBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript)
{
	DbgBacktrace bt;
	
	Vector<String> jdb_args;
	jdb_args.Add(exe);
	for (int i = 0; i < args.GetCount(); i++) {
		jdb_args.Add(args[i]);
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
	if (!lp.Start("jdb", jdb_args, env_block.IsEmpty() ? NULL : ~env_block, cwd.IsEmpty() ? NULL : ~cwd)) {
		bt.error = "Failed to launch jdb";
		return bt;
	}
	
	String out;
	bool sent_run = false;
	bool sent_where = false;
	bool sent_quit = false;
	
	int timeout_ms = 10000; // 10 seconds total timeout
	int elapsed_ms = 0;
	int no_output_ms = 0;
	
	while (lp.IsRunning() && elapsed_ms < timeout_ms) {
		String chunk;
		bool read_any = false;
		if (lp.Read(chunk)) {
			if (!chunk.IsEmpty()) {
				out.Cat(chunk);
				transcript.Cat(chunk);
				read_any = true;
				no_output_ms = 0;
			}
		}
		
		if (!read_any) {
			no_output_ms += 20;
		}
		
		if (!sent_run && out.Find(">") >= 0) {
			lp.Write("run\n");
			sent_run = true;
		}
		
		if (sent_run && !sent_where && (out.Find("Exception occurred:") >= 0 || out.Find("main[") >= 0)) {
			lp.Write("where\n");
			sent_where = true;
			no_output_ms = 0;
		}
		
		if (sent_where && !sent_quit && no_output_ms >= 500) {
			lp.Write("quit\n");
			sent_quit = true;
		}
		
		Sleep(20);
		elapsed_ms += 20;
	}
	
	// Read remaining output
	String chunk;
	while (lp.Read(chunk)) {
		out.Cat(chunk);
		transcript.Cat(chunk);
	}
	
	Vector<String> lines = Split(out, '\n');
	for (int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		
		// Strip prompt at the beginning of the line if present
		// E.g. "main[1]   [3] JavaCrashSmoke.main..." -> "  [3] JavaCrashSmoke.main..."
		int p_open = line.Find('[');
		if (p_open > 0) {
			int p_close = line.Find(']', p_open);
			if (p_close > p_open) {
				bool is_prompt = true;
				for (int j = 0; j < p_open; j++) {
					if (!IsAlNum(line[j]) && line[j] != '_') {
						is_prompt = false;
						break;
					}
				}
				for (int j = p_open + 1; j < p_close; j++) {
					if (!IsDigit(line[j])) {
						is_prompt = false;
						break;
					}
				}
				if (is_prompt) {
					line = TrimBoth(line.Mid(p_close + 1));
				}
			}
		}
		
		// Expected format: [1] JavaCrashSmoke.level2 (JavaCrashSmoke.java:4)
		if (line.StartsWith("[")) {
			int close_bracket = line.Find(']');
			if (close_bracket < 0) continue;
			
			String remainder = TrimBoth(line.Mid(close_bracket + 1));
			int open_paren = remainder.Find('(');
			if (open_paren < 0) continue;
			
			DbgFrame frame;
			frame.function = TrimBoth(remainder.Left(open_paren));
			
			int close_paren = remainder.Find(')', open_paren);
			if (close_paren >= 0) {
				String loc = remainder.Mid(open_paren + 1, close_paren - open_paren - 1);
				int colon = loc.ReverseFind(':');
				if (colon >= 0) {
					frame.file = TrimBoth(loc.Left(colon));
					frame.line = atoi(~loc.Mid(colon + 1));
				} else {
					frame.file = loc;
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
