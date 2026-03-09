#ifdef flagGUI
#include "MCP.h"
#include <ide/ide.h>
#include <ide/Debuggers/Debuggers.h>

NAMESPACE_UPP

DebugBridge sDebugBridge;

// ---- helpers ---------------------------------------------------------------

bool DebugBridge::RunOnGui(Function<void()> fn, int timeout_ms) const
{
	Semaphore done;
	PostCallback([fn = pick(fn), &done]() mutable {
		fn();
		done.Release();
	});
	return done.Wait(timeout_ms);
}

String DebugBridge::DetectBackend() const
{
	// Must be called under GuiLock.
	Ide* ide = TheIde();
	if(!ide || !ide->debugger)
		return String();
	if(dynamic_cast<Gdb*>(ide->debugger.Get()))
		return "gdb";
	if(dynamic_cast<LLDB*>(ide->debugger.Get()))
		return "lldb";
#ifdef PLATFORM_WIN32
	if(dynamic_cast<Pdb*>(ide->debugger.Get()))
		return "pdb";
#endif
	return "unknown";
}

// ---- state -----------------------------------------------------------------

DbgState DebugBridge::GetState() const
{
	DbgState s;
	GuiLock __;
	Ide* ide = TheIde();
	s.active  = ide && ide->debugger;
	s.file    = IdeGetFileName();
	s.line    = IdeGetFileLine();
	// paused = debugger exists AND not running (debuglock == 0 means stopped at breakpoint)
	s.paused  = s.active && ide->debuglock == 0;
	s.backend = DetectBackend();
	return s;
}

// ---- breakpoints -----------------------------------------------------------

Vector<DbgBreakpoint> DebugBridge::GetBreakpoints() const
{
	Mutex::Lock __(bp_mutex);
	return clone(breakpoints);
}

String DebugBridge::SetBreakpoint(const String& file, int line, const String& condition)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		String bp = condition.IsEmpty() ? "1" : condition;
		// lineinfo uses 0-based lineno; MCP line is 1-based. Convert.
		int lineno = line - 1;
		// Always write to Filedata.lineinfo — this is what the debugger reads at session start.
		LineInfo& li = ide->Filedata(file).lineinfo;
		bool found = false;
		for(int i = 0; i < li.GetCount(); i++) {
			if(li[i].lineno == lineno) {
				li[i].breakpoint = bp;
				found = true;
				break;
			}
		}
		if(!found) {
			LineInfoRecord r;
			r.lineno = lineno; r.breakpoint = bp; r.count = 1; r.firstedited = 0;
			li.Add(r);
		}
		// If the file is currently open in the editor, also set it there (shows red dot).
		// editor.SetBreakpoint takes 0-based editor line index.
		if(PathIsEqual(file, ide->editfile)) {
			ide->editor.SetBreakpoint(lineno, bp);
			ide->editor.RefreshFrame();
		}
		// If debugger is already active, notify it directly (also 0-based).
		if(ide->debugger)
			ide->debugger->SetBreakpoint(file, lineno, bp);
	});
	if(!err.IsEmpty())
		return err;
	Mutex::Lock __(bp_mutex);
	for(int i = breakpoints.GetCount() - 1; i >= 0; i--)
		if(breakpoints[i].file == file && breakpoints[i].line == line)
			breakpoints.Remove(i);
	DbgBreakpoint& b = breakpoints.Add();
	b.file = file; b.line = line; b.condition = condition; b.enabled = true;
	return String();
}

String DebugBridge::ClearBreakpoint(const String& file, int line)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		int lineno = line - 1;
		// Clear from Filedata.lineinfo
		LineInfo& li = ide->Filedata(file).lineinfo;
		for(int i = 0; i < li.GetCount(); i++)
			if(li[i].lineno == lineno) { li[i].breakpoint = String(); break; }
		// Clear via editor if file is open (hides red dot)
		if(PathIsEqual(file, ide->editfile)) {
			ide->editor.SetBreakpoint(lineno, Null);
			ide->editor.RefreshFrame();
		}
		// Notify active debugger
		if(ide->debugger)
			ide->debugger->SetBreakpoint(file, lineno, String());
	});
	Mutex::Lock __(bp_mutex);
	for(int i = breakpoints.GetCount() - 1; i >= 0; i--)
		if(breakpoints[i].file == file && breakpoints[i].line == line)
			breakpoints.Remove(i);
	return err;
}

// ---- build control ---------------------------------------------------------

String DebugBridge::BuildStart()
{
	if(!TheIde()) return "IDE not available";
	PostCallback([] { if(Ide* ide = TheIde()) ide->DoBuild(); });
	return String();
}

String DebugBridge::BuildStop()
{
	if(!TheIde()) return "IDE not available";
	PostCallback([] { if(Ide* ide = TheIde()) ide->StopBuild(); });
	return String();
}

bool DebugBridge::IsBuilding() const
{
	GuiLock __;
	Ide* ide = TheIde();
	return ide && ide->IdeIsBuilding();
}

