#include <Core/Core.h>
#include <Maestro/Maestro.h>

using namespace Upp;

struct Command {
	virtual ~Command() {}
	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;
	virtual void ShowHelp() const = 0;
	virtual void Execute(const Vector<String>& args) = 0;
};

String FindPlanRoot()
{
	String d = GetCurrentDirectory();
	while(d.GetCount() > 1) {
		String p = AppendFileName(AppendFileName(AppendFileName(d, "uppsrc"), "AI"), "plan");
		if(DirectoryExists(p))
			return NormalizePath(p);
		d = GetFileDirectory(d);
	}
	return "";
}

String GetDocsRoot(const String& plan_root)
{
	if(plan_root.IsEmpty()) return "";
	return NormalizePath(AppendFileName(plan_root, "../../.."));
}

struct PlanCommand : Command {
	String GetName() const override { return "plan"; }
	String GetDescription() const override { return "Plan management"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI plan [-h] {add,list,ls,remove,show,sh,add-item,ai,remove-item,ri,status}\n"
		       << "\n"
		       << "Plan subcommands:\n"
		       << "    add (a)             Add a new plan (track)\n"
		       << "    list (ls)           List all plans\n"
		       << "    remove (rm)         Remove a plan\n"
		       << "    show (sh)           Show a plan and its items\n"
		       << "    add-item (ai)       Add an item to a plan\n"
		       << "    remove-item (ri)    Remove an item from a plan\n"
		       << "    status              Update task status\n";
	}
	
