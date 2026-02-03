#include "WorkSession.h"

namespace Upp {

String StatusToString(WorkSessionStatus s) {
	switch(s) {
		case WorkSessionStatus::RUNNING: return "running";
		case WorkSessionStatus::PAUSED: return "paused";
		case WorkSessionStatus::COMPLETED: return "completed";
		case WorkSessionStatus::INTERRUPTED: return "interrupted";
		case WorkSessionStatus::FAILED: return "failed";
		default: return "unknown";
	}
}

WorkSessionStatus StringToWorkSessionStatus(const String& s) {
	if(s == "running") return WorkSessionStatus::RUNNING;
	if(s == "paused") return WorkSessionStatus::PAUSED;
	if(s == "completed") return WorkSessionStatus::COMPLETED;
	if(s == "interrupted") return WorkSessionStatus::INTERRUPTED;
	if(s == "failed") return WorkSessionStatus::FAILED;
	return WorkSessionStatus::UNKNOWN;
}

WorkSession::WorkSession() {
	created = modified = GetSysTime();
}

void WorkSession::Jsonize(JsonIO& jio) {
	jio("session_id", session_id)
	   ("session_type", session_type)
	   ("parent_session_id", parent_session_id)
	   ("children_wsession_ids", children_ids)
	   ("state", state)
	   ("purpose", purpose)
	   ("context", context)
	   ("related_entity", related_entity)
	   ("breadcrumbs_dir", breadcrumbs_dir)
	   ("metadata", metadata);
	
	String s = StatusToString(status);
	jio("status", s);
	if(jio.IsLoading()) status = StringToWorkSessionStatus(s);
	
jio("created", created)
	   ("modified", modified);
}

String WorkSessionManager::GetSessionsBasePath(const String& docs_root) {
	return AppendFileName(AppendFileName(docs_root, "docs"), "sessions");
}

Array<WorkSession> WorkSessionManager::ListSessions(const String& docs_root) {
	Array<WorkSession> list;
	String base = GetSessionsBasePath(docs_root);
	if(!DirectoryExists(base)) return list;
	
	Index<String> seen;
	FindFile ff(AppendFileName(base, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			String session_file = AppendFileName(ff.GetPath(), "session.json");
			if(FileExists(session_file)) {
				WorkSession s;
				if(LoadSession(session_file, s) && seen.Find(s.session_id) < 0) {
					seen.Add(s.session_id);
					list.Add(pick(s));
				}
			}
			
			// Check one level deeper for nested sessions
			FindFile nff(AppendFileName(ff.GetPath(), "*"));
			while(nff) {
				if(nff.IsDirectory() && nff.GetName() != "." && nff.GetName() != "..") {
					String nested_file = AppendFileName(nff.GetPath(), "session.json");
					if(FileExists(nested_file)) {
						WorkSession s;
						if(LoadSession(nested_file, s) && seen.Find(s.session_id) < 0) {
							seen.Add(s.session_id);
							list.Add(pick(s));
						}
					}
				}
				nff.Next();
			}
		}
		ff.Next();
	}
	return list;
}

bool WorkSessionManager::LoadSession(const String& path, WorkSession& session) {
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	return LoadFromJson(session, json);
}

bool WorkSessionManager::SaveSession(const WorkSession& session, const String& path) {
	String json = StoreAsJson(session, true);
	bool ok = SaveFile(path, json);
	if(!ok) Cerr() << "Error: Failed to save session to " << path << "\n";
	return ok;
}

String WorkSessionManager::FindSessionPath(const String& docs_root, const String& session_id) {
	String base = GetSessionsBasePath(docs_root);
	if(!DirectoryExists(base)) return "";
	
	FindFile ff(AppendFileName(base, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			if(session_id.StartsWith(ff.GetName())) {
				String p = AppendFileName(ff.GetPath(), "session.json");
				if(FileExists(p)) return p;
			}
			
			FindFile nff(AppendFileName(ff.GetPath(), "*"));
			while(nff) {
				if(nff.IsDirectory() && nff.GetName() != "." && nff.GetName() != ".." && session_id.StartsWith(nff.GetName())) {
					String p = AppendFileName(nff.GetPath(), "session.json");
					if(FileExists(p)) return p;
				}
				nff.Next();
			}
		}
		ff.Next();
	}
	return "";
}

WorkSession WorkSessionManager::CreateSession(const String& docs_root, const String& type, const String& purpose) {
	WorkSession s;
	s.session_id = AsString(Uuid::Create());
	s.session_type = type;
	s.purpose = purpose;
	s.status = WorkSessionStatus::RUNNING;
	s.state = "running";
	
	String base = GetSessionsBasePath(docs_root);
	String session_dir = AppendFileName(base, s.session_id);
	RealizeDirectory(session_dir);
	
	String breadcrumbs_dir = AppendFileName(session_dir, "breadcrumbs");
	RealizeDirectory(breadcrumbs_dir);
	s.breadcrumbs_dir = breadcrumbs_dir;
	
	SaveSession(s, AppendFileName(session_dir, "session.json"));
	return s;
}

} // namespace Upp