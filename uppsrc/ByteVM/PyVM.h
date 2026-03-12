#ifndef _ByteVM_PyVM_h_
#define _ByteVM_PyVM_h_

#include <Draw/Draw.h>

NAMESPACE_UPP

class PyVM {
	struct Frame : Moveable<Frame> {
		PyValue func;
		const Vector<PyIR>* ir;
		int pc;
		VectorMap<PyValue, PyValue> locals;
		PyValue globals;
		bool    is_module = false;
		int     stack_base = 0; // stack depth when frame was entered

		struct ExceptHandler {
			int handler_pc;   // jump target in IR on exception
			int stack_depth;  // stack depth to restore when entering handler
		};
		Vector<ExceptHandler> except_stack;
	};
	
	Vector<Frame> frames;
	Vector<PyValue> stack;
	PyValue globals; // This is the 'main' module's globals dict

	Frame& TopFrame() { return frames.Top(); }
	void Push(PyValue v);
	PyValue Pop();

public:
	// Breakpoint management
	struct Breakpoint : Moveable<Breakpoint> {
		String file;
		int line;
		bool enabled;
		int hit_count;

		Breakpoint() : line(0), enabled(true), hit_count(0) {}
		Breakpoint(const String& f, int l) : file(f), line(l), enabled(true), hit_count(0) {}
	};

	enum DebugState {
		DEBUG_RUNNING,      // Normal execution
		DEBUG_PAUSED,       // Paused at breakpoint or step
		DEBUG_STEP_OVER,    // Step over next instruction
		DEBUG_STEP_IN,      // Step into next instruction
		DEBUG_STEP_OUT      // Step out of current function
	};

	void Continue();        // Resume from pause
	void Pause();           // Pause execution
	void StepOver();        // Execute next line, don't enter calls
	void StepIn();          // Execute next line, enter calls
	void StepOut();         // Run until current function returns
	void Reset();           // Stop execution and clear frames
	DebugState GetDebugState() const { return debug_state; }

	struct StackFrame : Moveable<StackFrame> {
		String function_name;
		String file;
		int line;
		int frame_index;
		const VectorMap<PyValue, PyValue>* locals;  // Pointer to frame locals
	};

	// Get current call stack (deepest first)
	Vector<StackFrame> GetCallStack() const;

	// Get locals at specific frame
	const VectorMap<PyValue, PyValue>& GetLocals(int frame_index) const;

	// Get current frame index
	int GetCurrentFrameIndex() const { return frames.GetCount() - 1; }

	// Add breakpoint
	void AddBreakpoint(const String& file, int line);
	void RemoveBreakpoint(const String& file, int line);
	void ClearBreakpoints();
	void EnableBreakpoint(const String& file, int line, bool enable = true);

	// Query breakpoints
	bool HasBreakpoint(const String& file, int line) const;
	const Vector<Breakpoint>& GetBreakpoints() const { return breakpoints; }

	// Breakpoint hit callback
	Event<const String&, int> WhenBreakpointHit;

	// Output callback
	Event<const String&> WhenPrint;
	Event<const Image&> WhenPlot;

	PyVM();
	~PyVM();
	void Clear();

	void SetIR(Vector<PyIR>& ir);
	PyValue Run();
	PyValue Call(const PyValue& callable, const Vector<PyValue>& args);

	// Pre-load a Python source file as a named module into sys.modules.
	// Dotted name like "hearts.logic" is supported: the parent "hearts" package
	// dict is created automatically.  Returns false on compile/run error.
	bool LoadModule(const String& module_name, const String& src, const String& filename);
	
	PyValue GetGlobals() { return globals; }
	
	bool    Step();
	PyValue GetLastResult() const { return last_result; }
	bool    IsRunning() const { return !frames.IsEmpty(); }
	int     GetFramesCount() const { return frames.GetCount(); }

	String  GetCurrentFile() const { return current_file; }
	int     GetCurrentLine() const { return current_line; }

private:
	PyValue last_result = PyValue::None();
	int     instruction_count = 0;
	Vector<Breakpoint> breakpoints;
	DebugState debug_state = DEBUG_RUNNING;
	int step_frame_depth = 0;
	String current_file;
	int current_line = 0;

	bool CheckBreakpoint(const String& file, int line);
};

END_UPP_NAMESPACE

#endif