	void Execute(const Vector<String>& args) override {
		CommandLineArguments cla;
		cla.AddPositional("subcommand", UNKNOWN_V);
		cla.AddPositional("arg1", UNKNOWN_V);
		cla.AddPositional("arg2", UNKNOWN_V);
		cla.AddPositional("arg3", UNKNOWN_V);
		cla.AddPositional("arg4", UNKNOWN_V);
		cla.Parse(args);
		
		if (cla.GetPositionalCount() == 0) {
			ShowHelp();
			return;
		}
		
		String plan_root = FindPlanRoot();
		if(plan_root.IsEmpty()) {
			Cerr() << "Error: Could not find project plan directory.\n";
			return;
		}
		String docs_root = GetDocsRoot(plan_root);

		PlanParser parser;
		parser.Load(plan_root);

		String sub = AsString(cla.GetPositional(0));
		int track_idx = -1;
		if(!sub.IsEmpty() && IsDigit(sub[0])) {
			track_idx = StrInt(sub);
			if(cla.GetPositionalCount() > 1) sub = AsString(cla.GetPositional(1));
			else sub = "show";
		}

		if (sub == "list" || sub == "ls") {
			Cout() << "Plans (Tracks) found in " << plan_root << ":\n";
			for(int i = 0; i < parser.tracks.GetCount(); i++) {
				const auto& track = parser.tracks[i];
				Cout() << Format(" %d. [%-10s] %s (%d phases)\n", 
				                 i, track.status, track.name, track.phases.GetCount());
			}
		}
		else if (sub == "show" || sub == "sh") {
			const Track* found = nullptr;
			if(track_idx >= 0) {
				if(track_idx < parser.tracks.GetCount()) found = &parser.tracks[track_idx];
				else { Cerr() << "Error: Track index out of range.\n"; return; }
			} else {
				if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires track name or index.\n"; return; }
				String name = AsString(cla.GetPositional(1));
				for(const auto& track : parser.tracks) {
					if(track.name == name || track.id == name) { found = &track; break; }
				}
			}
			if(!found) { Cerr() << "Error: Track not found.\n"; return; }
			
			Cout() << "Track: " << found->name << " [" << found->status << "]\n";
			Cout() << "Path:  " << found->path << "\n\n";
			for(const auto& phase : found->phases) {
				Cout() << "Phase: " << phase.name << " [" << phase.status << "]\n";
				for(int i = 0; i < phase.tasks.GetCount(); i++) {
					const Task& task = phase.tasks[i];
					Cout() << Format("  %d. [%-11s] %s\n", i + 1, StatusToString(task.status), task.name);
				}
				Cout() << "\n";
			}
		}
		else if (sub == "add" || sub == "a") {
			if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires track name.\n"; return; }
			String name = AsString(cla.GetPositional(1));
			String path = AppendFileName(plan_root, name);
			if(DirectoryExists(path)) { Cerr() << "Error: Track already exists.\n"; return; }
			if(RealizeDirectory(path)) Cout() << "Created track directory: " << path << "\n";
			else Cerr() << "Error: Failed to create track directory.\n";
		}
		else if (sub == "add-item" || sub == "ai") {
			if(cla.GetPositionalCount() < 4) { Cerr() << "Error: Requires <track> <phase> <task_title>.\n"; return; }
			String track = AsString(cla.GetPositional(1));
			String phase = AsString(cla.GetPositional(2));
			String title = AsString(cla.GetPositional(3));
			String track_path = AppendFileName(plan_root, track);
			if(!DirectoryExists(track_path)) { Cerr() << "Error: Track not found.\n"; return; }
			String phase_path = AppendFileName(track_path, phase);
			RealizeDirectory(phase_path);
			String task_file = AppendFileName(phase_path, title + ".md");
			if(FileExists(task_file)) { Cerr() << "Error: Task already exists.\n"; return; }
			String content = "# Task: " + title + "\n# Status: TODO\n\n## Objective\n" + title + "\n";
			if(SaveFile(task_file, content)) Cout() << "Created task file: " << task_file << "\n";
			else Cerr() << "Error: Failed to create task file.\n";
		}
		else if (sub == "status") {
			if(cla.GetPositionalCount() < 5) { Cerr() << "Error: Requires <track> <phase> <task> <status>.\n"; return; }
			String track = AsString(cla.GetPositional(1));
			String phase = AsString(cla.GetPositional(2));
			String task = AsString(cla.GetPositional(3));
			String status_str = AsString(cla.GetPositional(4));
			TaskStatus status = StringToStatus(ToLower(status_str));
			if(status == STATUS_UNKNOWN) { Cerr() << "Error: Unknown status '" << status_str << "'.\n"; return; }
			PlanParser p;
			if(p.UpdateTaskStatus(docs_root, track, phase, task, status)) Cout() << "Updated status of '" << task << "' to " << status_str << "\n";
			else Cerr() << "Error: Failed to update task status.\n";
				}
				else {
					Cout() << "Unknown plan subcommand: " << sub << "\n";
					ShowHelp();
				}
			}
		};
		
		struct TrackCommand : Command {
			String GetName() const override { return "track"; }
			String GetDescription() const override { return "Manage project tracks"; }
			
			void ShowHelp() const override {
				Cout() << "usage: MaestroCLI track [-h] {list,add,remove}\n";
			}
			
			void Execute(const Vector<String>& args) override {
				Cout() << "Track command not yet fully implemented\n";
			}
		};
		
		struct DiscussCommand : Command {
	String GetName() const override { return "discuss"; }
	String GetDescription() const override { return "Start an AI discussion using the current context"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI discuss [-h] [prompt]\n";
	}
	
	void Execute(const Vector<String>& args) override {
		CommandLineArguments cla;
		cla.AddPositional("prompt", UNKNOWN_V);
		cla.Parse(args);
		
		String prompt;
		if(cla.GetPositionalCount() > 0) prompt = AsString(cla.GetPositional(0));
		else { Cout() << "Enter your prompt: "; prompt = ReadStdIn(); }
		if(prompt.IsEmpty()) return;

		String plan_root = FindPlanRoot();
		if(plan_root.IsEmpty()) { Cerr() << "Error: Could not find plan root.\n"; return; }
		String docs_root = GetDocsRoot(plan_root);
		
		PlanParser parser;
		parser.Load(plan_root);
		
		String context = PlanSummarizer::GetPlanSummaryText(parser.tracks, "", "", "");
		String full_prompt = "You are the Maestro AI Assistant. Your goal is to help manage the project plan.\n"
		                     "You have access to the 'update_task_status' tool.\n"
		                     "Do NOT try to use other tools unless explicitly requested.\n\n"
		                     "Project Context:\n" + context + "\n\nUser Question: " + prompt;

		CliMaestroEngine engine;
		engine.binary = "gemini";
		engine.Arg("-m").Arg("gemini-1.5-flash");
		engine.Arg("-y"); 
		engine.Arg("-o").Arg("stream-json");
		engine.session_id = ""; 
		
		MaestroToolRegistry tool_reg;
		RegisterMaestroTools(tool_reg);

		Cout() << "Thinking...\n";
		Breadcrumb bc;
		bc.prompt = full_prompt;
		bc.model_used = "gemini-1.5-flash";
		
		bool done = false;
		engine.Send(full_prompt, [&](const MaestroEvent& ev) {
			if(ev.type == "message") {
				if(ev.role == "assistant" || ev.role.IsEmpty()) {
					if(ev.delta) { Cout() << ev.text; bc.response << ev.text; }
					else { Cout() << ev.text << "\n"; bc.response = ev.text; }
				}
			}
			else if(ev.type == "tool_use" || ev.type == "tool_call") {
				Cout() << "\n[AI calls tool: " << ev.tool_name << "(" << ev.tool_input << ")]\n";
				ToolCall& tc = bc.tools_called.Add();
				tc.tool = ev.tool_name;
				tc.timestamp = GetSysTime();
				const MaestroTool* tool = tool_reg.Find(ev.tool_name);
				if(tool) {
					Value input = ParseJSON(ev.tool_input);
					if(IsError(input)) input = ValueMap(); 
					tc.args = input;
					Value result = tool->Execute(input);
					tc.result = result;
					Cout() << "[Tool Result: " << result << "]\n";
					engine.WriteToolResult(ev.tool_id, result);
				} else {
					tc.error = "Tool not found";
					engine.WriteToolResult(ev.tool_id, "Error: Tool not found");
				}
			}
			else if (ev.type == "result") {
				if(ev.text.StartsWith("[API Error:")) { Cerr() << "\nAI " << ev.text << "\n"; bc.error = ev.text; }
				done = true;
			}
			else if (ev.type == "done") done = true;
		});

		while(!done && engine.Do()) Sleep(10);
		Cout() << "\n";
		
		String sid = engine.session_id;
		if(sid.IsEmpty()) {
			WorkSession s = WorkSessionManager::CreateSession(docs_root, "discussion", "MaestroCLI Discussion");
			sid = s.session_id;
		}
		BreadcrumbManager::SaveBreadcrumb(bc, docs_root, sid);
		Cout() << "[Interaction saved to session " << sid << "]\n";
	}
};

