#include "Maestro.h"
#include "ProjectScaffolder.h"

namespace Upp {

void InitCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI init [-h] [--dir DIR] [--name NAME] [--template TEMPLATE] [--force]\n"
	       << "Initialize a Maestro project with required directories and configuration files.\n"
	       << "options:\n"
	       << "  -h, --help           show this help message and exit\n"
	       << "  --dir DIR            Target directory to initialize (default: current directory)\n"
	       << "  --name NAME          U++ package name (default: directory name)\n"
	       << "  --template TEMPLATE  Project template: gui, accounting, none (default: gui)\n"
	       << "  --force              Force initialization even if Maestro files already exist\n";
}

void InitCommand::Execute(const Vector<String>& args) {
	String dir = GetCurrentDirectory();
	String name;
	String template_name = "gui";
	bool force = false;
	
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--dir" && i + 1 < args.GetCount()) dir = args[++i];
		else if(args[i].StartsWith("--dir=")) dir = args[i].Mid(6);
		else if(args[i] == "--name" && i + 1 < args.GetCount()) name = args[++i];
		else if(args[i].StartsWith("--name=")) name = args[i].Mid(7);
		else if(args[i] == "--template" && i + 1 < args.GetCount()) template_name = args[++i];
		else if(args[i].StartsWith("--template=")) template_name = args[i].Mid(11);
		else if(args[i] == "--force") force = true;
	}
	
	if(name.IsEmpty()) name = GetFileName(dir);
	
	Cout() << "Initializing Maestro project in " << dir << "...\n";
	
	if(template_name != "none") {
		ProjectScaffolder::Scaffold(dir, name, template_name);
	} else {
		ProjectScaffolder::InitMaestro(dir);
	}
	
	Cout() << "\nMaestro project initialized successfully!\n";
}

} // namespace Upp