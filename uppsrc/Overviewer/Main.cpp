#include "Overviewer.h"

void PrintHelp() {
	Cout() << "Overviewer CLI Mode\n"
	       << "Usage:\n"
	       << "  Overviewer [--help]\n"
	       << "  Overviewer --create-project <file> [--dir <working_dir>]\n"
	       << "  Overviewer --open-project <file>\n"
	       << "  Overviewer --roundtrip-project <input> <output>\n"
	       << "  Overviewer --set-flag <project> <path> <flag>\n"
	       << "  Overviewer --set-priority <project> <path> <value>\n"
	       << "  Overviewer --get-entry <project> <path>\n"
	       << "\nFlags: TEMPORARY, WRONG_LOCATION, WRONG_NAME, TOO_LARGE, NEEDS_REVIEW, CONTENT_NEEDS_REVIEW\n";
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
			Cout() << "Entries: " << p.metadata.GetCount() << "\n";
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

	if (args[0] == "--set-flag" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		String flag_name = args[3];
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		uint32 bit = 0;
		if(flag_name == "TEMPORARY") bit = FLAG_TEMPORARY;
		else if(flag_name == "WRONG_LOCATION") bit = FLAG_WRONG_LOCATION;
		else if(flag_name == "WRONG_NAME") bit = FLAG_WRONG_NAME;
		else if(flag_name == "TOO_LARGE") bit = FLAG_TOO_LARGE;
		else if(flag_name == "NEEDS_REVIEW") bit = FLAG_NEEDS_REVIEW;
		else if(flag_name == "CONTENT_NEEDS_REVIEW") bit = FLAG_CONTENT_NEEDS_REVIEW;
		else {
			Cerr() << "Invalid flag name: " << flag_name << "\n";
			return 1;
		}
		
		p.metadata.GetAdd(f_path).flags |= bit;
		if(StoreAsJsonFile(p, p_path)) {
			Cout() << "Flag " << flag_name << " set for " << f_path << "\n";
			return 0;
		}
		return 1;
	}

	if (args[0] == "--set-priority" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		int val = ScanInt(args[3]);
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		p.metadata.GetAdd(f_path).priority = val;
		if(StoreAsJsonFile(p, p_path)) {
			Cout() << "Priority set to " << val << " for " << f_path << "\n";
			return 0;
		}
		return 1;
	}

	if (args[0] == "--get-entry" && args.GetCount() >= 3) {
		String p_path = args[1];
		String f_path = args[2];
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		const FileMetadata* m = p.metadata.FindPtr(f_path);
		if(m) {
			Cout() << "Path: " << f_path << "\n";
			Cout() << "Flags: " << (int)m->flags << "\n";
			Cout() << "Priority: " << m->priority << "\n";
			Cout() << "Quality: " << m->quality << "\n";
			Cout() << "Completion: " << m->completion << "\n";
			return 0;
		} else {
			Cerr() << "No metadata for path: " << f_path << "\n";
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
