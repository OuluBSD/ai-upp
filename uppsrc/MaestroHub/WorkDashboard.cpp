#include "MaestroHub.h"

NAMESPACE_UPP

WorkPane::WorkPane() {
	btn_approve.SetLabel("Approve Plan");
	btn_approve << THISBACK(OnApprove);
	btn_reject.SetLabel("Reject / Feedback");
	btn_reject << THISBACK(OnReject);
	btn_subwork.SetLabel("Subwork...");
	btn_subwork << THISBACK(OnSubwork);
	btn_refresh.SetLabel("Refresh");
	btn_refresh << THISBACK(Refresh);
	
	Add(vsplit.SizePos());
	vsplit.Vert(hsplit, breadcrumbs);
	hsplit.Horz(task_details, plan_steps);
	
	Add(btn_approve.BottomPos(8, 32).LeftPos(8, 150));
	Add(btn_reject.BottomPos(8, 32).LeftPos(164, 150));
	Add(btn_subwork.BottomPos(8, 32).LeftPos(320, 150));
	Add(btn_refresh.BottomPos(8, 32).RightPos(8, 100));
}

void WorkPane::OnSubwork() {
	if(active_session_id.IsEmpty()) {
		PromptOK("No active session to manage subwork for.");
		return;
	}
	SubworkManagerDialog dlg;
	dlg.Run();
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
	Array<WorkSession> sessions = pick(WorkSessionManager::ListSessions(current_root));
	if(sessions.IsEmpty()) return;
	
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
		Array<Breadcrumb> bc_list = pick(BreadcrumbManager::ListBreadcrumbs(current_root, active_session_id));
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