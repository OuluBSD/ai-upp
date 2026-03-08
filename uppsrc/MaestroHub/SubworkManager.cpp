#include "MaestroHub.h"

NAMESPACE_UPP

SubworkManagerDialog::SubworkManagerDialog() {
	CtrlLayout(*this, "Subwork Manager");
	
	subwork_tree.AddColumn("Session", 200);
	subwork_tree.AddColumn("Type", 100);
	
	context_stack.AddColumn("Level");
	context_stack.AddColumn("Session Type");
	context_stack.AddColumn("Purpose");
	
	btn_push << THISBACK(OnPush);
	btn_pop << THISBACK(OnPop);
	btn_close << [=] { Close(); };
}

void SubworkManagerDialog::Load(const String& maestro_root, const String& session_id) {
	root = maestro_root;
	active_session_id = session_id;
	UpdateUI();
}

void SubworkManagerDialog::UpdateUI() {
	subwork_tree.Clear();
	context_stack.Clear();
	
	if(active_session_id.IsEmpty()) return;
	
	// Build tree from root session upwards/downwards
	// For now, simple visualization of the current "stack" based on parents
	
	String current = active_session_id;
	Vector<WorkSession> stack;
	
	while(!current.IsEmpty()) {
		String path = WorkSessionManager::FindSessionPath(root, current);
		WorkSession s;
		if(WorkSessionManager::LoadSession(path, s)) {
			stack.Insert(0, s);
			current = s.parent_session_id;
		} else break;
	}
	
	int root_node = subwork_tree.Add(0, CtrlImg::Dir(), "Sessions");
	int parent = root_node;
	
	for(int i = 0; i < stack.GetCount(); i++) {
		const auto& s = stack[i];
		parent = subwork_tree.Add(parent, CtrlImg::Dir(), s.session_id);
		subwork_tree.SetRowValue(parent, 1, s.session_type);
		context_stack.Add(i, s.session_type, s.purpose);
	}
	
	subwork_tree.Open(root_node);
}

void SubworkManagerDialog::OnPush() {
	PromptOK("Push Subwork: This would start a child session with inherited context.");
}

void SubworkManagerDialog::OnPop() {
	if(active_session_id.IsEmpty()) return;
	if(PromptYesNo("Pop Subwork: Mark current session as completed and return to parent?")) {
		// Logic to update status and notify parent
		PromptOK("Popped. (Stub)");
	}
}

END_UPP_NAMESPACE