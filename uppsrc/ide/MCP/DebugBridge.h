#ifndef _ide_MCP_DebugBridge_h_
#define _ide_MCP_DebugBridge_h_

// Thread-safe facade over the live Debugger (Gdb/LLDB/Pdb) on the GUI thread.
// McpServer runs on its own thread; DebugBridge mediates all cross-thread access.
//
// Read-only state (GetState, GetBreakpoints) use GuiLock or a private mutex.
// Mutating / commanding operations (Step, Continue, etc.) use RunOnGui():
//   PostCallback to the GUI event loop, then block on a Semaphore until done.
//   Timeout: 8 seconds (generous for template-heavy step operations).

class DebugBridge {
public:
	// --- Read operations (safe from any thread) ---

	// Snapshot of current debug session state.
	DbgState GetState() const;

	// Copy of the tracked breakpoint list.
	Vector<DbgBreakpoint> GetBreakpoints() const;

	// --- Inspection operations (post to GUI thread, block until done) ---

	// Returns parsed call stack frames (up to limit).
	Vector<DbgFrame> GetStackFrames(int limit = 30) const;

	// Returns local variable name->value map.
	VectorMap<String, String> GetLocals() const;

	// Evaluates an expression and returns its string value.
	String Evaluate(const String& expr) const;

	// Returns raw thread list lines from the debugger.
	Vector<String> GetThreads() const;

	// Returns raw "info registers" output (one line per register).
	String GetRegisters() const;

	// Returns raw disassembly text at the current instruction pointer.
	String GetDisassembly() const;

	// Returns all current watches as {expr, value} pairs.
	VectorMap<String, String> GetWatches() const;

	// Adds an expression to the watch list. Returns error or empty.
	String AddWatch(const String& expr);

	// Removes the watch at index i. Returns error or empty.
	String RemoveWatch(int index);

	// Removes all watches. Returns error or empty.
	String ClearWatches();

	// --- Build control ---

	// Start a build (asynchronous).
	String BuildStart();

	// Abort a running build.
	String BuildStop();

	// Returns true if a build is currently running.
	bool   IsBuilding() const;

	// Build and run (no debugger).
	String RunStart();

	// --- Debug session control (post to GUI thread, block until done) ---

	// Trigger build + debug start (asynchronous, returns immediately).
	String Start();

	// Stop the running debug session.
	String Stop();

	// Resume execution from a breakpoint/step.
	String Continue();

	// Step over current line.
	String StepOver();

	// Step into the call on the current line.
	String StepInto();

	// Step out of the current function.
	String StepOut();

	// Break into a running process (send interrupt).
	String Pause();

	// Set a breakpoint. condition="" means unconditional.
	// Returns empty string on success, error message on failure.
	String SetBreakpoint(const String& file, int line, const String& condition);

	// Clear a breakpoint.
	String ClearBreakpoint(const String& file, int line);

private:
	// Post fn to the GUI thread and block until it runs (or timeout elapses).
	// Returns false on timeout. timeout_ms=-1 means wait forever.
	bool RunOnGui(Function<void()> fn, int timeout_ms = 8000) const;

	// Detect which backend is active (Gdb/LLDB/Pdb).
	String DetectBackend() const;  // called under GuiLock

	mutable Mutex         bp_mutex;
	Vector<DbgBreakpoint> breakpoints;
};

extern DebugBridge sDebugBridge;

#endif
