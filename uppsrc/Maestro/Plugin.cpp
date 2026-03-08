#include "Maestro.h"
#include "Plugin.h"

namespace Upp {

void PluginManager::LoadPlugins(const String& maestro_root)
{
	// Manual registration of internal "plugins" for now
	// To be expanded with dynamic loading
}

#ifdef flagGUI
void PluginManager::RegisterAll(MaestroToolRegistry& reg, TabCtrl& tabs)
{
	for(auto& p : plugins) {
		p.RegisterTools(reg);
		p.RegisterViews(tabs);
	}
}

void PluginManager::RegisterMenu(Bar& bar)
{
	for(auto& p : plugins)
		p.RegisterMenu(bar);
}
#else
void PluginManager::RegisterAll(MaestroToolRegistry& reg)
{
	for(auto& p : plugins) {
		p.RegisterTools(reg);
	}
}
#endif

PluginManager& PluginManager::Get()
{
	static PluginManager pm;
	return pm;
}

}
