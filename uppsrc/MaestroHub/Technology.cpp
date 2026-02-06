#include "MaestroHub.h"

NAMESPACE_UPP

TechnologyPane::TechnologyPane() {
	Add(split.SizePos());
	split.Horz(repo, plan);
}

void TechnologyPane::Load(const String& root_path) {
	root = root_path;
	RepoScanner rs;
	rs.Scan(root);
	repo.Set(rs.assemblies, rs.packages);
	
	PlanParser pp;
	pp.LoadMaestroTracks(root);
	plan.Set(pp.tracks);
}

END_UPP_NAMESPACE