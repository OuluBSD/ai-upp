#include "ScriptCommon.h"

NAMESPACE_UPP

CardGamePlugin::CardGamePlugin()
{
}

CardGamePlugin::~CardGamePlugin()
{
}

void CardGamePlugin::Init(IPluginContext& ctx)
{
	context = &ctx;
	context->RegisterCustomExecuteProvider(*this);
	context->RegisterPythonBindingProvider(*this);
}

void CardGamePlugin::Shutdown()
{
}

bool CardGamePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void CardGamePlugin::Execute(const String& path)
{
	// Core execute logic (e.g. log launch)
}

void CardGamePlugin::SyncBindings(PyVM& vm)
{
	// Core bindings
}

REGISTER_PLUGIN(CardGamePlugin)

END_UPP_NAMESPACE
