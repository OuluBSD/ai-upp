#include "Product.h"

NAMESPACE_UPP

ProductPane::ProductPane() {
	Add(split.SizePos());
	split.Horz(runbooks, workflows);
	
	runbooks.AddColumn("Runbook ID");
	runbooks.AddColumn("Title");
	
	workflows.AddColumn("Workflow ID");
	workflows.AddColumn("Title");
	
	// Add selection handlers to show summary (future enhancement: detailed view)
}

void ProductPane::Load(const String& root) {
	PlanParser pp;
	pp.LoadRunbooks(root);
	pp.LoadWorkGraphs(root);
	
	runbooks.Clear();
	for(const auto& rb : pp.runbooks)
		runbooks.Add(rb.id, rb.title);
		
	workflows.Clear();
	for(const auto& wg : pp.workgraphs)
		workflows.Add(wg.id, wg.title);
}

END_UPP_NAMESPACE