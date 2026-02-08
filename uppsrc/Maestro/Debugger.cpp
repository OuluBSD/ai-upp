#include "Maestro.h"

namespace Upp {

GdbService::GdbService() {
}

void GdbService::Run(const String& cmd, const String& args) {
	String gdb_cmd = "gdb --interpreter=mi " + cmd;
	if(!proc.Start(gdb_cmd)) {
		if(WhenOutput) WhenOutput("Failed to start GDB: " + gdb_cmd + "\n");
		return;
	}
	is_running = true;
	if(WhenRunning) WhenRunning();
	
	SendCommand("-gdb-set target-async on");
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
	
	// Filter out some MI noise for the console
	if(!line.StartsWith("(gdb)") && !line.StartsWith("^") && !line.StartsWith("~")) {
		if(WhenOutput) WhenOutput(line + "\n");
	}
	
	if(line.StartsWith("*stopped")) {
		if(WhenPaused) WhenPaused();
		
		RegExp re("fullname=\"([^\"]+)\",line=\"(\\d+)\"");
		if(re.Match(line)) {
			Vector<StackFrame> stack;
			StackFrame& f = stack.Add();
			f.file = re[0];
			f.line = atoi(re[1]);
			f.function = "unknown";
			
			RegExp re_func("func=\"([^\"]+)\"");
			if(re_func.Match(line)) f.function = re_func[0];
			
			if(WhenStack) WhenStack(stack);
		}
	}
}

} // namespace Upp