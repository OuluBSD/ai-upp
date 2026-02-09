#include "Maestro.h"

namespace Upp {

GdbService::GdbService() {
}

void GdbService::Run(const String& cmd, const String& args) {
	String gdb_cmd = "gdb --interpreter=mi";
	if(!proc.Start(gdb_cmd)) {
		if(WhenOutput) WhenOutput("Failed to start GDB: " + gdb_cmd + "\n");
		return;
	}
	is_running = true;
	if(WhenRunning) WhenRunning();
	
	SendCommand("-gdb-set mi-async on");
	SendCommand("-file-exec-and-symbols " + cmd);
	SendCommand("-break-insert main");
	SendCommand("-exec-run");
	
#ifdef flagGUI
	PostCallback(THISBACK(ReadOutput));
#endif
}

void GdbService::Stop() {
	SendCommand("-gdb-exit");
	proc.Kill();
	is_running = false;
}

void GdbService::Step() {
	SendCommand("-exec-step");
}

void GdbService::Next() {
	SendCommand("-exec-next");
}

void GdbService::Cont() {
	SendCommand("-exec-continue");
}

void GdbService::SendCommand(const String& cmd) {
	if(proc.IsRunning()) {
		Upp::Cout() << "GDB_SEND: " << cmd << "\n";
		Upp::Cout().Flush();
		proc.Write(cmd + "\n");
	}
}

void GdbService::ReadOutput() {
	if(!proc.IsRunning()) {
		is_running = false;
		return;
	}
	
	String out;
	if(proc.Read(out)) {
		Vector<String> lines = Split(out, '\n');
		for(const String& l : lines) {
			ParseLine(TrimBoth(l));
		}
	}
	
#ifdef flagGUI
	if(is_running) PostCallback(THISBACK(ReadOutput));
#endif
}

void GdbService::ParseLine(const String& line) {
	if(line.IsEmpty()) return;
	
	if(line.StartsWith("*stopped")) {
		if(WhenPaused) WhenPaused();
		SendCommand("-stack-list-frames");
	}

	if(line.StartsWith("^done,stack")) {
		Vector<StackFrame> stack;
		
		auto GetMIValue = [&](const String& src, const char *key) -> String {
			String skey = String(key) + "=\"";
			int p = src.Find(skey);
			if(p >= 0) {
				p += skey.GetLength();
				int e = src.Find('\"', p);
				if(e >= 0) return src.Mid(p, e - p);
			}
			return "";
		};

		// Split by frame={ or just { if we are inside the list
		Vector<String> frames = Split(line, "frame={");
		if(frames.GetCount() <= 1) frames = Split(line, "{"); // fallback
		
		for(int i = 1; i < frames.GetCount(); i++) {
			String fsrc = frames[i];
			if(fsrc.Find("level=") >= 0) {
				StackFrame& f = stack.Add();
				f.function = GetMIValue(fsrc, "func");
				if(f.function.IsEmpty()) f.function = "unknown";
				f.file = GetMIValue(fsrc, "fullname");
				if(f.file.IsEmpty()) f.file = GetMIValue(fsrc, "file");
				f.line = atoi(GetMIValue(fsrc, "line"));
			}
		}
		
		if(WhenStack) WhenStack(stack);
	}
}

} // namespace Upp