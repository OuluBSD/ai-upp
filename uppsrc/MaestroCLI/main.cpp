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
		String p = AppendFileName(d, "uppsrc/AI/plan");
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
		cla.AddPositional("name", UNKNOWN_V);
		
		// Note: cla.Parse() might return false if name is missing but it's expected.
		// We'll manually check GetPositionalCount instead.
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
				/*for(const auto& phase : track.phases) {
					Cout() << Format("   - %-30s [%s]\n", phase.name, phase.status);
				}*/
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