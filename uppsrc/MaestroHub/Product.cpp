#include "Product.h"
#include <AI/Engine/PlanSummarizer.h>

NAMESPACE_UPP

ProductPane::ProductPane() {
	Add(split.SizePos());
	
vsplit_rb.Vert(runbooks, rb_detail);
	
	// Workflow view split: Detail (Text) vs Graph (Visual)
	workflow_view.Add(wg_split.SizePos());
	wg_split.Vert(wg_detail, workflow_graph);
	wg_split.SetPos(2000); // Give detail some space, graph the rest
	
vsplit_wg.Vert(workflows, workflow_view);
	
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
			int step_idx = ScanInt(link.Mid(5)) - 1; 
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
		workflow_graph.GetGraph().Clear();
		workflow_graph.Refresh();
		return;
	}
	
	const WorkGraph& wg = workflow_data[workflows.GetCursor()];
	Cout() << "DEBUG: Selecting Workflow: " << wg.title << " Phases: " << wg.phases.GetCount() << "\n";
	String summary = PlanSummarizer::GetWorkGraphSummary(wg);
	wg_detail.SetQTF("[C1 " + DeQtf(summary) + "]");
	
	// Visualization
	GraphLib::Graph& g = workflow_graph.GetGraph();
	g.Clear();
	
	int y = 50;
	for(const auto& p : wg.phases) {
		// Group per phase
		String groupId = "Phase_" + p.name;
		GraphLib::GroupNode& group = workflow_graph.AddGroup(groupId, p.name, Point(50, y), Size(300, p.tasks.GetCount() * 80 + 40));
		group.header_clr = Color(200, 220, 255);
		
		int ty = y + 30;
		for(const auto& t : p.tasks) {
			String nodeId = t.title; // Using title as ID for display simplicity
			// In real usage, task should have an ID. WorkGraphTask currently lacks explicit ID in struct, assumes title/index
			
			GraphLib::Node& n = workflow_graph.AddNode(nodeId, Point(70, ty));
			n.SetLabel(t.title);
			if(t.status == "done") n.SetFill(Green());
			else if(t.status == "blocked") n.SetFill(LtRed());
			else n.SetFill(White());
			
			n.AddPin("in", GraphLib::PinKind::Input);
			n.AddPin("out", GraphLib::PinKind::Output);
			
			workflow_graph.AddNodeToGroup(nodeId, groupId);
			
			ty += 80;
		}
		
		y += p.tasks.GetCount() * 80 + 60;
	}
	
	// Add dependencies based on depends_on
	for(const auto& p : wg.phases) {
		for(const auto& t : p.tasks) {
			for(const String& dep : t.depends_on) {
				// dep is the title of the prerequisite task
				workflow_graph.AddEdge(dep, "out", t.title, "in");
			}
		}
	}
	
	workflow_graph.Refresh();
}

END_UPP_NAMESPACE