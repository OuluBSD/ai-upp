#include "Maestro.h"

NAMESPACE_UPP

CliMaestroEngine::GeminiSessionCache CliMaestroEngine::gemini_cache;
Mutex CliMaestroEngine::gemini_mutex;
bool CliMaestroEngine::gemini_updating = false;
Thread CliMaestroEngine::update_thread;

CliMaestroEngine::~CliMaestroEngine() {
	// Note: static update_thread might still be running, but it doesn't touch 'this' anymore
}

static String GetWhich(const String& bin) {
	if(bin.StartsWith("/") || bin.Find('/') != -1) return bin;
#ifdef PLATFORM_POSIX
	String p = TrimBoth(Sys("which " + bin));
	return p.IsEmpty() ? bin : p;
#else
	return bin;
#endif
}

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	event_callback = cb;
	if(p && p->IsRunning()) p->Kill();
	p.Create();
	
	String full_binary = GetWhich(binary);
	Vector<String> cmd_args;
	bool use_stdin = true;
	
	for(int i = 0; i < args.GetCount(); i++) {
		const String& a = args[i];
		if(!model.IsEmpty() && (a == "-m" || a == "--model")) {
			i++; 
			continue;
		}
		if(a == "-p" || a == "--prompt" || a == "--print") {
			use_stdin = false;
			continue;
		}
		cmd_args.Add(a);
	}
	
	if(!model.IsEmpty()) {
		cmd_args.Add("-m");
		cmd_args.Add(model);
	}
	
	if(!session_id.IsEmpty()) {
		if(binary == "codex") {
			cmd_args.Add("exec");
			cmd_args.Add("resume");
			cmd_args.Add(session_id);
		} else if(binary == "claude") {
			cmd_args.Add("--session-id");
			cmd_args.Add(session_id);
		} else {
			cmd_args.Add("-r");
			cmd_args.Add(session_id);
		}
	}
	
	if(!use_stdin) {
		cmd_args.Add(prompt);
	}
	
	debug_log << "=== START SEND ===\n";
	
	String dir = working_dir;
	if(dir.IsEmpty()) dir = ConfigFile("ai-discussion");
	RealizeDirectory(dir);
	debug_log << "CWD: " << dir << "\n";
	
	String dbg_cmd = full_binary;
	for(const auto& a : cmd_args) dbg_cmd << " " << a;
	debug_log << "Command: " << dbg_cmd << "\n";
	if(use_stdin) debug_log << "Prompt (stdin): " << prompt << "\n";
	
	if(!p->Start(full_binary, cmd_args, NULL, dir)) {
		debug_log << "ERROR: Failed to start process: " << dbg_cmd << "\n";
		MaestroEvent e;
		e.type = "error";
		e.text = "Failed to start process: " + dbg_cmd;
		if(event_callback) event_callback(e);
		return;
	}
	
	if(use_stdin) {
		p->Write(prompt + "\n");
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
				if(e.role.IsEmpty() && !v["message"].IsVoid())
					e.role = v["message"]["role"].ToString();
				if(e.role.IsEmpty() && !v["event"].IsVoid())
					e.role = v["event"]["message"]["role"].ToString();
				
				if(!e.delta) e.delta = (bool)v["delta"];
				if(e.type == "turn.delta" || e.type == "partial_message" || e.type == "stream_event") e.delta = true;
				
				String type = v["type"].ToString();
				if(type == "tool_use") {
					e.type = "tool_use";
					e.tool_id = v["tool_id"].ToString();
					e.tool_name = v["tool_name"].ToString();
					if(e.tool_name.IsEmpty()) e.tool_name = v["name"].ToString();
					if(e.tool_id.IsEmpty()) e.tool_id = v["id"].ToString();
					
					Value params = v["parameters"];
					if(params.IsVoid()) params = v["input"];
					
					if(params.Is<ValueMap>()) {
						const ValueMap& vm = params;
						for(int i = 0; i < vm.GetCount(); i++) {
							if(i > 0) e.tool_input << "\n";
							e.tool_input << vm.GetKey(i) << ": " << vm.GetValue(i).ToString();
						}
					} else {
						e.tool_input = AsJSON(params);
					}
				} else if(type == "tool_result") {
					e.type = "tool_result";
					e.tool_name = "result";
					e.text = v["output"].ToString();
					if(e.text.IsEmpty()) e.text = v["content"].ToString();
				} else if(type == "assistant" || type == "user") {
					Value content = v["message"]["content"];
					if(content.Is<ValueArray>() && content.GetCount() > 0) {
						Value c0 = content[0];
						if(c0["type"] == "tool_use") {
							e.type = "tool_use";
							e.tool_id = c0["id"].ToString();
							e.tool_name = c0["name"].ToString();
							Value params = c0["input"];
							if(params.Is<ValueMap>()) {
								const ValueMap& vm = params;
								for(int i = 0; i < vm.GetCount(); i++) {
									if(i > 0) e.tool_input << "\n";
									e.tool_input << vm.GetKey(i) << ": " << vm.GetValue(i).ToString();
								}
							} else {
								e.tool_input = AsJSON(params);
							}
						} else if(c0["type"] == "tool_result") {
							e.type = "tool_result";
							e.tool_name = "result";
							e.text = c0["content"].ToString();
						}
					}
				}
				
				if(!v["session_id"].IsVoid()) {
					e.session_id = v["session_id"].ToString();
					session_id = e.session_id;
				} else if(!v["thread_id"].IsVoid()) {
					e.session_id = v["thread_id"].ToString();
					session_id = e.session_id;
				}
				
				debug_log << "EVENT: " << e.type << (e.delta ? " (delta)" : "") << ", role=" << e.role << ", len=" << e.text.GetCount() << (e.session_id.IsEmpty() ? "" : ", sid=" + e.session_id) << "\n";
				
				if(e.type == "tool_use" && !e.tool_input.IsEmpty() && (e.tool_input.StartsWith("{ ") || e.tool_input.StartsWith("[ "))) {
					Value v_in = ParseJSON(e.tool_input);
					if(!v_in.IsError() && v_in.Is<ValueMap>()) {
						e.tool_input.Clear();
						const ValueMap& vm = v_in;
						for(int i = 0; i < vm.GetCount(); i++) {
							if(i > 0) e.tool_input << "\n";
							e.tool_input << vm.GetKey(i) << ": " << vm.GetValue(i).ToString();
						}
					}
				}
				
				if(event_callback) event_callback(e);
			} else {
				debug_log << "WARN: Failed to parse JSON: " << line << "\n";
			}
		}
	}
	return p && (p->IsRunning() || !buffer.IsEmpty());
}