String DebugBridge::RunStart()
{
	if(!TheIde()) return "IDE not available";
	PostCallback([] { if(Ide* ide = TheIde()) ide->BuildAndExecute(); });
	return String();
}

// ---- session control -------------------------------------------------------

String DebugBridge::Start()
{
	if(!TheIde())
		return "IDE not available";
	// Asynchronous — do not wait; BuildAndDebug() runs build + launches debugger.
	PostCallback([] { if(TheIde()) TheIde()->BuildAndDebug(false); });
	return String();
}

String DebugBridge::Stop()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		ide->debugger->Stop();
	});
	return err;
}

String DebugBridge::Continue()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		ide->debugger->Run();
	});
	return err;
}

String DebugBridge::StepOver()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->Step("next");
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->Step("thread step-over");
		else
			err = "Step not supported for this debugger backend";
	});
	return err;
}

String DebugBridge::StepInto()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->Step("step");
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->Step("thread step-in");
		else
			err = "Step not supported for this debugger backend";
	});
	return err;
}

String DebugBridge::StepOut()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->Step("finish");
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->Step("thread step-out");
		else
			err = "Step not supported for this debugger backend";
	});
	return err;
}

String DebugBridge::Pause()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->BreakRunning();
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->BreakRunning();
		else
			err = "Pause not supported for this debugger backend";
	});
	return err;
}

// ---- state inspection ------------------------------------------------------

Vector<DbgFrame> DebugBridge::GetStackFrames(int limit) const
{
	String raw;
	String backend;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get())) {
			raw = g->Cmd(("bt " + AsString(limit)).Begin());
			backend = "gdb";
		}
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get())) {
			// LLDB: "thread backtrace" gives all frames in GDB-compatible #N format
			raw = l->Cmd("thread backtrace");
			backend = "lldb";
		}
	});
	Vector<DbgFrame> frames;
	ParseBacktrace(raw, frames);
	if(frames.GetCount() > limit)
		frames.SetCount(limit);
	return frames;
}

VectorMap<String, String> DebugBridge::GetLocals() const
{
	String raw;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			raw = g->Cmd("info locals");
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			raw = l->Cmd("info locals"); // LLDB also accepts GDB-style "info locals"
	});
	VectorMap<String, String> result;
	ParseLocals(raw, result);
	return result;
}

String DebugBridge::Evaluate(const String& expr) const
{
	String result;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { result = "<no active debugger>"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			result = g->Print(expr);
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			result = l->Print(expr);
		else
			result = "<unsupported backend>";
	});
	return result;
}

String DebugBridge::GetRegisters() const
{
	String raw;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			raw = g->McpGetRegisters();
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			raw = l->McpGetRegisters();
	});
	return raw;
}

String DebugBridge::GetDisassembly() const
{
	String raw;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			raw = g->McpGetDisassembly();
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			raw = l->McpGetDisassembly();
	});
	return raw;
}

VectorMap<String, String> DebugBridge::GetWatches() const
{
	VectorMap<String, String> result;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get())) {
			for(int i = 0; i < g->McpGetWatchCount(); i++)
				result.Add(g->McpGetWatchExpr(i), g->McpGetWatchValue(i));
		} else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get())) {
			for(int i = 0; i < l->McpGetWatchCount(); i++)
				result.Add(l->McpGetWatchExpr(i), l->McpGetWatchValue(i));
		}
	});
	return result;
}

String DebugBridge::AddWatch(const String& expr)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->McpAddWatch(expr);
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->McpAddWatch(expr);
		else
			err = "Watch not supported for this debugger backend";
	});
	return err;
}

String DebugBridge::RemoveWatch(int index)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get())) {
			if(index < 0 || index >= g->McpGetWatchCount()) { err = "Watch index out of range"; return; }
			g->McpRemoveWatch(index);
		} else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get())) {
			if(index < 0 || index >= l->McpGetWatchCount()) { err = "Watch index out of range"; return; }
			l->McpRemoveWatch(index);
		} else
			err = "Watch not supported for this debugger backend";
	});
	return err;
}

String DebugBridge::ClearWatches()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			g->McpClearWatches();
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			l->McpClearWatches();
		else
			err = "Watch not supported for this debugger backend";
	});
	return err;
}

Vector<String> DebugBridge::GetThreads() const
{
	String raw;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
			raw = g->ObtainThreadsInfo();
		else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
			raw = l->ObtainThreadsInfo();
	});
	Vector<String> result;
	StringStream ss(raw);
	// Skip the header line ("  Id   Target Id ...") present in GDB output.
	bool first = true;
	while(!ss.IsEof()) {
		String line = ss.GetLine();
		if(first) { first = false; if(line.StartsWith("  Id")) continue; }
		line = TrimBoth(line);
		if(!line.IsEmpty())
			result.Add(line);
	}
	return result;
}

END_UPP_NAMESPACE
#endif // flagGUI
