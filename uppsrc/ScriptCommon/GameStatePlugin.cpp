#include "ScriptCommon.h"

NAMESPACE_UPP

GameStatePlugin::GameStatePlugin()
{
}

GameStatePlugin::~GameStatePlugin()
{
}

void GameStatePlugin::Init(IPluginContext& context_)
{
	context = &context_;
	context->RegisterPythonBindingProvider(*this);
	// Note: CardGamePlugin handles .gamestate execution; GameStatePlugin only provides bindings/stats
}

void GameStatePlugin::Shutdown()
{
}

void GameStatePlugin::SyncBindings(PyVM& vm)
{
	vm.GetGlobals().SetItem(PyValue("get_game_score"), PyValue::Function("get_game_score", &GameStatePlugin::GetScore));
}

bool GameStatePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void GameStatePlugin::Execute(const String& path)
{
	OnUpdateStats(LoadFile(path));
}

PyValue GameStatePlugin::GetScore(const Vector<PyValue>& args, void* user_data)
{
	return PyValue(42); // Dummy score
}

REGISTER_PLUGIN(GameStatePlugin)

END_UPP_NAMESPACE
