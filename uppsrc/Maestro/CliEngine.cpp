#include "Maestro.h"

namespace Upp {

CliMaestroEngine::GeminiSessionCache CliMaestroEngine::gemini_cache;
Mutex CliMaestroEngine::gemini_mutex;
bool CliMaestroEngine::gemini_updating = false;
Thread CliMaestroEngine::update_thread;

CliMaestroEngine::~CliMaestroEngine() {
	if(p && p->IsRunning()) p->Kill();
}

void CliMaestroEngine::UpdateGeminiSessions() {
	// ... implementation stub ...
}

void CliMaestroEngine::LoadGeminiCache() {
	// ... implementation stub ...
}

void CliMaestroEngine::SaveGeminiCache() {
	// ... implementation stub ...
}

void CliMaestroEngine::ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) {
	// ... implementation stub ...
}

void CliMaestroEngine::Send(const String& prompt, Function<void(const MaestroEvent&)> cb) {
	event_callback = cb;
	if(p && p->IsRunning()) p->Kill();
	
	p.Create();
	
	// String full_binary = GetWhich(binary);
	
	String full_binary = binary; 
	
	if(full_binary.IsEmpty()) {
	

		if(cb) {
			MaestroEvent e;
			e.type = "error";
			e.text = "Binary not found: " + binary;
			cb(e);
		}
		return;
	}
	
	if(!p->Start(full_binary, args)) {
		if(cb) {
			MaestroEvent e;
			e.type = "error";
			e.text = "Failed to start process: " + full_binary;
			cb(e);
		}
		return;
	}
	
	p->Write(prompt);
	p->CloseWrite();
}

void CliMaestroEngine::Cancel() {
	if(p && p->IsRunning()) p->Kill();
}

bool CliMaestroEngine::Do() {
	if(!p) return false;
	
	String out;
	if(p->Read(out)) {
		if(event_callback) {
			MaestroEvent e;
			e.type = "message";
			e.text = out;
			e.role = "assistant";
			event_callback(e);
		}
		return true;
	}
	
	if(!p->IsRunning()) {
		if(event_callback) {
			MaestroEvent e;
			e.type = "done";
			event_callback(e);
		}
		return false;
	}
	
	return true;
}

void CliMaestroEngine::WriteToolResult(const String& tool_id, const Value& result) {
	// Simple text-based tool result for CLI interaction
	if(p && p->IsRunning()) {
		String json = AsJSON(result);
		p->Write("\nTool Result [" + tool_id + "]: " + json + "\n");
	}
}

} // namespace Upp
