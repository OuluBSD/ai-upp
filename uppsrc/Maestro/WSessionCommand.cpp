#include "Maestro.h"

namespace Upp {

void WSessionCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI wsession [-h] {list,ls,show,sh,create,breadcrumbs,timeline,stats}\n"
	       << "\n"
	       << "Work session subcommands:\n"
	       << "    list (ls)           List all work sessions\n"
	       << "    show (sh) <id>      Show work session details\n"
	       << "    create <type> [purp] Create a new work session\n"
	       << "    breadcrumbs <id>    Show breadcrumbs for a session\n"
	       << "    timeline <id>       Show session timeline\n"
	       << "    stats [id]          Show work session statistics\n";
}

void WSessionCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	String plan_root = FindPlanRoot();
	if(plan_root.IsEmpty()) { Cerr() << "Error: Could not find project root.\n"; return; }
	String docs_root = GetDocsRoot(plan_root);

	String sub = AsString(cla.GetPositional(0));
	if (sub == "list" || sub == "ls") {
		Array<WorkSession> sessions = WorkSessionManager::ListSessions(docs_root);
		if(sessions.IsEmpty()) { Cout() << "No work sessions found.\n"; return; }
		Cout() << Format("% -40s % -15s % -15s %s\n", "SESSION ID", "TYPE", "STATUS", "CREATED");
		Cout() << String('-', 80) << "\n";
		for(const auto& s : sessions) Cout() << Format("% -40s % -15s % -15s %s\n", s.session_id, s.session_type, StatusToString(s.status), Format(s.created));
	}
	else if (sub == "create") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires session type.\n"; return; }
		String type = AsString(cla.GetPositional(1));
		String purpose = cla.GetPositionalCount() > 2 ? AsString(cla.GetPositional(2)) : "";
		WorkSession s = WorkSessionManager::CreateSession(docs_root, type, purpose);
		Cout() << "Created session: " << s.session_id << "\n";
	}
	else if (sub == "show" || sub == "sh") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires session ID.\n"; return; }
		String id = AsString(cla.GetPositional(1));
		String path = WorkSessionManager::FindSessionPath(docs_root, id);
		if(path.IsEmpty()) { Cerr() << "Error: Session not found.\n"; return; }
		WorkSession s;
		if(WorkSessionManager::LoadSession(path, s)) {
			Cout() << "Session ID:      " << s.session_id << "\n" << "Type:            " << s.session_type << "\n" << "Status:          " << StatusToString(s.status) << "\n" << "State:           " << s.state << "\n" << "Parent:          " << s.parent_session_id << "\n" << "Created:         " << s.created << "\n" << "Modified:        " << s.modified << "\n" << "Purpose:         " << s.purpose << "\n" << "Breadcrumbs Dir: " << s.breadcrumbs_dir << "\n";
			if(s.related_entity.GetCount() > 0) Cout() << "Related Entity:  " << s.related_entity << "\n";
			if(s.metadata.GetCount() > 0) Cout() << "Metadata:        " << s.metadata << "\n";
			
			SessionStats st = WorkSessionManager::CalculateSessionStats(docs_root, s);
			Cout() << "\nStatistics:\n"
			       << "  Breadcrumbs:   " << st.total_breadcrumbs << "\n"
			       << "  Tokens:        In: " << st.total_tokens_input << ", Out: " << st.total_tokens_output << "\n"
			       << "  Cost:          $" << Format("%.6f", st.estimated_cost) << "\n"
			       << "  Files Modified: " << st.files_modified << "\n"
			       << "  Tools Called:   " << st.tools_called << "\n"
			       << "  Duration:      " << (int)st.duration_seconds << " seconds\n"
			       << "  Success Rate:  " << Format("%.1f", st.success_rate) << "%\n";
		} else Cerr() << "Error: Failed to load session.\n";
	}
	else if (sub == "breadcrumbs" || sub == "timeline") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires session ID.\n"; return; }
		String id = AsString(cla.GetPositional(1));
		Array<Breadcrumb> list = BreadcrumbManager::ListBreadcrumbs(docs_root, id);
		if(list.IsEmpty()) { Cout() << "No breadcrumbs found.\n"; return; }
		Cout() << "Breadcrumbs for Session: " << id << "\n" << String('-', 80) << "\n";
		for(int i = 0; i < list.GetCount(); i++) {
			const auto& b = list[i];
			Cout() << Format("%d. [%s] - %s\n", i + 1, b.timestamp_id, b.model_used);
			Cout() << "   Prompt:   " << b.prompt.Left(100) << (b.prompt.GetCount() > 100 ? "..." : "") << "\n";
			Cout() << "   Response: " << b.response.Left(100) << (b.response.GetCount() > 100 ? "..." : "") << "\n\n";
		}
	}
	else if (sub == "stats") {
		if(cla.GetPositionalCount() >= 2) {
			String id = AsString(cla.GetPositional(1));
			String path = WorkSessionManager::FindSessionPath(docs_root, id);
			WorkSession s;
			if(!path.IsEmpty() && WorkSessionManager::LoadSession(path, s)) {
				SessionStats st = WorkSessionManager::CalculateSessionStats(docs_root, s);
				Cout() << "Statistics for " << s.session_id << ":\n"
				       << "  Breadcrumbs:   " << st.total_breadcrumbs << "\n"
			       << "  Tokens:        In: " << st.total_tokens_input << ", Out: " << st.total_tokens_output << "\n"
			       << "  Cost:          $" << Format("%.6f", st.estimated_cost) << "\n"
			       << "  Files Modified: " << st.files_modified << "\n"
			       << "  Tools Called:   " << st.tools_called << "\n"
			       << "  Duration:      " << (int)st.duration_seconds << " seconds\n"
			       << "  Success Rate:  " << Format("%.1f", st.success_rate) << "%\n";
			} else Cerr() << "Error: Session not found.\n";
		} else {
			Array<WorkSession> sessions = WorkSessionManager::ListSessions(docs_root);
			int total_b = 0, total_ti = 0, total_to = 0, total_fm = 0, total_tc = 0;
			double total_cost = 0, total_dur = 0, total_sr = 0;
			for(const auto& s : sessions) {
				SessionStats st = WorkSessionManager::CalculateSessionStats(docs_root, s);
				total_b += st.total_breadcrumbs;
				total_ti += st.total_tokens_input;
				total_to += st.total_tokens_output;
				total_cost += st.estimated_cost;
				total_fm += st.files_modified;
				total_tc += st.tools_called;
				total_dur += st.duration_seconds;
				total_sr += st.success_rate;
			}
			Cout() << "Aggregate Statistics for " << sessions.GetCount() << " Sessions:\n"
			       << "  Total Breadcrumbs: " << total_b << "\n"
			       << "  Total Tokens:      In: " << total_ti << ", Out: " << total_to << "\n"
			       << "  Total Cost:        $" << Format("%.6f", total_cost) << "\n"
			       << "  Files Modified:    " << total_fm << "\n"
			       << "  Tools Called:      " << total_tc << "\n"
			       << "  Total Duration:    " << (int)total_dur << " seconds\n"
			       << "  Avg Success Rate:  " << (sessions.IsEmpty() ? 0 : total_sr / sessions.GetCount()) << "%\n";
		}
	}
	else { Cout() << "Unknown wsession subcommand: " << sub << "\n"; ShowHelp(); }
}

} // namespace Upp
