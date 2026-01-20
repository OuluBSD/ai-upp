#include "Maestro.h"

namespace Upp {

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	callback = cb;
	if(p.IsRunning()) p.Kill();
	
	String cmd = binary;
	for(const auto& arg : args)
		cmd << " " << arg;
	
	if(!p.Start(cmd)) {
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
	if(p.IsRunning()) p.Kill();
}

bool CliMaestroEngine::Do() {
	String out;
	if(p.Read(out)) {
		buffer.Cat(out);
		int pos;
		while((pos = buffer.Find('\n')) >= 0) {
			String line = buffer.Left(pos);
			buffer = buffer.Mid(pos + 1);
			
			if(line.IsEmpty()) continue;
			
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
				
				if(callback) callback(e);
			} else {
				// Treat as raw output if not JSON?
				// For stream-json mode, everything should be JSON.
				// But warnings might come as text.
			}
		}
	}
	return p.IsRunning() || !buffer.IsEmpty();
}

}
