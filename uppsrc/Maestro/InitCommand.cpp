#include "Maestro.h"

namespace Upp {

void InitCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI init [-h] [--dir DIR] [--force]\n"
	       << "Initialize a Maestro project with required directories and configuration files.\n"
	       << "options:\n"
	       << "  -h, --help  show this help message and exit\n"
	       << "  --dir DIR   Target directory to initialize (default: current directory)\n"
	       << "  --force     Force initialization even if Maestro files already exist\n";
}

void InitCommand::Execute(const Vector<String>& args) {
	String dir = GetCurrentDirectory();
	bool force = false;
	
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--dir" && i + 1 < args.GetCount()) dir = args[++i];
		else if(args[i].StartsWith("--dir=")) dir = args[i].Mid(6);
		else if(args[i] == "--force") force = true;
	}
	
	Cout() << "Initializing Maestro project in " << dir << "...\n";
	
	static const char* dirs[] = {
		".maestro", "docs", "docs/maestro", "docs/tracks", "docs/phases",
		"docs/tasks", "docs/sessions", "docs/maestro/log_scans", ".maestro/issues"
	};
	
	for(const char* d : dirs) {
		String p = AppendFileName(dir, d);
		RealizeDirectory(p);
		Cout() << "  Created: " << d << "\n";
	}
	
	String settings_path = AppendFileName(dir, "docs/Settings.md");
	if(!FileExists(settings_path) || force) {
		String content = "# Maestro Settings\n\n## Project Configuration\n- Project name: My Project\n";
		SaveFile(settings_path, content);
		Cout() << "  Created: docs/Settings.md\n";
	}
	
	Cout() << "\nMaestro project initialized successfully!\n";
}

} // namespace Upp