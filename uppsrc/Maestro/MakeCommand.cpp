#include "Maestro.h"

namespace Upp {

void MakeCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI make [-h] {build,rebuild,clean,analyze,config,methods,export,android,jar,structure,run}\n"
	       << "\n"
	       << "Make subcommands:\n"
	       << "    build <pkg>         Build a package using umk\n"
	       << "    rebuild <pkg>       Clean and build a package\n"
	       << "    clean <pkg>         Clean build artifacts\n"
	       << "    analyze <pkg>       Analyze build dependencies and structure\n"
	       << "    config              Manage build configuration\n"
	       << "    methods             List available build methods\n"
	       << "    export              Export build configuration\n"
	       << "    android <pkg>       Build for Android platform\n"
	       << "    jar <pkg>           Build JAR package\n"
	       << "    structure <pkg>     Show package build structure\n"
	       << "    run <pkg>           Run built executable\n";
}

void MakeCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("pkg", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() < 2) {
		ShowHelp();
		return;
	}
	
	String sub = AsString(cla.GetPositional(0));
	String pkg = AsString(cla.GetPositional(1));
	
	String plan_root = FindPlanRoot();
	String docs_root = GetDocsRoot(plan_root);

	if (sub == "build" || sub == "rebuild") {
		if(sub == "rebuild") {
			Cout() << "Cleaning " << pkg << "...\n";
			system("umk uppsrc,./uppsrc " + pkg + " GCC -clean");
		}
		Cout() << "Building " << pkg << "...\n";
		int res = system("umk uppsrc,./uppsrc " + pkg + " GCC -bm -s");
		if(res == 0) Cout() << "Build successful.\n";
		else Cerr() << "Build failed with exit code " << res << "\n";
	}
	else if (sub == "clean") {
		Cout() << "Cleaning " << pkg << "...\n";
		system("umk uppsrc,./uppsrc " + pkg + " GCC -clean");
	}
	else if (sub == "run") {
		Cout() << "Running " << pkg << "...\n";
		// Very basic run stub: find executable in common U++ path
		String exe = AppendFileName(AppendFileName(GetHomeDirectory(), ".cache/upp.out"), pkg);
		// This is complex to get right without knowing build flags, but for now:
		Cout() << "Subcommand 'run' is partially implemented. Path lookup is required.\n";
	}
	else if (sub == "analyze" || sub == "config" || sub == "methods" || sub == "export" || sub == "android" || sub == "jar" || sub == "structure") {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
	else {
		Cout() << "Unknown make subcommand: " << sub << "\n";
		ShowHelp();
	}
}

}
