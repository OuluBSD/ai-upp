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
			return p;
		d = GetFileDirectory(d);
	}
	return "";
}

struct PlanCommand : Command {
	String GetName() const override { return "plan"; }
	String GetDescription() const override { return "Plan management"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI plan [-h] {add,list,ls,remove,show,sh}\n"
		       << "\n"
		       << "Plan subcommands:\n"
		       << "    add (a)             Add a new plan\n"
		       << "    list (ls)           List all plans\n"
		       << "    remove (rm)         Remove a plan\n"
		       << "    show (sh)           Show a plan and its items\n";
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
			Cerr() << "Error: Could not find 'uppsrc/AI/plan' directory.\n";
			return;
		}

		PlanParser parser;
		parser.Load(plan_root);

		String sub = cla.GetPositional(0);
		if (sub == "list" || sub == "ls") {
			Cout() << "Plans (Tracks) found in " << plan_root << ":\n";
			for(const auto& track : parser.tracks) {
				Cout() << Format(" [%-10s] %s (%d phases)\n", 
				                 track.status, track.name, track.phases.GetCount());
			}
		}
		else if (sub == "show" || sub == "sh") {
			if(cla.GetPositionalCount() < 2) {
				Cerr() << "Error: 'plan show' requires a track name.\n";
				return;
			}
			String name = cla.GetPositional(1);
			const Track* found = nullptr;
			for(const auto& track : parser.tracks) {
				if(track.name == name || track.id == name) {
					found = &track;
					break;
				}
			}
			
			if(!found) {
				Cerr() << "Error: Track '" << name << "' not found.\n";
				return;
			}
			
			Cout() << "Track: " << found->name << " [" << found->status << "]\n";
			Cout() << "Path:  " << found->path << "\n\n";
			for(const auto& phase : found->phases) {
				Cout() << "Phase: " << phase.name << " [" << phase.status << "]\n";
				for(int i = 0; i < phase.tasks.GetCount(); i++) {
					const Task& task = phase.tasks[i];
					Cout() << Format("  %d. [%-11s] %s\n", 
					                 i + 1,
					                 StatusToString(task.status),
					                 task.name);
				}
				Cout() << "\n";
			}
		}
		else if (sub == "add" || sub == "a") {
			if(cla.GetPositionalCount() < 2) {
				Cerr() << "Error: 'plan add' requires a track name.\n";
				return;
			}
			String name = cla.GetPositional(1);
			String path = AppendFileName(plan_root, name);
			if(DirectoryExists(path)) {
				Cerr() << "Error: Track '" << name << "' already exists.\n";
				return;
			}
			if(RealizeDirectory(path)) {
				Cout() << "Created track directory: " << path << "\n";
			} else {
				Cerr() << "Error: Failed to create track directory.\n";
			}
		}
		else if (sub == "remove" || sub == "rm") {
			if(cla.GetPositionalCount() < 2) {
				Cerr() << "Error: 'plan remove' requires a track name.\n";
				return;
			}
			String name = cla.GetPositional(1);
			String path = AppendFileName(plan_root, name);
			if(!DirectoryExists(path)) {
				Cerr() << "Error: Track '" << name << "' not found.\n";
				return;
			}
			
			Cout() << "Are you sure you want to remove track '" << name << "'? [y/N]: ";
			String response = ReadStdIn();
			if(ToLower(TrimBoth(response)) == "y") {
				// Use busybox or rm -rf
				String cmd = "rm -rf " + path;
				if(system(cmd) == 0) {
					Cout() << "Removed track: " << name << "\n";
				} else {
					Cerr() << "Error: Failed to remove track.\n";
				}
			}
		}
		else if (sub == "add-item" || sub == "ai") {
			if(cla.GetPositionalCount() < 4) {
				Cerr() << "Error: 'plan add-item' requires <track> <phase> <task_title>.\n";
				return;
			}
			String track = cla.GetPositional(1);
			String phase = cla.GetPositional(2);
			String title = cla.GetPositional(3);
			
			String track_path = AppendFileName(plan_root, track);
			if(!DirectoryExists(track_path)) {
				Cerr() << "Error: Track '" << track << "' not found.\n";
				return;
			}
			
			String phase_path = AppendFileName(track_path, phase);
			if(!DirectoryExists(phase_path)) {
				if(RealizeDirectory(phase_path)) {
					Cout() << "Created phase directory: " << phase_path << "\n";
				} else {
					Cerr() << "Error: Failed to create phase directory.\n";
					return;
				}
			}
			
			String task_file = AppendFileName(phase_path, title + ".md");
			if(FileExists(task_file)) {
				Cerr() << "Error: Task '" << title << "' already exists in phase '" << phase << "'.\n";
				return;
			}
			
			String content = "# Task: " + title + "\n# Status: TODO\n\n## Objective\n" + title + "\n";
			if(SaveFile(task_file, content)) {
				Cout() << "Created task file: " << task_file << "\n";
			} else {
				Cerr() << "Error: Failed to create task file.\n";
			}
		}
		else if (sub == "remove-item" || sub == "ri") {
			if(cla.GetPositionalCount() < 4) {
				Cerr() << "Error: 'plan remove-item' requires <track> <phase> <task_title>.\n";
				return;
			}
			String track = cla.GetPositional(1);
			String phase = cla.GetPositional(2);
			String title = cla.GetPositional(3);
			
			String task_file = AppendFileName(AppendFileName(AppendFileName(plan_root, track), phase), title + ".md");
			if(!FileExists(task_file)) {
				Cerr() << "Error: Task file not found: " << task_file << "\n";
				return;
			}
			
			Cout() << "Are you sure you want to remove task '" << title << "'? [y/N]: ";
			String response = ReadStdIn();
			if(ToLower(TrimBoth(response)) == "y") {
				if(DeleteFile(task_file)) {
					Cout() << "Removed task: " << title << "\n";
				} else {
					Cerr() << "Error: Failed to remove task file.\n";
				}
			}
		}
		else if (sub == "status") {
			if(cla.GetPositionalCount() < 5) {
				Cerr() << "Error: 'plan status' requires <track> <phase> <task> <status>.\n";
				return;
			}
			String track = cla.GetPositional(1);
			String phase = cla.GetPositional(2);
			String task = cla.GetPositional(3);
			String status_str = cla.GetPositional(4);
			
			TaskStatus status = StringToStatus(ToLower(status_str));
			if(status == STATUS_UNKNOWN) {
				Cerr() << "Error: Unknown status '" << status_str << "'. Use: todo, in_progress, done, blocked.\n";
				return;
			}
			
			PlanParser p;
			// plan_root is /common/active/sblo/Dev/ai-upp/uppsrc/AI/plan
			String docs_root = AppendFileName(plan_root, "../../..");
			
			if(p.UpdateTaskStatus(docs_root, track, phase, task, status)) {
				Cout() << "Updated status of '" << task << "' to " << status_str << "\n";
			} else {
				Cerr() << "Error: Failed to update task status.\n";
			}
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
		if(cla.GetPositionalCount() > 0)
			prompt = cla.GetPositional(0);
		else {
			Cout() << "Enter your prompt: ";
			prompt = ReadStdIn();
		}
		
		if(prompt.IsEmpty()) return;

		String plan_root = FindPlanRoot();
		if(plan_root.IsEmpty()) {
			Cerr() << "Error: Could not find 'uppsrc/AI/plan' directory.\n";
			return;
		}

		PlanParser parser;
		parser.Load(plan_root);
		
		// 1. Get Context
		String context = PlanSummarizer::GetPlanSummaryText(parser.tracks, "", "", "");
		String full_prompt = "You are the Maestro AI Assistant. Your goal is to help manage the project plan.\n"
		                     "You have access to the 'update_task_status' tool.\n"
		                     "Do NOT try to use other tools unless explicitly requested.\n\n"
		                     "Project Context:\n" + context + "\n\nUser Question: " + prompt;

		        // 2. Setup Engine
		        CliMaestroEngine engine;
		        engine.binary = "gemini"; // Default
		        engine.Arg("-m").Arg("gemini-1.5-flash");
		        engine.Arg("-y"); // YOLO mode
		        engine.Arg("-o").Arg("stream-json");
		        engine.Arg("-p").Arg(full_prompt); // Pass prompt directly
		        engine.session_id = ""; // Truly empty
		        
		        MaestroToolRegistry tool_reg;
		        RegisterMaestroTools(tool_reg);
		
		        Cout() << "Thinking...\n";
		        
		        bool done = false;
		        engine.Send("", [&](const MaestroEvent& ev) { // Empty prompt here
		            Cout() << "DEBUG: ev.type=" << ev.type << " ev.sid=" << ev.session_id << " ev.text=" << ev.text << "\n";
		            if(ev.type == "message") {				if(ev.role == "assistant" || ev.role.IsEmpty()) {
					if(ev.delta) Cout() << ev.text;
					else Cout() << ev.text << "\n";
				}
			}
			else if(ev.type == "tool_use" || ev.type == "tool_call") {
				Cout() << "\n[AI calls tool: " << ev.tool_name << "(" << ev.tool_input << ")]\n";
				const MaestroTool* tool = tool_reg.Find(ev.tool_name);
				if(tool) {
					Value input = ParseJSON(ev.tool_input);
					if(IsError(input)) input = ValueMap(); 
					
					Value result = tool->Execute(input);
					Cout() << "[Tool Result: " << result << "]\n";
					engine.WriteToolResult(ev.tool_id, result);
				} else {
					engine.WriteToolResult(ev.tool_id, "Error: Tool not found");
				}
			}
			else if (ev.type == "result") {
				if(ev.text.StartsWith("[API Error:"))
					Cerr() << "\nAI " << ev.text << "\n";
				done = true;
			}
			else if (ev.type == "done") {
				done = true;
			}
		});

		while(!done && engine.Do()) {
			Sleep(10);
		}
		Cout() << "\n";
	}
};

