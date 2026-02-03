#ifndef _Maestro_WorkSession_h_
#define _Maestro_WorkSession_h_

#include <Core/Core.h>

namespace Upp {

enum class WorkSessionStatus {
	RUNNING,
	PAUSED,
	COMPLETED,
	INTERRUPTED,
	FAILED,
	UNKNOWN
};

String StatusToString(WorkSessionStatus s);
WorkSessionStatus StringToWorkSessionStatus(const String& s);

struct WorkSession : Moveable<WorkSession> {
	String            session_id;
	String            session_type;
	String            parent_session_id;
	Vector<String>    children_ids;
	WorkSessionStatus status = WorkSessionStatus::RUNNING;
	String            state = "running";
	String            purpose;
	ValueMap          context;
	Time              created;
	Time              modified;
	ValueMap          related_entity;
	String            breadcrumbs_dir;
	ValueMap          metadata;

	void Jsonize(JsonIO& jio);
	
	WorkSession();
	WorkSession(WorkSession&& s) : Moveable<WorkSession>(s) {
		session_id = pick(s.session_id);
		session_type = pick(s.session_type);
		parent_session_id = pick(s.parent_session_id);
		children_ids = pick(s.children_ids);
		status = s.status;
		state = pick(s.state);
		purpose = pick(s.purpose);
		context = pick(s.context);
		created = s.created;
		modified = s.modified;
		related_entity = pick(s.related_entity);
		breadcrumbs_dir = pick(s.breadcrumbs_dir);
		metadata = pick(s.metadata);
	}
};

class WorkSessionManager {
public:
	static String GetSessionsBasePath(const String& docs_root);
	static Array<WorkSession> ListSessions(const String& docs_root);
	static bool LoadSession(const String& path, WorkSession& session);
	static bool SaveSession(const WorkSession& session, const String& path);
	static String FindSessionPath(const String& docs_root, const String& session_id);
	static WorkSession CreateSession(const String& docs_root, const String& type, const String& purpose = "");
};

}

#endif
