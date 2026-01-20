#include "Maestro.h"

namespace Upp {

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	callback = cb;
	if(p && p->IsRunning()) p->Kill();
	p.Create();
	
	String cmd = binary;
	for(int i = 0; i < args.GetCount(); i++) {
		if((args[i] == "-p" || args[i] == "--prompt") && !session_id.IsEmpty()) {
			cmd << " -r " << session_id;
		}
		cmd << " " << args[i];
	}
	
	debug_log << "=== START SEND ===\n";
	
	String dir = ConfigFile("ai-discussion");
	RealizeDirectory(dir);
	debug_log << "CWD: " << dir << "\n";
	
	bool use_arg = false;
	if(args.GetCount() > 0 && (args.Top() == "-p" || args.Top() == "--prompt")) {
		use_arg = true;
		cmd << " " << "'" << prompt << "'";
	}
	
	debug_log << "Command: " << cmd << "\n";
	if(!use_arg) debug_log << "Prompt (stdin): " << prompt << "\n";
	
	if(!p->Start(cmd, NULL, dir)) {
		debug_log << "ERROR: Failed to start process\n";
		MaestroEvent e;
		e.type = "error";
		e.text = "Failed to start process: " + cmd;
		if(callback) callback(e);
		return;
	}
	
	if(!use_arg) {
		p->Write(prompt);
		p->CloseWrite();
	}
	buffer.Clear();
}

void CliMaestroEngine::Cancel() {
	if(p && p->IsRunning()) {
		debug_log << "=== CANCELLED ===\n";
		p->Kill();
	}
}

bool CliMaestroEngine::Do() {
	if(!p) return false;
	String out;
	if(p->Read(out)) {
		buffer.Cat(out);
		debug_log << "READ: " << out.GetCount() << " bytes\n";
		
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
				else if(!v["content"].IsVoid()) e.text = v["content"];
				else if(!v["error"].IsVoid()) e.text = v["error"];
				
				e.role = v["role"];
				e.delta = v["delta"];
				if(!v["session_id"].IsVoid()) {
					e.session_id = v["session_id"];
					session_id = e.session_id;
				}
				
				debug_log << "EVENT: " << e.type << (e.delta ? " (delta)" : "") << ", role=" << e.role << ", len=" << e.text.GetCount() << (e.session_id.IsEmpty() ? "" : ", sid=" + e.session_id) << "\n";
				
				if(callback) callback(e);
			} else {
				debug_log << "WARN: Failed to parse JSON: " << line << "\n";
			}
		}
	}
	return p->IsRunning() || !buffer.IsEmpty();
}

} // namespace Upp