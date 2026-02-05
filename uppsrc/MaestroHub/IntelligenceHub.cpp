#include "IntelligenceHub.h"

NAMESPACE_UPP

IntelligenceHub::IntelligenceHub() {
	CtrlLayout(*this);
	
	tabs.Add(repo.SizePos(), "Repository Browser");
	tabs.Add(tu.SizePos(), "Symbol Search (TU)");
	tabs.Add(log.SizePos(), "Log Analyzer");
}

void IntelligenceHub::Load(const String& maestro_root) {
	repo.Set({}, {}); // Should trigger scan or take data from LoadData
	tu.Load(maestro_root);
	log.Load(maestro_root);
}

END_UPP_NAMESPACE
