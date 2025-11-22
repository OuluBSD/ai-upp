#include "QwenManager.h"

NAMESPACE_UPP

String QwenServerConnectionConf::GetAddress() const {
	if (connection_type == "tcp") {
		return host + ":" + IntStr(port);
	} else if (connection_type == "stdin") {
		return "stdin/stdout";
	} else if (connection_type == "pipe") {
		return directory;
	}
	return String();
}

String QwenServerConnectionConf::GetLabel() const {
	return name.IsEmpty() ? GetAddress() : name;
}

String QwenServerConnectionConf::GetStatusString() const {
	if (is_connected) {
		return is_healthy ? "Connected" : "Connected (Unhealthy)";
	} else {
		return "Disconnected";
	}
}

void QwenServerConnectionConf::Jsonize(JsonIO& jio) {
	jio
		("name", name)
		("directory", directory)
		("host", host)
		("port", port)
		("connection_type", connection_type)
		("auto_start", auto_start)
		;
}

void QwenProject::Jsonize(JsonIO& jio) {
	jio
		("uniq", uniq)
		("name", name)
		("preferred_connection_name", preferred_connection_name)
		("git_origin_addr", git_origin_addr)
		("session_ids", session_ids)
		;
}

QwenManagerState& QwenManagerState::Global() {
	static QwenManagerState state;
	return state;
}


void QwenManagerState::Jsonize(JsonIO& jio) {
	jio
		("projects", projects)
		("servers", servers)
		;
}

bool QwenManagerState::Load(String path) {
	if (path.IsEmpty())
		path = ConfigFile("QwenManagerState.json");
	return LoadFromJsonFile(*this, path);
}

void QwenManagerState::Store(String path) {
	if (path.IsEmpty())
		path = ConfigFile("QwenManagerState.json");
	StoreAsJsonFile(*this, path);
}


END_UPP_NAMESPACE
