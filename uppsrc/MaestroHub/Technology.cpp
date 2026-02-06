#include "MaestroHub.h"

NAMESPACE_UPP

TechnologyPane::TechnologyPane() {
	Add(split.SizePos());
	split.Vert(repo, plan);
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



PipelinePane::PipelinePane() {



	CtrlLayout(layout);



	Add(layout.SizePos());



	layout.list.AddColumn("Stage");



	layout.list.AddColumn("Status");



	layout.list.AddColumn("Performance");



}







void PipelinePane::Load(const String& root_path) {







	layout.list.Clear();







	layout.list.Add("Inventory Scanning", "Completed", "0.4s");







	layout.list.Add("Dependency Mapping", "In Progress", "1.2s");







	layout.list.Add("Heuristic Analysis", "Pending", "-");







	layout.list.Add("Code Transformation", "Pending", "-");







	layout.list.Add("Semantic Verification", "Pending", "-");







}











END_UPP_NAMESPACE