void CliMaestroEngine::WriteToolResult(const String& tool_id, const Value& result) {
	if(!p || !p->IsRunning()) return;
	
	ValueMap res;
	res.Add("type", "message");
	res.Add("role", "user");
	res.Add("content", "TOOL_RESULT [" + tool_id + "]: " + AsString(result));
	
	p->Write(AsJSON(res) + "\n");
}

static void ScanMaestroProjects(const String& projects_dir, VectorMap<String, Array<SessionInfo>>& project_sessions, const String& ext) {
	FindFile ff(AppendFileName(projects_dir, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			String dir_name = ff.GetName();
			String resolved_path = dir_name;
			resolved_path.Replace("-", "/");
			if(!resolved_path.StartsWith("/")) resolved_path = "/" + resolved_path;
			
			Array<SessionInfo>& sessions = project_sessions.GetAdd(resolved_path);
			
			String chats_dir = AppendFileName(ff.GetPath(), "chats");
			FindFile fchat(AppendFileName(chats_dir, "*." + ext));
			while(fchat) {
				SessionInfo& s = sessions.Add();
				s.id = GetFileTitle(fchat.GetName());
				s.timestamp = fchat.GetLastWriteTime();
				
				String first_line = FileIn(fchat.GetPath()).GetLine();
				Value v = ParseJSON(first_line);
				if(!v.IsError()) {
					if(!v["message"]["parts"][0]["text"].IsVoid())
						s.name = v["message"]["parts"][0]["text"].ToString().Left(100);
					else if(!v["message"]["content"].IsVoid()) {
						Value content = v["message"]["content"];
						if(content.Is<ValueArray>() && content.GetCount() > 0)
								s.name = content[0]["text"].ToString().Left(100);
						else
							s.name = content.ToString().Left(100);
					}
					s.name.Replace("\n", " ");
				}
				if(s.name.IsEmpty()) s.name = s.id;
				fchat.Next();
			}
		}
		ff.Next();
	}
}

