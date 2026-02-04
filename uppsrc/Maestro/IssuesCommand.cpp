#include "Maestro.h"

namespace Upp {

void IssuesCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI issues [-h]\n"
	       << "                         {list,ls,show,state,triage,resolve,ignore,add}\n"
	       << "                         ...\n"
	       << "Manage project issues and triage.\n"
	       << "positional arguments:\n"
	       << "    list (ls)           List issues\n"
	       << "    show <id>           Show issue details\n"
	       << "    state <id> <st>     Update issue state\n"
	       << "    triage [id]         Triage issues\n"
	       << "    resolve <id>        Mark issue as resolved\n"
	       << "    add --manual        Add issue manually\n";
}

void IssuesCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	IssueManager ism;
	
	if (sub == "list" || sub == "ls") {
		Array<MaestroIssue> list = ism.ListIssues();
		Cout() << "Found " << list.GetCount() << " issue(s):\n";
		for(const auto& iss : list)
			Cout() << Format("  %-15s [%-10s] %s\n", iss.issue_id, iss.state, iss.message.IsEmpty() ? iss.title : iss.message);
	}
	else if (sub == "show") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires issue ID.\n"; return; }
		MaestroIssue iss = ism.LoadIssue(AsString(cla.GetPositional(1)));
		Cout() << "Issue:    " << iss.issue_id << "\n"
		       << "Status:   " << iss.state << "\n"
		       << "Severity: " << iss.severity << "\n"
		       << "Message:  " << iss.message << "\n"
		       << "File:     " << iss.file << ":" << iss.line << "\n";
	}
	else if (sub == "state") {
		if(cla.GetPositionalCount() < 3) { Cerr() << "Error: Requires <id> <state>.\n"; return; }
		MaestroIssue iss = ism.LoadIssue(AsString(cla.GetPositional(1)));
		iss.state = AsString(cla.GetPositional(2));
		if(ism.SaveIssue(iss)) Cout() << "✓ Updated issue state.\n";
		else Cerr() << "Error: Failed to save issue.\n";
	}
	else if (sub == "resolve") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires issue ID.\n"; return; }
		MaestroIssue iss = ism.LoadIssue(AsString(cla.GetPositional(1)));
		iss.state = "resolved";
		if(ism.SaveIssue(iss)) Cout() << "✓ Issue resolved.\n";
		else Cerr() << "Error: Failed to resolve issue.\n";
	}
	else if (sub == "triage") {
		if(cla.GetPositionalCount() >= 2) {
			if(ism.Triage(AsString(cla.GetPositional(1)))) Cout() << "✓ Issue triaged.\n";
			else Cerr() << "Error: Failed to triage issue.\n";
		} else {
			Cout() << "Triaging all open issues...\n";
			Array<MaestroIssue> list = ism.ListIssues("", "", "open");
			for(const auto& iss : list) ism.Triage(iss.issue_id);
			Cout() << "✓ Done.\n";
		}
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
}

} // namespace Upp