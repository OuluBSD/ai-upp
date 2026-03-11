#include "ScriptIDE.h"

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

// --- GameStatePluginGUI ---

GameStatePluginGUI::GameStatePluginGUI()
{
}

GameStatePluginGUI::~GameStatePluginGUI()
{
}

void GameStatePluginGUI::Init(IPluginContext& context_)
{
	GameStatePlugin::Init(context_);
	
	if(IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(context)) {
		gui->RegisterDockPane("GameStateStats", "Game Stats", stats_view);
		
		if(IPluginRegistryGUI* reg = dynamic_cast<IPluginRegistryGUI*>(context)) {
			reg->RegisterFileTypeHandler(*this);
			reg->RegisterDockPaneProvider(*this);
		}
		
		gui->GetIDE().Log("GameStatePlugin (GUI) initialized.");
	}
}

void GameStatePluginGUI::Shutdown()
{
	if(context) {
		if(IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(context)) {
			gui->UnregisterDockPane("GameStateStats");
		}
	}
	GameStatePlugin::Shutdown();
}

IDocumentHost* GameStatePluginGUI::CreateDocumentHost()
{
	return new GameStateDocumentHost();
}

void GameStatePluginGUI::OnUpdateStats(const String& json)
{
	stats_view.SetQTF(String("[[6 @r Game Statistics]&") +
	                  "[* JSON Length: ] " + AsString(json.GetLength()) + "&" +
	                  "[* Status: ] Active simulation.");
}

// Registration
REGISTER_PLUGIN(GameStatePluginGUI)