void CliMaestroEngine::ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) {
	debug_log << "=== LIST SESSIONS (CWD: " << cwd << ") ===\n";
	project_sessions.Clear();
	Array<SessionInfo> list;
	
	String home = GetHomeDirectory();
	
	if(binary == "gemini") {
		// Use cache if available
		LoadGeminiCache();
		
		{
			Mutex::Lock __(gemini_mutex);
			if(gemini_cache.sessions.GetCount() > 0) {
				for(const auto& s : gemini_cache.sessions) {
					list.Add(s);
				}
			}
			
			// Trigger background update if not already running
			if(!gemini_updating) {
				gemini_updating = true;
				update_thread.Start(callback(&CliMaestroEngine::UpdateGeminiSessions));
			}
		}
		
		// If cache was empty, we might need to wait or return empty and let callback handle update?
		// For now, return what we have (potentially stale or empty, but fast)
		cb(list);
		return;
	}
	
	if(binary == "qwen") {
		ScanMaestroProjects(AppendFileName(home, ".qwen/projects"), project_sessions, "jsonl");
	} else if(binary == "claude") {
		ScanMaestroProjects(AppendFileName(home, ".claude/projects"), project_sessions, "jsonl");
	} else if(binary == "codex") {
		String codex_dir = AppendFileName(home, ".codex/sessions");
		FindFile ff(AppendFileName(codex_dir, "*.json"));
		while(ff) {
			SessionInfo& s = list.Add();
			s.id = GetFileTitle(ff.GetName());
			s.timestamp = ff.GetLastWriteTime();
			s.name = s.id;
			ff.Next();
		}
		cb(list);
		return;
	}
	
	int q = project_sessions.Find(cwd);
	if(q < 0) {
		String ncwd = cwd;
		while(ncwd.EndsWith("/") || ncwd.EndsWith("\\")) ncwd.Trim(ncwd.GetCount() - 1);
		q = project_sessions.Find(ncwd);
	}
	
	if(q >= 0) cb(project_sessions[q]);
	else cb(list); 
}

void CliMaestroEngine::UpdateGeminiSessions() {
	String home = GetHomeDirectory();
	Vector<SessionInfo> new_list;
	
	String out = Sys(GetWhich("gemini") + " --list-sessions");
	Vector<String> lines = Split(out, '\n');
	for(const String& l : lines) {
		Value v = ParseJSON(l);
		if(!v.IsError() && !v["session_id"].IsVoid()) {
			SessionInfo& s = new_list.Add();
			s.id = v["session_id"];
			s.name = v["title"];
			if(s.name.IsEmpty()) s.name = s.id;
		}
	}
	
	// Also scan tmp dir for loose files (legacy/fallback)
	String tmp_dir = AppendFileName(home, ".gemini/tmp");
	FindFile ff(AppendFileName(tmp_dir, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			String chats_dir = AppendFileName(ff.GetPath(), "chats");
			FindFile fchat(AppendFileName(chats_dir, "*.json"));
			while(fchat) {
				String id = GetFileTitle(fchat.GetName());
				if(id.StartsWith("session-")) id = id.Mid(8);
				
				// Dedup check
				bool exists = false;
				for(const auto& s : new_list) if(s.id == id) { exists = true; break; }
				
				if(!exists) {
					SessionInfo& s = new_list.Add();
					s.id = id;
					s.timestamp = fchat.GetLastWriteTime();
					s.name = id;
				}
				fchat.Next();
			}
		}
		ff.Next();
	}
	
	{
		Mutex::Lock __(gemini_mutex);
		gemini_cache.sessions = pick(new_list);
		gemini_cache.last_update = GetSysTime();
	}
	SaveGeminiCache();
	gemini_updating = false;
}

void CliMaestroEngine::LoadGeminiCache() {
	String path = ConfigFile("gemini_sessions.json");
	String json = LoadFile(path);
	if(json.IsEmpty()) return;
	
	Mutex::Lock __(gemini_mutex);
	LoadFromJson(gemini_cache, json);
}

void CliMaestroEngine::SaveGeminiCache() {
	String path = ConfigFile("gemini_sessions.json");
	Mutex::Lock __(gemini_mutex);
	StoreAsJsonFile(gemini_cache, path);
}

END_UPP_NAMESPACE