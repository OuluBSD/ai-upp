#ifndef _ide_MCP_DebugProtocol_h_
#define _ide_MCP_DebugProtocol_h_

// debug.* protocol types and JSON helpers.
// All structs are plain data; no locking concerns here (DebugBridge fills them).

// Current debugger session state
struct DbgState : Moveable<DbgState> {
	bool   active  = false;  // debugger is running
	bool   paused  = false;  // execution is stopped at a breakpoint/step
	String file;             // current source file (empty when running)
	int    line    = 0;      // current line (0 when running)
	String backend;          // "gdb", "lldb", "pdb", or ""
};

// A single tracked breakpoint
struct DbgBreakpoint : Moveable<DbgBreakpoint> {
	String file;
	int    line      = 0;
	String condition;        // "" = unconditional
	bool   enabled   = true;
};

// A single stack frame
struct DbgFrame : Moveable<DbgFrame> {
	int    index    = 0;
	String function;
	String file;
	int    line     = 0;
	String address;
};

// JSON value converters (DbgState -> ValueMap, etc.)

inline Value ToValue(const DbgState& s)
{
	ValueMap m;
	m.Add("active",  s.active);
	m.Add("paused",  s.paused);
	m.Add("file",    s.file);
	m.Add("line",    s.line);
	m.Add("backend", s.backend);
	return m;
}

inline Value ToValue(const DbgBreakpoint& b)
{
	ValueMap m;
	m.Add("file",      b.file);
	m.Add("line",      b.line);
	m.Add("condition", b.condition);
	m.Add("enabled",   b.enabled);
	return m;
}

inline Value ToValue(const DbgFrame& f)
{
	ValueMap m;
	m.Add("index",    f.index);
	m.Add("function", f.function);
	m.Add("file",     f.file);
	m.Add("line",     f.line);
	m.Add("address",  f.address);
	return m;
}

// Parse GDB/LLDB backtrace text into DbgFrame list.
// GDB format:  #0  main (argc=1 ...) at /path/file.cpp:5
// LLDB format: frame #0: 0x... `main at file.cpp:5
inline void ParseBacktrace(const String& raw, Vector<DbgFrame>& out)
{
	StringStream ss(raw);
	while(!ss.IsEof()) {
		String line = ss.GetLine();
		DbgFrame f;

		// Locate '#N' — GDB starts with it, LLDB starts with "frame #N:"
		const char* s = line;
		while(*s == ' ' || *s == '*') s++;
		// LLDB: skip "frame " prefix if present
		if(s[0]=='f' && s[1]=='r' && s[2]=='a' && s[3]=='m' && s[4]=='e' && s[5]==' ') s += 6;
		if(*s != '#') continue;
		s++;
		f.index = 0;
		while(IsDigit(*s)) { f.index = f.index * 10 + (*s - '0'); s++; }
		while(*s == ' ') s++;

		// Skip LLDB ":" after frame index
		if(*s == ':') s++;
		while(*s == ' ') s++;

		// Optional hex address: "0x..." followed by "in" (GDB) or "`" (LLDB)
		if(s[0] == '0' && ToUpper(s[1]) == 'X') {
			const char* a = s;
			s += 2; while(IsXDigit(*s)) s++;
			f.address = String(a, s);
			while(*s == ' ') s++;
			if(s[0] == 'i' && s[1] == 'n') { s += 2; while(*s == ' ') s++; } // GDB: "in"
			if(*s == '`') s++;                                                  // LLDB: "`func"
			while(*s == ' ') s++;
		}

		// Function name: up to ' ' or '('
		const char* fn = s;
		while(*s && *s != ' ' && *s != '(') s++;
		f.function = String(fn, s);

		// Scan for " at file:line" or " at file" near end
		const char* at = strstr(line, " at ");
		if(at) {
			at += 4;
			const char* col = strrchr(at, ':');
			if(col && IsDigit(col[1])) {
				f.file = String(at, col);
				f.line = StrInt(col + 1);
			} else {
				f.file = at;
			}
		}

		if(!f.function.IsEmpty())
			out.Add(f);
	}
}

// Parse "name = value" lines from GDB "info locals" / LLDB "frame variable".
inline void ParseLocals(const String& raw, VectorMap<String, String>& out)
{
	StringStream ss(raw);
	while(!ss.IsEof()) {
		String line = ss.GetLine();
		int eq = line.Find('=');
		if(eq > 0) {
			String name  = TrimBoth(line.Left(eq));
			String value = TrimBoth(line.Mid(eq + 1));
			if(!name.IsEmpty() && !name.StartsWith("No locals") && !name.StartsWith("No symbol"))
				out.GetAdd(name) = value;
		}
	}
}

#endif
