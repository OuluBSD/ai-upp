#include "MaestroHub.h"

NAMESPACE_UPP

IntelligenceHub::IntelligenceHub() {
	Add(tabs.SizePos());
	
	tu_browser.Create();
	log_analyzer.Create();
	conversion.Create();
	
	tabs.Add(tu_browser->SizePos(), "Translation Unit Browser");
	tabs.Add(log_analyzer->SizePos(), "Log & Trace Analyzer");
	tabs.Add(conversion->SizePos(), "Conversion Factory");
	tabs.Add(repo.SizePos(), "Repository Map");
}

void IntelligenceHub::Load(const String& maestro_root) {
	if(tu_browser) tu_browser->Load(maestro_root);
	if(log_analyzer) log_analyzer->Load(maestro_root);
	if(conversion) conversion->Load(maestro_root);
	
	RepoScanner rs;
	rs.Scan(maestro_root);
	repo.Set(rs.assemblies, rs.packages);
}

END_UPP_NAMESPACE
