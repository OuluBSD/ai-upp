#include "QwenManager.h"

NAMESPACE_UPP

String QwenServerConnectionConf::GetAddress() const {
	// TODO read qwen address
	return String();
}

String QwenServerConnectionConf::GetLabel() const {
	return name.IsEmpty() ? GetAddress() : name;
}

String QwenServerConnectionConf::GetStatusString() const {
	// TODO qwen status string
	return String();
}

void QwenServerConnectionConf::Jsonize(JsonIO& jio) {
	jio
		("name", name)
		//("", ) // TODO qwen variables
		;
}

void QwenProject::Jsonize(JsonIO& jio) {
	jio
		("name", name)
		("preferred_connection_name", preferred_connection_name)
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
