#include "Maestro.h"

namespace Upp {

void SettingsCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI settings [-h]\n"
	       << "                           {list,ls,l,get,g,set,s,edit,e,reset,r,wizard,w,profile,prof,pr,help,h}\n"
	       << "                           ...\n"
	       << "Manage project configuration.\n"
	       << "positional arguments:\n"
	       << "    list (ls)           List all settings\n"
	       << "    get <key>           Get a setting value\n"
	       << "    set <key> <val>     Set a setting value\n";
}

void SettingsCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("key", UNKNOWN_V);
	cla.AddPositional("value", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	SettingsManager sm;
	
	if (sub == "list" || sub == "ls") {
		ValueMap settings = sm.LoadSettings();
		for(int i = 0; i < settings.GetCount(); i++)
			Cout() << Format("  %s: %s\n", AsString(settings.GetKey(i)), AsString(settings.GetValue(i)));
	}
	else if (sub == "get" || sub == "g") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires key.\n"; return; }
		Cout() << AsString(sm.GetSetting(AsString(cla.GetPositional(1)))) << "\n";
	}
	else if (sub == "set" || sub == "s") {
		if(cla.GetPositionalCount() < 3) { Cerr() << "Error: Requires key and value.\n"; return; }
		if(sm.SetSetting(AsString(cla.GetPositional(1)), AsString(cla.GetPositional(2))))
			Cout() << "✓ Updated setting.\n";
		else Cerr() << "Error: Failed to save setting.\n";
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++.\n";
	}
}

} 
