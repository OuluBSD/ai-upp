#include "CommandDispatcher.h"

namespace Upp {

CommandDispatcher& CommandDispatcher::Get() {
	static CommandDispatcher d;
	return d;
}

Command* CommandDispatcher::FindCommand(const String& name) {
	for(int i = 0; i < commands.GetCount(); i++) {
		Command& cmd = commands[i];
		if(cmd.GetName() == name) return &cmd;
		for(const auto& a : cmd.GetAliases()) if(a == name) return &cmd;
	}
	return nullptr;
}

void CommandDispatcher::Execute(const String& cmdName, const Vector<String>& args) {
	Command* found = FindCommand(cmdName);
	if(found) {
		bool show_help = false;
		for(const auto& a : args) if(a == "--help" || a == "-h") show_help = true;
		
		if(show_help) found->ShowHelp();
		else found->Execute(args);
	}
	else {
		Cout() << "Unknown command: " << cmdName << "\n";
		MainHelp();
	}
}

void CommandDispatcher::MainHelp() {
	Cout() << "Maestro - AI Task Management CLI\n\n"
	       << "Available commands:\n";
	for(int i = 0; i < commands.GetCount(); i++) {
		const auto& cmd = commands[i];
		String names = cmd.GetName();
		const Vector<String>& al = cmd.GetAliases();
		if(al.GetCount() > 0) names << " (" << Join(al, ", ") << ")";
		Cout() << "    " << Format("% -19s", names) << " " << cmd.GetDescription() << "\n";
	}
}

void RegisterAllMaestroCommands(CommandDispatcher& d) {
	d.Register<InitCommand>();
	d.Register<RunbookCommand>();
	d.Register<WorkflowCommand>();
	d.Register<RepoCommand>();
	d.Register<PlanCommand>();
	d.Register<MakeCommand>();
	d.Register<LogCommand>();
	d.Register<CacheCommand>();
	d.Register<TrackCacheCommand>();
	d.Register<OpsCommand>();
	d.Register<TrackCommand>();
	d.Register<PhaseCommand>();
	d.Register<TaskCommand>();
	d.Register<DiscussCommand>();
	d.Register<SettingsCommand>();
	d.Register<IssuesCommand>();
	d.Register<SolutionsCommand>();
	d.Register<AiCommand>();
	d.Register<WorkCommand>();
	d.Register<WSessionCommand>();
	d.Register<TuCommand>();
	d.Register<ConvertCommand>();
	d.Register<UxCommand>();
	d.Register<TutorialCommand>();
	d.Register<AutomationCommand>();
}

}