#ifndef _Maestro_Engine_h_
#define _Maestro_Engine_h_

#ifdef _GraphLib_GraphLib_h_
 #error Wrong inclusion order
#endif


struct SessionInfo : Moveable<SessionInfo> {
	String   id;
	String   name;
	Time     timestamp;
	ValueMap metadata;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("timestamp", timestamp)("metadata", metadata);
	}
	
	template <class T>
	void Visit(T& v) {
		v("id", id)("name", name)("timestamp", timestamp)("metadata", metadata);
	}
};

struct MaestroEvent {
	String type;
	String role;
	String persona;
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
	
	String GetToolSummary() const {
		String s;
		if(tools.GetCount() > 0) {
			s << "Available Tools:\n";
			for(int i = 0; i < tools.GetCount(); i++) {
				const MaestroTool& t = tools[i];
				s << "- " << t.GetName() << ": " << t.GetDescription() << "\n";
				s << "  Schema: " << StoreAsJson(t.GetSchema()) << "\n";
			}
		}
		return s;
	}
};

class MockMaestroEngine : public MaestroEngine {
public:
	struct MockResponse {
		String regex;
		String response;
	};
	
	Array<MockResponse> mocks;

	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) override {
		String res = "Mock Response";
		for(const auto& m : mocks) {
			if(RegExp(m.regex).Match(prompt)) {
				res = m.response;
				break;
			}
		}
		
		if(cb) {
			MaestroEvent e;
			e.type = "message";
			e.text = res;
			e.role = "assistant";
			e.delta = false;
			cb(e);
			
			e.type = "done";
			cb(e);
		}
	}
	
	virtual void Cancel() override {}
	virtual bool Do() override { return false; }
	virtual void ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) override {}
	
	void AddMock(String regex, String response) {
		MockResponse& m = mocks.Add();
		m.regex = regex;
		m.response = response;
	}
	
	static MockMaestroEngine& Get() {
		static MockMaestroEngine e;
		return e;
	}
};


#endif