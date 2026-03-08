#include <Core/Core.h>
#include <Maestro/Maestro.h>
#include <Maestro/CommandDispatcher.h>
#include <ByteVM/ByteVM.h>
#ifdef flagGUI
#include <AI/LogicGui/LogicGui.h>
#endif

using namespace Upp;

void RegisterMaestroModule(PyVM& vm);
void ShowVersion() { Cout() << "Maestro CLI 1.0.0\n"; } 

CONSOLE_APP_MAIN
{
#ifdef flagGUI
	LinkLogicGui();
#endif
	CommandDispatcher& d = CommandDispatcher::Get();
	RegisterAllMaestroCommands(d);
	
	// Add evidence command (local impl for now or move to Maestro?)
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
	d.Register<EvidenceCommandImpl>();

#ifdef flagGUI
	d.Register<TestCommand>();
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
	if (cmdName.IsEmpty()) { d.MainHelp(); return; }

	if(cmdName == "help" || cmdName == "h") { 
		if(sub_args.GetCount() > 0) {
			String subHelp = sub_args[0];
			Command* c = d.FindCommand(subHelp);
			if(c) { c->ShowHelp(); return; }
		}
		d.MainHelp(); 
		return; 
	}
	
	d.Execute(cmdName, sub_args);
}
