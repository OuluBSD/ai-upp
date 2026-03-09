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
	s.active  = IdeIsDebug();
	s.file    = IdeGetFileName();
	s.line    = IdeGetFileLine();
	s.paused  = s.active && !s.file.IsEmpty();
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
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		// Non-empty bp string = set; the condition string is the value.
		String bp = condition.IsEmpty() ? "1" : condition;
		if(!ide->debugger->SetBreakpoint(file, line, bp))
			err = "Debugger rejected breakpoint";
	});
	if(!err.IsEmpty())
		return err;
	// Update tracked list.
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
		if(!ide || !ide->debugger) { err = "No active debugger"; return; }
		// bp="" signals removal per Debugger interface contract.
		ide->debugger->SetBreakpoint(file, line, String());
	});
	// Remove from tracking regardless (bp may have been set before session started).
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
