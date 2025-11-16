#include "QwenManager.h"

NAMESPACE_UPP

QwenManager::QwenManager() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lvsplit << mainarea;
	hsplit.SetPos(1250);
	
	lvsplit.Vert() << servers << projects;
	lvsplit.SetPos(3333);
	
	
	servers.AddColumn("Address");
	servers.AddColumn("Status");
	servers.ColumnWidths("2 1");
	servers.AddIndex("IDX");
	servers.WhenCursor = THISBACK(OnServer);
	
	projects.AddColumn("Name");
	projects.AddColumn("Server");
	projects.AddIndex("IDX");
	projects.ColumnWidths("2 1");
	projects.WhenCursor = THISBACK(OnProject);
	
	SetView(VIEW_QWEN_PROJECT);
}

void QwenManager::SetView(int i) {
	if (view == i)
		return;
	
	if (active_view) {
		mainarea.RemoveChild(active_view);
		active_view = 0;
	}
	
	if(i == VIEW_QWEN_PROJECT) {
		QwenManagerState& state = QwenManagerState::Global();
		int i = projects.Get("IDX");
		auto& prj = state.projects[i];
		ASSERT(prj.uniq >= 0);
		auto& qwen_view = qwen_views.GetAdd(prj.uniq);
		qwen_view.prj = &prj;
		mainarea.Add(qwen_view.SizePos());
		active_view = &qwen_view;
		active_qwen_view = &qwen_view;
	}
	view = i;
}

void QwenManager::Data() {
	DataServerList();
	DataProjectList();
	
	if (active_view == (Ctrl*)&active_qwen_view)
		active_qwen_view->Data();
}

void QwenManager::DataServerList() {
	QwenManagerState& state = QwenManagerState::Global();
	
	for(int i = 0; i < state.servers.GetCount(); i++) {
		QwenServerConnectionConf& srv = state.servers[i];
		servers.Set(i, 0, srv.GetLabel());
		servers.Set(i, 1, srv.GetStatusString());
		servers.Set(i, "IDX", i);
	}
	servers.SetCount(state.servers.GetCount());
	if (servers.GetCount() == 0 && servers.GetCount())
		servers.SetCursor(0);
	
}

void QwenManager::DataProjectList() {
	QwenManagerState& state = QwenManagerState::Global();
	
	for(int i = 0; i < state.projects.GetCount(); i++) {
		auto& prj = state.projects[i];
		projects.Set(i, 0, prj.name);
		projects.Set(i, 1, prj.srv ? prj.srv->GetAddress() : String());
		projects.Set(i, "IDX", i);
	}
	projects.SetCount(state.projects.GetCount());
	if (projects.GetCount() == 0 && projects.GetCount())
		projects.SetCursor(0);
	
}

void QwenManager::OnServer() {
	
	SetView(VIEW_SERVER);
	PostCallback(THISBACK(Data));
}

void QwenManager::OnProject() {
	// Basic requirements for this function
	if (!projects.IsCursor()) // any row in list must be selected
		return;
	
	// Get reference to the project which is selected in the project list
	QwenManagerState& state = QwenManagerState::Global();
	int i = projects.Get("IDX");
	auto& prj = state.projects[i];
	
	// Do indirect data updating
	SetView(VIEW_QWEN_PROJECT);
	PostCallback(THISBACK(Data));
}


END_UPP_NAMESPACE
