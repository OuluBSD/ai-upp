#ifndef _Maestro_Engine_h_
#define _Maestro_Engine_h_

#include <Core/Core.h>

NAMESPACE_UPP

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
	String tool_name;
	String tool_input;
	
	String ToString() const { return String().Cat() << type << (delta ? " (delta)" : "") << ": " << text; }
};

class MaestroEngine {
public:
	String debug_log;
	String session_id;

	virtual ~MaestroEngine() {}
	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) = 0;
	virtual void Cancel() = 0;
	virtual bool Do() = 0;
	virtual void ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) = 0;
};

END_UPP_NAMESPACE

#endif
