#include "MaestroHub.h"

NAMESPACE_UPP

WorkPane::WorkPane() {
	Add(vsplit.SizePos());
	
	hsplit.Horz(task_details, plan_steps);
	vsplit.Vert(hsplit, breadcrumbs);
	vsplit.SetPos(4000);
	
	plan_steps.AddColumn("Step");
	plan_steps.AddColumn("Action");
	
	Add(btn_refresh.LeftPos(10, 100).BottomPos(10, 30));
	btn_refresh.SetLabel("Refresh");
	btn_refresh << [=] { Refresh(); };
	
	Add(btn_approve.RightPos(120, 100).BottomPos(10, 30));
	btn_approve.SetLabel("Approve");
	btn_approve.SetImage(CtrlImg::save());
	btn_approve << THISBACK(OnApprove);
	
	Add(btn_reject.RightPos(10, 100).BottomPos(10, 30));
	btn_reject.SetLabel("Reject");
	btn_reject.SetImage(CtrlImg::remove());
	btn_reject << THISBACK(OnReject);
}

void WorkPane::Load(const String& root) {
	current_root = root;
	Refresh();
}

void WorkPane::Refresh() {
	task_details.SetQTF("[* No active work session]");
	plan_steps.Clear();
	breadcrumbs.SetQTF("");
	
	// Find latest active session
	Array<WorkSession> sessions = WorkSessionManager::ListSessions(current_root);
	if(sessions.IsEmpty()) return;
	
	// Sort by created time descending
	// (Simple linear search for demo)
	const WorkSession* latest = nullptr;
	for(const auto& s : sessions) {
		if(s.status == WorkSessionStatus::RUNNING) {
			if(!latest || s.created > latest->created)
				latest = &s;
		}
	}
	
	if(latest) {
		active_session_id = latest->session_id;
		task_details.SetQTF("[*@3 Active Session: " + DeQtf(latest->session_id) + "]&[* Type:] " + latest->session_type + "&[* Purpose:] " + DeQtf(latest->purpose));
		
		// Load breadcrumbs
		Array<Breadcrumb> bc_list = BreadcrumbManager::ListBreadcrumbs(current_root, active_session_id);
		String b_qtf;
		for(const auto& bc : bc_list) {
			b_qtf << "[* " << bc.timestamp_id << "] (" << bc.model_used << ")&";
			b_qtf << DeQtf(bc.response) << "&";
			b_qtf << "------------------------------------------&";
		}
		breadcrumbs.SetQTF(b_qtf);
	}
}

void WorkPane::OnApprove() {
	if(active_session_id.IsEmpty()) return;
	PromptOK("Plan approved. Proceeding with execution (Stub).");
}

void WorkPane::OnReject() {
	if(active_session_id.IsEmpty()) return;
	PromptOK("Plan rejected. Please provide feedback in the Chat.");
}

END_UPP_NAMESPACE
