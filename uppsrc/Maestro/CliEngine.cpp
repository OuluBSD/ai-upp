#include "Maestro.h"

namespace Upp {

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	callback = cb;
	if(p.IsRunning()) p.Kill();
	
	String cmd = binary;
	for(const auto& arg : args)
		cmd << " " << arg;
	
debug_log << "=== START SEND ===\n";
debug_log << "Command: " << cmd << "\n";
debug_log << "Prompt: " << prompt << "\n";

	if(!p.Start(cmd)) {
		debug_log << "ERROR: Failed to start process\n";
		MaestroEvent e;
		e.type = "error";
		e.text = "Failed to start process: " + cmd;
		if(callback) callback(e);
		return;
	}
	
p.Write(prompt);
	p.CloseWrite();
	buffer.Clear();
}

void CliMaestroEngine::Cancel() {
	if(p.IsRunning()) {
		debug_log << "=== CANCELLED ===\n";
		p.Kill();
	}
}

bool CliMaestroEngine::Do() {
	String out;
	if(p.Read(out)) {
		buffer.Cat(out);
		debug_log << "READ: " << out.GetCount() << " bytes\n";
		// Optional: debug_log << "RAW: " << out << "\n"; 
		
		int pos;
		while((pos = buffer.Find('\n')) >= 0) {
			String line = buffer.Left(pos);
			buffer = buffer.Mid(pos + 1);
			
			if(line.IsEmpty()) continue;
			
			debug_log << "LINE: " << line << "\n";
			
			// Parse JSON
			Value v = ParseJSON(line);
			if(!v.IsError()) {
				MaestroEvent e;
				e.json = v;
				e.type = v["type"];
				
				if(!v["text_delta"].IsVoid()) e.text = v["text_delta"];
				else if(!v["text_full"].IsVoid()) e.text = v["text_full"];
				else if(!v["error"].IsVoid()) e.text = v["error"];
				
e.role = v["role"];
				
debug_log << "EVENT: " << e.type << ", role=" << e.role << ", len=" << e.text.GetCount() << "\n";
				
				if(callback) callback(e);
			} else {
				debug_log << "WARN: Failed to parse JSON\n";
			}
		}
	}
	return p.IsRunning() || !buffer.IsEmpty();
}

} // namespace Upp