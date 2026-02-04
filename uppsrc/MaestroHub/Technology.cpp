#include "MaestroHub.h"

NAMESPACE_UPP

TechnologyPane::TechnologyPane() {
	Add(split.SizePos());
	split.Horz(repo, plan);
	
	plan.WhenEnact = [=](String t, String p, String k) {
		if(WhenEnact) WhenEnact(t, p, k);
	};
}

void TechnologyPane::Load(const String& root) {
	RepoScanner rs;
	rs.Scan(root);
	rs.DetectAssemblies();
	repo.Set(rs.assemblies, rs.packages);
	
	PlanParser pp;
	pp.LoadMaestroTracks(root);
	plan.Set(pp.tracks);
}

END_UPP_NAMESPACE
