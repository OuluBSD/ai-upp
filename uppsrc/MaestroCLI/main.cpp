#include <Core/Core.h>
#include <Maestro/Maestro.h>

using namespace Upp;

void ShowVersion() { Cout() << "Maestro CLI 1.0.0\n"; } 

void MainHelp(const Array<Command>& commands)
{
	Cout() << "usage: MaestroCLI [-h] [--version] [-s SESSION] [-v] [--validate-cache] [-q]\n"
	       << "                  {help";
	for(int i = 0; i < commands.GetCount(); i++) {
		const auto& cmd = commands[i];
		Cout() << "," << cmd.GetName();
		for(const auto& a : cmd.GetAliases()) Cout() << "," << a;
	}
	Cout() << "}\n                  ...\n\nMaestro - AI Task Management CLI (C++ Port)\n\n"
	       << "Short aliases are available for all commands and subcommands.\n"
	       << "Examples: 'maestro b p' (build plan), 'maestro s l' (session list),\n"
	       << "          'maestro p tr' (plan tree), 'maestro t l' (track list)\n\n"
	       << "positional arguments:\n"
	       << "  {...}                 Available commands\n"
	       << "    help (h)            Show help for all commands\n";
	
	for(int i = 0; i < commands.GetCount(); i++) {
		const auto& cmd = commands[i];
		String names = cmd.GetName();
		const Vector<String>& al = cmd.GetAliases();
		if(al.GetCount() > 0) {
			names << " (" << Join(al, ", ") << ")";
		}
		Cout() << "    " << Format("% -19s", names) << " " << cmd.GetDescription() << "\n";
	}
	
	Cout() << "\noptions:\n"
	       << "  -h, --help            show this help message and exit\n"
	       << "  --version             Show version information\n"
	       << "  -s SESSION, --session SESSION\n"
	       << "                        Path to session JSON file (required for most commands)\n"
	       << "  -v, --verbose         Show detailed debug information\n"
	       << "  --validate-cache      Validate the track/phases/task cache before reuse\n"
	       << "  -q, --quiet           Suppress streaming AI output\n";
}

CONSOLE_APP_MAIN
{
	Array<Command> commands;
	commands.Create<InitCommand>();
	commands.Create<RunbookCommand>();
	commands.Create<WorkflowCommand>();
	commands.Create<RepoCommand>();
	commands.Create<PlanCommand>();
	commands.Create<MakeCommand>();
	commands.Create<LogCommand>();
	commands.Create<CacheCommand>();
	commands.Create<TrackCacheCommand>();
	commands.Create<OpsCommand>();
	commands.Create<TrackCommand>();
	commands.Create<PhaseCommand>();
	commands.Create<TaskCommand>();
	commands.Create<DiscussCommand>();
	commands.Create<SettingsCommand>();
	commands.Create<IssuesCommand>();
	commands.Create<SolutionsCommand>();
	commands.Create<AiCommand>();
	commands.Create<WorkCommand>();
	commands.Create<WSessionCommand>();
	commands.Create<TuCommand>();
	commands.Create<ConvertCommand>();
	
	struct EvidenceCommandImpl : Command {
		String GetName() const override { return "evidence"; }
		Vector<String> GetAliases() const override { return {"ev"}; }
		String GetDescription() const override { return "Collect repository evidence packs"; }
		void ShowHelp() const override { Cout() << "usage: MaestroCLI evidence <root> <out_json>\n"; }
		void Execute(const Vector<String>& args) override {
			if(args.GetCount() < 2) { ShowHelp(); return; }
			EvidenceCollector collector(args[0]);
			EvidencePack pack = collector.CollectAll();
			if(StoreAsJsonFile(pack, args[1], true))
				Cout() << "✓ Evidence pack saved to " << args[1] << " (" << pack.meta.evidence_count << " items)\n";
			else Cerr() << "Error: Failed to save evidence pack.\n";
		}
	};
	commands.Create<EvidenceCommandImpl>();
	commands.Create<UxCommand>();
	commands.Create<TutorialCommand>();
#ifdef flagGUI
	commands.Create<TestCommand>();
#endif

	const Vector<String>& raw_args = CommandLine();
	bool show_help = false;
	bool show_version = false;
	String session_file;
	bool verbose = false;
	bool quiet = false;
	
	String cmdName;
	Vector<String> sub_args;
	
	for(int i = 0; i < raw_args.GetCount(); i++) {
		String a = raw_args[i];
		if(cmdName.IsEmpty()) {
			if(a == "--help" || a == "-h") show_help = true;
			else if(a == "--version") show_version = true;
			else if(a == "-v" || a == "--verbose") verbose = true;
			else if(a == "-q" || a == "--quiet") quiet = true;
			else if((a == "-s" || a == "--session") && i + 1 < raw_args.GetCount()) session_file = raw_args[++i];
			else if(!a.StartsWith("-")) cmdName = a;
		} else {
			sub_args.Add(a);
		}
	}

	if (show_version) { ShowVersion(); return; }
	if (cmdName.IsEmpty()) { MainHelp(commands); return; }

	// Handle explicit "help" command
	if(cmdName == "help" || cmdName == "h") { 
		if(sub_args.GetCount() > 0) {
			String subHelp = sub_args[0];
			for(int i = 0; i < commands.GetCount(); i++) {
				if(commands[i].GetName() == subHelp) {
					commands[i].ShowHelp();
					return;
				}
			}
		}
		MainHelp(commands); 
		return; 
	}
	
	Command* found = nullptr;
	for(int i = 0; i < commands.GetCount(); i++) {
		Command& cmd = commands[i];
		if(cmd.GetName() == cmdName) { found = &cmd; break; }
		for(const auto& a : cmd.GetAliases()) if(a == cmdName) { found = &cmd; break; }
		if(found) break;
	}

	if (found) {
		// Check if any sub_args contain help flags
		for(const auto& a : sub_args) if(a == "--help" || a == "-h") show_help = true;
		
		if (show_help) found->ShowHelp();
		else found->Execute(sub_args);
	}
	else { 
		Cout() << "Unknown command: " << cmdName << "\n"; 
		MainHelp(commands); 
	}
}