struct WSessionCommand : Command {
	String GetName() const override { return "wsession"; }
	String GetDescription() const override { return "Work session management"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI wsession [-h] {list,ls,show,sh,create,breadcrumbs,timeline}\n"
		       << "\n"
		       << "Work session subcommands:\n"
		       << "    list (ls)           List all work sessions\n"
		       << "    show (sh) <id>      Show work session details\n"
		       << "    create <type> [purp] Create a new work session\n"
		       << "    breadcrumbs <id>    Show breadcrumbs for a session\n"
		       << "    timeline <id>       Show session timeline\n";
	}
	
	void Execute(const Vector<String>& args) override {
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
		else { Cout() << "Unknown wsession subcommand: " << sub << "\n"; ShowHelp(); }
	}
};

void ShowVersion() { Cout() << "Maestro CLI 1.0.0\n"; }

void MainHelp(const ArrayMap<String, Command>& commands)
{
	Cout() << "usage: MaestroCLI [-h] [--version] [-s SESSION] [-v] [-q]\n"
	       << "                  {help";
	for(int i = 0; i < commands.GetCount(); i++) Cout() << "," << commands.GetKey(i);
	Cout() << "}\n                  ...\n\nMaestro - AI Task Management CLI (C++ Port)\n\npositional arguments:\n  {help";
	for(int i = 0; i < commands.GetCount(); i++) Cout() << "," << commands.GetKey(i);
	Cout() << "}\n                        Available commands\n    help                Show help for all commands\n";
	for(int i = 0; i < commands.GetCount(); i++) Cout() << "    " << Format("% -19s", commands.GetKey(i)) << " " << commands[i].GetDescription() << "\n";
	Cout() << "\noptions:\n  -h, --help            show this help message and exit\n  --version             Show version information\n  -s SESSION, --session SESSION\n                        Path to session JSON file\n  -v, --verbose         Show detailed debug information\n  -q, --quiet           Suppress streaming AI output\n";
}

CONSOLE_APP_MAIN
{
	ArrayMap<String, Command> commands;
	commands.Create<PlanCommand>("plan");
	commands.Create<PlanCommand>("pl");
	commands.Create<TrackCommand>("track");
	commands.Create<TrackCommand>("tr");
	commands.Create<TrackCommand>("t");
	commands.Create<DiscussCommand>("discuss");
	commands.Create<DiscussCommand>("d");
	commands.Create<WSessionCommand>("wsession");
	commands.Create<WSessionCommand>("ws");

	CommandLineArguments cla;
	cla.AddArg('v', "Verbose output", false);
	cla.AddArg('q', "Quiet mode", false);
	cla.AddArg('s', "Session file", true, "SESSION");
	cla.AddPositional("command", UNKNOWN_V);
	
	if (!cla.Parse()) { MainHelp(commands); return; }
	if (cla.GetPositionalCount() == 0) { MainHelp(commands); return; }

	String cmdName = AsString(cla.GetPositional(0));
	if (cmdName == "help") { MainHelp(commands); return; }
	if (cmdName == "--version") { ShowVersion(); return; }

	int idx = commands.Find(cmdName);
	if (idx >= 0) commands[idx].Execute(cla.GetRest());
	else { Cout() << "Unknown command: " << cmdName << "\n"; MainHelp(commands); }
}