void ShowVersion()
{
	Cout() << "Maestro CLI 1.0.0\n";
}

void MainHelp(const ArrayMap<String, Command>& commands)
{
	Cout() << "usage: MaestroCLI [-h] [--version] [-s SESSION] [-v] [-q]\n"
	       << "                  {help";
	for(int i = 0; i < commands.GetCount(); i++)
		Cout() << "," << commands.GetKey(i);
	Cout() << "}\n"
	       << "                  ...\n\n"
	       << "Maestro - AI Task Management CLI (C++ Port)\n\n"
	       << "positional arguments:\n"
	       << "  {help";
	for(int i = 0; i < commands.GetCount(); i++)
		Cout() << "," << commands.GetKey(i);
	Cout() << "}\n"
	       << "                        Available commands\n"
	       << "    help                Show help for all commands\n";
	
	for(int i = 0; i < commands.GetCount(); i++) {
		Cout() << "    " << Format("%-19s", commands.GetKey(i)) << " " << commands[i].GetDescription() << "\n";
	}

	Cout() << "\noptions:\n"
	       << "  -h, --help            show this help message and exit\n"
	       << "  --version             Show version information\n"
	       << "  -s SESSION, --session SESSION\n"
	       << "                        Path to session JSON file\n"
	       << "  -v, --verbose         Show detailed debug information\n"
	       << "  -q, --quiet           Suppress streaming AI output\n";
}

