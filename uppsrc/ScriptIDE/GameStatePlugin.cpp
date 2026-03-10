#include "ScriptIDE.h"

NAMESPACE_UPP

// --- GameStateDocumentHost ---

GameStateDocumentHost::GameStateDocumentHost()
{
	Add(view.SizePos());
	view.SetQTF("Empty Game State");
}

GameStateDocumentHost::~GameStateDocumentHost()
{
}

bool GameStateDocumentHost::Load(const String& path_)
{
	path = path_;
	String data = LoadFile(path);
	if(data.IsEmpty()) return false;
	
	// Simple visualization
	view.SetQTF(String("[[6 @g Game State Path: ] ") + DeQtf(path) + "&" +
	           "[[6 @b Content: ]&" + DeQtf(data));
	return true;
}

bool GameStateDocumentHost::Save()
{
	return true; // Read-only
}

// --- GameStatePlugin ---

GameStatePlugin::GameStatePlugin()
{
	stats_view.SetQTF("Game Stats: No data loaded.");
}

GameStatePlugin::~GameStatePlugin()
{
}

void GameStatePlugin::Init(IPluginContext& context_)
{
	context = &context_;
	context->RegisterDockPane("GameStateStats", "Game Stats", stats_view);
	
	context->GetIDE().Log("GameStatePlugin initialized.");
}

void GameStatePlugin::Shutdown()
{
	if(context) {
		context->UnregisterDockPane("GameStateStats");
	}
}

IDocumentHost* GameStatePlugin::CreateDocumentHost()
{
	return new GameStateDocumentHost();
}

void GameStatePlugin::SyncBindings(PyVM& vm)
{
	vm.GetGlobals().GetAdd("get_game_score") = PyValue::Function("get_game_score", &GameStatePlugin::GetScore);
}

bool GameStatePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void GameStatePlugin::Execute(const String& path)
{
	if(context) {
		context->GetIDE().Log("Simulating Game State: " + path);
		UpdateStats(LoadFile(path));
	}
}

void GameStatePlugin::UpdateStats(const String& json)
{
	stats_view.SetQTF(String("[[6 @r Game Statistics]&") +
	                  "[* JSON Length: ] " + AsString(json.GetLength()) + "&" +
	                  "[* Status: ] Active simulation.");
}

PyValue GameStatePlugin::GetScore(const Vector<PyValue>& args, void* user_data)
{
	return PyValue(42); // Dummy score
}

// Registration
REGISTER_PLUGIN(GameStatePlugin)

END_UPP_NAMESPACE
