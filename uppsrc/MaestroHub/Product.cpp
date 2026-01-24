#include "Product.h"
#include <AI/Engine/PlanSummarizer.h>

NAMESPACE_UPP

ProductPane::ProductPane() {
	Add(split.SizePos());
	
vsplit_rb.Vert(runbooks, rb_detail);
	vsplit_wg.Vert(workflows, wg_detail);
	
split.Horz(vsplit_rb, vsplit_wg);
	
	runbooks.AddColumn("Runbook ID");
	runbooks.AddColumn("Title");
	runbooks.WhenCursor = THISBACK(OnRunbookSelect);
	
	workflows.AddColumn("Workflow ID");
	workflows.AddColumn("Title");
	workflows.WhenCursor = THISBACK(OnWorkflowSelect);
	
	rb_detail.WhenLink = [=](const String& link) {
		if(link.StartsWith("step:")) {
			if(!runbooks.IsCursor()) return;
			const Runbook& rb = runbook_data[runbooks.GetCursor()];
			int step_idx = ScanInt(link.Mid(5)) - 1; // 1-based to 0-based
			if(step_idx >= 0 && step_idx < rb.steps.GetCount()) {
				const auto& s = rb.steps[step_idx];
				String instruction;
				instruction << "Execute Step " << s.n << ": " << s.action << "\n";
				if(!s.command.IsEmpty()) instruction << "Command: " << s.command << "\n";
				if(!s.expected.IsEmpty()) instruction << "Expected: " << s.expected << "\n";
				
				if(WhenEnactStep) WhenEnactStep(rb.title, s.n, instruction);
			}
		}
	};
}

void ProductPane::Load(const String& root) {
	PlanParser pp;
	pp.LoadRunbooks(root);
	pp.LoadWorkGraphs(root);
	
	runbook_data = pick(pp.runbooks);
	workflow_data = pick(pp.workgraphs);
	
	runbooks.Clear();
	for(const auto& rb : runbook_data)
		runbooks.Add(rb.id, rb.title);
		
	workflows.Clear();
	for(const auto& wg : workflow_data)
		workflows.Add(wg.id, wg.title);
}

void ProductPane::OnRunbookSelect() {
	if(!runbooks.IsCursor()) {
		rb_detail.SetQTF("");
		return;
	}
	
	const Runbook& rb = runbook_data[runbooks.GetCursor()];
	
	// Custom QTF generation for interactive links
	String qtf;
	qtf << "[*@3 " << DeQtf(rb.title) << " (" << DeQtf(rb.id) << ")]&";
	qtf << "[* Goal:] " << DeQtf(rb.goal) << "&";
	qtf << "[* Steps:]&";
	
	for(const auto& s : rb.steps) {
		qtf << "[^step:" << s.n << "^ [C1 [>] Execute Step " << s.n << "]] ";
		qtf << "[* " << DeQtf(s.action) << "] [" << DeQtf(s.actor) << "]&";
		if(!s.command.IsEmpty()) 
			qtf << "   [C1 " << DeQtf(s.command) << "]&";
		if(!s.expected.IsEmpty())
			qtf << "   [2 Expected: " << DeQtf(s.expected) << "]&";
		qtf << "&";
	}
	
	rb_detail.SetQTF(qtf);
}

void ProductPane::OnWorkflowSelect() {
	if(!workflows.IsCursor()) {
		wg_detail.SetQTF("");
		return;
	}
	
	const WorkGraph& wg = workflow_data[workflows.GetCursor()];
	String summary = PlanSummarizer::GetWorkGraphSummary(wg);
	wg_detail.SetQTF("[C1 " + DeQtf(summary) + "]");
}

END_UPP_NAMESPACE
