#ifndef _Maestro_Debugger_h_
#define _Maestro_Debugger_h_

struct StackFrame : Moveable<StackFrame> {
	String file;
	int line;
	String function;
	String address;
	
	String ToString() const { return function + " (" + file + ":" + AsString(line) + ")"; }
};

struct Variable : Moveable<Variable> {
	String name;
	String value;
	String type;
};

class DebuggerService {
public:
	virtual void Run(const String& cmd, const String& args) = 0;
	virtual void Stop() = 0;
	virtual void Step() = 0; // Step Into
	virtual void Next() = 0; // Step Over
	virtual void Cont() = 0;
	
	Event<const Vector<StackFrame>&> WhenStack;
	Event<const Vector<Variable>&> WhenLocals;
	Event<String> WhenOutput;
	Event<> WhenRunning;
	Event<> WhenPaused;
	
	virtual ~DebuggerService() {}
};

class GdbService : public DebuggerService {
public:
	// Stub implementation for now
	void Run(const String& cmd, const String& args) override;
	void Stop() override;
	void Step() override;
	void Next() override;
	void Cont() override;
	
	typedef GdbService CLASSNAME;
	GdbService();
};

#endif
