#include "Maestro.h"

namespace Upp {

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	callback = cb;
	if(p && p->IsRunning()) p->Kill();
	p.Create();
	
	Vector<String> cmd_args;
	bool use_arg = false;
	
	// Check if we should use positional prompt argument or stdin
	for(const auto& a : args) {
		if(a == "-p" || a == "--prompt" || a == "--print") {
			use_arg = true;
			break;
		}
	}
	
	if(!session_id.IsEmpty()) {
		if(binary == "codex") {
			cmd_args.Add("exec");
			cmd_args.Add("resume");
			cmd_args.Add(session_id);
			for(const auto& arg : args)
				if(arg != "exec") cmd_args.Add(arg);
		} else if(binary == "claude") {
			cmd_args.Add("--session-id");
			cmd_args.Add(session_id);
			for(const auto& arg : args)
				cmd_args.Add(arg);
		} else {
			// Generic flag-based resumption (Gemini, Qwen, Claude)
			for(int i = 0; i < args.GetCount(); i++) {
				if(args[i] == "-p" || args[i] == "--prompt" || args[i] == "--print") {
					cmd_args.Add("-r");
					cmd_args.Add(session_id);
				}
				cmd_args.Add(args[i]);
			}
		}
	} else {
		for(const auto& arg : args)
			cmd_args.Add(arg);
	}
	
	if(use_arg) {
		cmd_args.Add(prompt);
	}
	
	debug_log << "=== START SEND ===\n";
	
	String dir = ConfigFile("ai-discussion");
	RealizeDirectory(dir);
	debug_log << "CWD: " << dir << "\n";
	
	String dbg_cmd = binary;
	for(const auto& a : cmd_args) dbg_cmd << " " << a;
	debug_log << "Command: " << dbg_cmd << "\n";
	if(!use_arg) debug_log << "Prompt (stdin): " << prompt << "\n";
	
	if(!p->Start(binary, cmd_args, NULL, dir)) {
		debug_log << "ERROR: Failed to start process: " << dbg_cmd << "\n";
		MaestroEvent e;
		e.type = "error";
		e.text = "Failed to start process: " + dbg_cmd;
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
				e.type = v["type"].ToString();
				
				if(!v["text_delta"].IsVoid()) e.text = v["text_delta"].ToString();
				else if(!v["text_full"].IsVoid()) e.text = v["text_full"].ToString();
				else if(!v["content"].IsVoid()) {
					Value content = v["content"];
					if(content.Is<ValueArray>() && content.GetCount() > 0)
						e.text = content[0]["text"].ToString();
					else
						e.text = content.ToString();
				}
				else if(!v["chunk"].IsVoid()) e.text = v["chunk"].ToString(); 
				else if(!v["message"].IsVoid()) {
					Value msg = v["message"];
					if(msg.Is<ValueMap>() && !msg["content"].IsVoid()) {
						Value content = msg["content"];
						if(content.Is<ValueArray>() && content.GetCount() > 0)
							e.text = content[0]["text"].ToString();
						else
							e.text = content.ToString();
					} else {
						e.text = msg.ToString();
					}
				}
				else if(!v["event"].IsVoid()) {
					Value event = v["event"];
					if(event["type"] == "content_block_delta") {
						Value delta = event["delta"];
						if(!delta["text"].IsVoid()) e.text = delta["text"].ToString();
						else if(!delta["text_delta"].IsVoid()) e.text = delta["text_delta"].ToString();
						e.delta = true;
					}
				}
				else if(!v["result"].IsVoid()) e.text = v["result"].ToString();
				else if(!v["error"].IsVoid()) {
					if(v["error"].Is<ValueMap>() && !v["error"]["message"].IsVoid())
						e.text = v["error"]["message"].ToString();
					else
						e.text = v["error"].ToString();
				}
				
				e.role = v["role"].ToString();
				if(e.role.IsEmpty() && !v["event"].IsVoid())
					e.role = v["event"]["message"]["role"].ToString();
				
				if(!e.delta) e.delta = (bool)v["delta"];
				if(e.type == "turn.delta" || e.type == "partial_message" || e.type == "stream_event") e.delta = true;
				
				// Capture session_id
				if(!v["session_id"].IsVoid()) {
					e.session_id = v["session_id"].ToString();
					session_id = e.session_id;
				} else if(!v["thread_id"].IsVoid()) {
					e.session_id = v["thread_id"].ToString();
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