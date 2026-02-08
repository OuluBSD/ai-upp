
#ifndef _Maestro_WorkSession_h_
#define _Maestro_WorkSession_h_

enum class WorkSessionStatus {
	RUNNING,
	PAUSED,
	COMPLETED,
	FAILED,
	UNKNOWN
};

String StatusToString(WorkSessionStatus status);
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
	
	WorkSession() { created = modified = GetSysTime(); }
	WorkSession(const WorkSession& s) {
		session_id = s.session_id;
		session_type = s.session_type;
		parent_session_id = s.parent_session_id;
		children_ids = clone(s.children_ids);
		status = s.status;
		state = s.state;
		purpose = s.purpose;
		context = clone(s.context);
		created = s.created;
		modified = s.modified;
		related_entity = clone(s.related_entity);
		breadcrumbs_dir = s.breadcrumbs_dir;
		metadata = clone(s.metadata);
	}
};

struct SessionStats {
	int    total_breadcrumbs = 0;
	int    total_tokens_input = 0;
	int    total_tokens_output = 0;
	double estimated_cost = 0;
	int    files_modified = 0;
	int    tools_called = 0;
	double duration_seconds = 0;
	double success_rate = 0;
};

class WorkSessionManager {
public:
	static String GetSessionsBasePath(const String& docs_root);
	static Array<WorkSession> ListSessions(const String& docs_root);
	static bool LoadSession(const String& path, WorkSession& session);
	static bool SaveSession(const WorkSession& session, const String& path);
	static String FindSessionPath(const String& docs_root, const String& session_id);
	static WorkSession CreateSession(const String& docs_root, const String& type, const String& purpose = "");
	static SessionStats CalculateSessionStats(const String& docs_root, const WorkSession& session);
};

#endif

