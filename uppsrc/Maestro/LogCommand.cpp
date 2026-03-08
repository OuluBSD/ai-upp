#include "Maestro.h"

namespace Upp {

void LogCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI log [-h] {scan,list,ls,show,sh} ...\n"
	       << "positional arguments:\n"
	       << "    scan                Scan build/run output or log files for errors and warnings\n"
	       << "    list (ls)           List all log scans\n"
	       << "    show (sh)           Show scan details and findings\n";
}

void LogCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	LogManager lm;
	
	if (sub == "scan") {
		String target = cla.GetPositionalCount() > 1 ? AsString(cla.GetPositional(1)) : "";
		// If target is empty, maybe read from stdin? 
		// For now require target file
		if(target.IsEmpty()) { Cerr() << "Error: Requires log file path.\n"; return; }
		
		String id = lm.CreateScan(target, "");
		Cout() << "✓ Scan complete. ID: " << id << "\n";
	}
	else if (sub == "list" || sub == "ls") {
		Array<LogScanMeta> list = lm.ListScans();
		Cout() << "Found " << list.GetCount() << " scan(s):\n";
		for(const auto& m : list)
			Cout() << Format("  %-20s %d findings\n", m.scan_id, m.finding_count);
	}
	else if (sub == "show" || sub == "sh") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires scan ID.\n"; return; }
		LogScan scan = lm.LoadScan(AsString(cla.GetPositional(1)));
		Cout() << "Scan: " << scan.meta.scan_id << "\n";
		Cout() << "Findings: " << scan.findings.GetCount() << "\n";
		for(const auto& f : scan.findings)
			Cout() << "  [" << f.severity << "] " << f.message << "\n";
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++.\n";
	}
}

}    
