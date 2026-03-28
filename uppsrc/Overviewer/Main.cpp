#include "Overviewer.h"

void PrintHelp() {
	Cout() << "Overviewer CLI Mode\n"
	       << "Usage:\n"
	       << "  Overviewer [--help]\n"
	       << "  Overviewer --create-project <file> [--dir <working_dir>]\n"
	       << "  Overviewer --open-project <file>\n"
	       << "  Overviewer --roundtrip-project <input> <output>\n";
}

int CliMain(const Vector<String>& args) {
	if (args.GetCount() == 0 || args[0] == "--help") {
		PrintHelp();
		return 0;
	}

	if (args[0] == "--create-project" && args.GetCount() >= 2) {
		OverviewerProject p;
		p.path = args[1];
		if (args.GetCount() >= 4 && args[2] == "--dir")
			p.working_dir = args[3];
		String json = StoreAsJson(p);
		if (SaveFile(p.path, json)) {
			Cout() << "Project created: " << p.path << "\n";
			return 0;
		}
		Cerr() << "Failed to create project: " << p.path << "\n";
		return 1;
	}

	if (args[0] == "--open-project" && args.GetCount() >= 2) {
		String content = LoadFile(args[1]);
		if (content.IsEmpty()) {
			Cerr() << "Failed to load project: " << args[1] << "\n";
			return 1;
		}
		OverviewerProject p;
		try {
			LoadFromJson(p, content);
			Cout() << "Project loaded: " << args[1] << "\n";
			Cout() << "Working directory: " << p.working_dir << "\n";
			return 0;
		} catch (const Exc& e) {
			Cerr() << "Failed to parse project: " << e << "\n";
			return 1;
		}
	}

	if (args[0] == "--roundtrip-project" && args.GetCount() >= 3) {
		String content = LoadFile(args[1]);
		if (content.IsEmpty()) {
			Cerr() << "Failed to load input project: " << args[1] << "\n";
			return 1;
		}
		OverviewerProject p;
		try {
			LoadFromJson(p, content);
			String json = StoreAsJson(p);
			if (SaveFile(args[2], json)) {
				Cout() << "Project roundtripped to: " << args[2] << "\n";
				return 0;
			}
			Cerr() << "Failed to save output project: " << args[2] << "\n";
			return 1;
		} catch (const Exc& e) {
			Cerr() << "Failed to parse input project: " << e << "\n";
			return 1;
		}
	}

	Cerr() << "Unknown arguments. Use --help for usage.\n";
	return 1;
}

GUI_APP_MAIN {
	const Vector<String>& args = CommandLine();
	if (args.GetCount() > 0 && args[0].StartsWith("--")) {
		SetExitCode(CliMain(args));
		return;
	}

	OverviewerWindow().Run();
}
