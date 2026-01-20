#ifndef _Maestro_Engine_h_
#define _Maestro_Engine_h_

struct MaestroEvent {
	String type;        // "init", "delta", "message", "result", "error"
	String role;
	String text;        // delta or full text
	Value  json;        // raw event data
	bool   delta = false;
	String session_id;
	
	String ToString() const { return String().Cat() << type << (delta ? " (delta)" : "") << ": " << text; }
};

class MaestroEngine {
public:
	String debug_log;
	String session_id;

	virtual ~MaestroEngine() {}
	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) = 0;
	virtual void Cancel() = 0;
	virtual bool Do() = 0; // Returns true if running/processing
};

#endif