CONSOLE_APP_MAIN
{
	ArrayMap<String, Command> commands;
	commands.Create<PlanCommand>("plan");
	commands.Create<PlanCommand>("pl"); // Aliases can be added like this
	commands.Create<TrackCommand>("track");
	commands.Create<TrackCommand>("tr");
	commands.Create<TrackCommand>("t");
	commands.Create<DiscussCommand>("discuss");
	commands.Create<DiscussCommand>("d");

	CommandLineArguments cla;
	cla.AddArg('v', "Verbose output", false);
	cla.AddArg('q', "Quiet mode", false);
	cla.AddArg('s', "Session file", true, "SESSION");
	cla.AddPositional("command", UNKNOWN_V);
	
	if (!cla.Parse()) {
		MainHelp(commands);
		return;
	}

	if (cla.IsArg('v')) { /* set verbose */ }
	if (cla.IsArg('q')) { /* set quiet */ }
	String session = cla.GetArg('s');

	if (cla.GetPositionalCount() == 0) {
		MainHelp(commands);
		return;
	}

	String cmdName = cla.GetPositional(0);
	if (cmdName == "help") {
		MainHelp(commands);
		return;
	}

	int idx = commands.Find(cmdName);
	if (idx >= 0) {
		commands[idx].Execute(cla.GetRest());
	}
	else {
		Cout() << "Unknown command: " << cmdName << "\n";
		MainHelp(commands);
	}
}