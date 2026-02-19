#include "CtrlCore.h"

namespace Upp {

#define LLOG(x) DLOG(x)

Vector<String> Ctrl::constraints;
Event<>        Ctrl::WhenCheckConstraints;
int            Ctrl::ugui_verbosity = 1;

void Ctrl::SetUGUIVerbosity(int level)
{
	ugui_verbosity = max(0, level);
}

int Ctrl::GetUGUIVerbosity()
{
	return ugui_verbosity;
}

void Ctrl::InitUGUI()
{
	String title = GetExeTitle();
	String home = GetHomeDirectory();
	String log_dir = AppendFileName(home, ".local/state/u++/guilog");
	RealizeDirectory(log_dir);
	String log_path = AppendFileName(log_dir, title + ".log");
	
	String ugui_path;
	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++) {
		if(cmd[i] == "--ugui" && i + 1 < cmd.GetCount()) {
			ugui_path = cmd[i + 1];
			break;
		}
	}
	int ugui_verbosity = GetCommandLineArgInt("--ugui-verbosity", INT_MIN);
	if(ugui_verbosity == INT_MIN)
		ugui_verbosity = GetCommandLineArgInt("-uv", GetUGUIVerbosity());
	SetUGUIVerbosity(ugui_verbosity);
	
	if(ugui_path.IsEmpty()) {
		String exe = GetExeFilePath();
		String base = GetFileTitle(exe);
		ugui_path = AppendFileName(GetFileDirectory(exe), base + ".ugui");
		if(!FileExists(ugui_path)) {
			ugui_path = base + ".ugui"; // check CWD
		}
	}
	
	if(FileExists(ugui_path)) {
		FileAppend log(log_path);
		if(GetUGUIVerbosity() >= 1)
			log << "[" << GetSysTime() << "] Loading UGUI: " << ugui_path << "\n";
		
		String content = LoadFile(ugui_path);
		if(content.IsVoid()) {
			log << "Error: Could not read UGUI file\n";
		}
		else {
			StringStream ss(content);
			bool in_constraints = false;
			while(!ss.IsEof()) {
				String line = ss.GetLine();
				String trimmed = TrimBoth(line);
				if (trimmed == "constraints:") {
					in_constraints = true;
					continue;
				}
				if (in_constraints) {
					if (trimmed.IsEmpty()) continue;
					if (line.GetCount() > 0 && !IsSpace(line[0]) && trimmed.EndsWith(":")) {
						in_constraints = false; // new block
						continue;
					}
					
					if (trimmed.StartsWith("-")) {
						int q0 = trimmed.Find('"');
						int q1 = trimmed.ReverseFind('"');
						if(q0 >= 0 && q1 > q0) {
							String c = trimmed.Mid(q0 + 1, q1 - q0 - 1);
							constraints.Add(c);
							if(GetUGUIVerbosity() >= 2)
								log << "  Added constraint: " << c << "\n";
						}
					}
				}
			}
			if(GetUGUIVerbosity() >= 1)
				log << "UGUI loading complete. Total constraints: " << constraints.GetCount() << "\n";
		}
		log.Close();
		
		if(GetUGUIVerbosity() >= 1) {
			Cout() << "UGUI system initialized with " << constraints.GetCount() << " constraints.\n";
			Cout() << "Log: tail -f " << log_path << "\n";
		}
		
		Upp::SetTimeCallback(-1000, [] { CheckConstraints(); });
	}
}

}
