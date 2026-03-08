#include "Maestro.h"

namespace Upp {

String StatusToString(WorkSessionStatus status) {
	switch(status) {
		case WorkSessionStatus::RUNNING: return "running";
		case WorkSessionStatus::PAUSED: return "paused";
		case WorkSessionStatus::COMPLETED: return "completed";
		case WorkSessionStatus::FAILED: return "failed";
		default: return "unknown";
	}
}

WorkSessionStatus StringToWorkSessionStatus(const String& s) {
	if(s == "running") return WorkSessionStatus::RUNNING;
	if(s == "paused") return WorkSessionStatus::PAUSED;
	if(s == "completed") return WorkSessionStatus::COMPLETED;
	if(s == "failed") return WorkSessionStatus::FAILED;
	return WorkSessionStatus::UNKNOWN;
}

void WorkSession::Jsonize(JsonIO& jio) {
	jio("session_id", session_id)
	   ("session_type", session_type)
	   ("parent_session_id", parent_session_id)
	   ("children_ids", children_ids)
	   ("status", (int&)status)
	   ("state", state)
	   ("purpose", purpose)
	   ("context", context)
	   ("created", created)
	   ("modified", modified)
	   ("related_entity", related_entity)
	   ("breadcrumbs_dir", breadcrumbs_dir)
	   ("metadata", metadata);
}

String WorkSessionManager::GetSessionsBasePath(const String& docs_root) {
	return AppendFileName(docs_root, "docs/sessions");
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
			
			// Legacy layout support: check subdirectories
			FindFile nff(AppendFileName(ff.GetPath(), "*"));
			while(nff) {
				if(nff.IsDirectory() && nff.GetName() != "." && nff.GetName() != "..") {
					String ps = AppendFileName(nff.GetPath(), "session.json");
					if(FileExists(ps)) {
						WorkSession s;
						if(LoadSession(ps, s) && seen.Find(s.session_id) < 0) {
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
	if(json.IsVoid()) return false;
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

SessionStats WorkSessionManager::CalculateSessionStats(const String& docs_root, const WorkSession& session) {
	Array<Breadcrumb> breadcrumbs = BreadcrumbManager::ListBreadcrumbs(docs_root, session.session_id);
	
	SessionStats st;
	st.total_breadcrumbs = breadcrumbs.GetCount();
	int success_count = 0;
	
	for(const auto& b : breadcrumbs) {
		st.total_tokens_input += b.input_tokens;
		st.total_tokens_output += b.output_tokens;
		st.estimated_cost += b.cost;
		st.files_modified += b.files_modified.GetCount();
		st.tools_called += b.tools_called.GetCount();
		if(b.error.IsEmpty()) success_count++;
	}
	
	if(st.total_breadcrumbs > 0) {
		st.success_rate = (double)success_count / st.total_breadcrumbs * 100.0;
		st.duration_seconds = session.modified - session.created;
	}
	
	return st;
}

} // namespace Upp
