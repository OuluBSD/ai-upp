#ifndef _Maestro_Engine_h_
#define _Maestro_Engine_h_

struct SessionInfo : Moveable<SessionInfo> {
	String   id;
	String   name;
	Time     timestamp;
	ValueMap metadata;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("timestamp", timestamp)("metadata", metadata);
	}
};

struct MaestroEvent {
	String type;
	String role;
	String text;
	Value  json;
	bool   delta = false;
	String session_id;
	String tool_id;
	String tool_name;
	String tool_input;
	
	String ToString() const { return String().Cat() << type << (delta ? " (delta)" : "") << ": " << text; }
};

class MaestroEngine {
public:
	String debug_log;
	String session_id;
	String model;

	virtual ~MaestroEngine() {}
	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) = 0;
	virtual void Cancel() = 0;
	virtual bool Do() = 0;
	virtual void ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) = 0;
};

class MaestroTool : public Moveable<MaestroTool> {
public:
	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;
	virtual Value  GetSchema() const = 0; // JSON Schema
	virtual Value  Execute(const ValueMap& params) const = 0;
	virtual ~MaestroTool() {}
};

class MaestroToolRegistry {
	ArrayMap<String, MaestroTool> tools;
public:
	void Add(MaestroTool *t) { tools.Add(t->GetName(), t); }
	const MaestroTool* Find(const String& name) const { 
		int q = tools.Find(name); 
		return q >= 0 ? &tools[q] : nullptr; 
	}
	const ArrayMap<String, MaestroTool>& GetTools() const { return tools; }
};

#endif