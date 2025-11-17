#include "QwenManager.h"

NAMESPACE_UPP

QwenManager::QwenManager() {
	MaximizeBox().MinimizeBox().Sizeable();
	Maximize();
	Title("Qwen Manager");
	
	AddFrame(menubar);
	AddFrame(statusbar);
	Add(hsplit.SizePos());
	
	menubar.Set(THISBACK(OnMenuBar));

	hsplit.Horz() << lvsplit << mainarea;
	hsplit.SetPos(1250);

	lvsplit.Vert() << servers << projects;
	lvsplit.SetPos(3333);


	servers.AddColumn("Address");
	servers.AddColumn("Status");
	servers.ColumnWidths("2 1");
	servers.AddIndex("IDX");
	servers.WhenCursor = THISBACK(OnServer);
	servers.WhenBar = [=](Bar& menu) { OnServerBar(menu); };

	projects.AddColumn("Name");
	projects.AddColumn("Server");
	projects.AddIndex("IDX");
	projects.ColumnWidths("2 1");
	projects.WhenCursor = THISBACK(OnProject);
	projects.WhenBar = [=](Bar& menu) { OnProjectBar(menu); };

	SetView(VIEW_QWEN_PROJECT);
	statusbar.Set("Ready");
	
	PostCallback(THISBACK(Data));
}

void QwenManager::OnMenuBar(Bar& b) {
	b.Sub("App", [this](Bar& b) {
		b.Add(t_("Exit"), [this]{this->Close();});
	});
	b.Sub("View", [this](Bar& b) {
		
	});
	b.Sub("Help", [this](Bar& b) {
		
	});
	
}

void QwenManager::SetView(int new_view) {
	if (view == new_view)
		return;
	
	if (active_view) {
		mainarea.RemoveChild(active_view);
		active_view = 0;
	}
	
	if(new_view == VIEW_QWEN_PROJECT) {
		QwenManagerState& state = QwenManagerState::Global();
		if (projects.IsCursor()) {
			int i = projects.Get("IDX");
			auto& prj = state.projects[i];
			ASSERT(prj.uniq >= 0);
			QwenProjectView& qwen_view = qwen_views.GetAdd(prj.uniq);
			qwen_view.prj = &prj;
			mainarea.Add(qwen_view.SizePos());
			active_view = &qwen_view;
			active_qwen_view = &qwen_view;
		}
	}
	view = new_view;
}

void QwenManager::Data() {
	UpdateProjectServerConnections();
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

	// Update server connections for all projects
	UpdateProjectServerConnections();

	for(int i = 0; i < state.projects.GetCount(); i++) {
		auto& prj = state.projects[i];

		projects.Set(i, 0, prj.name);
		projects.Set(i, 1, prj.preferred_connection_name);
		projects.Set(i, "IDX", i);
	}
	projects.SetCount(state.projects.GetCount());
	if (projects.GetCount() == 0 && projects.GetCount())
		projects.SetCursor(0);

}

void QwenManager::UpdateProjectServerConnections() {
	QwenManagerState& state = QwenManagerState::Global();

	for(int i = 0; i < state.projects.GetCount(); i++) {
		auto& prj = state.projects[i];

		// If the server connection pointer is null, find the corresponding server configuration
		if (!prj.srv) {
			for(int j = 0; j < state.servers.GetCount(); j++) {
				if(state.servers[j].name == prj.preferred_connection_name) {
					prj.srv = &state.servers[j];
					break;
				}
			}
		}
	}
}

void QwenManager::OnServer() {
	// Update status bar with server info
	if (servers.IsCursor()) {
		QwenManagerState& state = QwenManagerState::Global();
		int i = servers.Get("IDX");
		auto& srv = state.servers[i];
		statusbar.Set("Server: " + srv.GetLabel() + " | Status: " + srv.GetStatusString());
	} else {
		statusbar.Set("Server view selected");
	}

	SetView(VIEW_SERVER);
	PostCallback(THISBACK(Data));
}

void QwenManager::OnServerBar(Bar& menu) {
	menu.Add("New Server", THISBACK(NewServer));
	if (servers.IsCursor()) {
		menu.Separator();
		menu.Add("Edit Server", THISBACK(EditServer));
		menu.Add("Delete Server", THISBACK(DeleteServer));
	}
}

void QwenManager::OnProjectBar(Bar& menu) {
	menu.Add("New Project", THISBACK(NewProject));
	if (projects.IsCursor()) {
		menu.Separator();
		menu.Add("Edit Project", THISBACK(EditProject));
		menu.Add("Delete Project", THISBACK(DeleteProject));
	}
}

void QwenManager::NewServer() {
	QwenManagerState& state = QwenManagerState::Global();
	QwenServerConnectionConf& new_srv = state.servers.Add();
	new_srv.name = "New Server " + IntStr(state.servers.GetCount());
	PostCallback(THISBACK(Data));
	statusbar.Set("Created new server: " + new_srv.name);
}

void QwenManager::EditServer() {
	if (!servers.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int i = servers.Get("IDX");
	auto& srv = state.servers[i];
	
	WithEditServer<TopWindow> form;
	CtrlLayoutOKCancel(form, "Edit server");
	form.host.SetData(srv.host);
	form.port.SetData(srv.port);
	
	if (form.Execute() == IDOK) {
		srv.host = ~form.host;
		srv.port = ~form.port;
		DataServerList();
	}
}

void QwenManager::DeleteServer() {
	if (!servers.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int i = servers.Get("IDX");

	if (PromptYesNo("Delete server '" + state.servers[i].name + "'?")) {
		// Remove any projects that use this server
		for (int j = state.projects.GetCount() - 1; j >= 0; j--) {
			if (state.projects[j].preferred_connection_name == state.servers[i].name) {
				state.projects.Remove(j);
			}
		}

		state.servers.Remove(i);
		PostCallback(THISBACK(Data));
		statusbar.Set("Deleted server");
	}
}

void QwenManager::NewProject() {
	QwenManagerState& state = QwenManagerState::Global();
	QwenProject& new_prj = state.projects.Add();
	new_prj.name = "New Project " + IntStr(state.projects.GetCount());
	new_prj.uniq = GetTickCount(); // Use timestamp as unique ID
	PostCallback(THISBACK(Data));
	statusbar.Set("Created new project: " + new_prj.name);
}

void QwenManager::EditProject() {
	if (!projects.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int i = projects.Get("IDX");
	auto& prj = state.projects[i];

	WithEditProject<TopWindow> form;
	CtrlLayoutOKCancel(form, "Edit project");
	form.name.SetData(prj.name);
	form.preferred_connection_name.SetData(prj.preferred_connection_name);
	
	if (form.Execute() == IDOK) {
		prj.name = ~form.name;
		prj.preferred_connection_name = ~form.preferred_connection_name;
		DataProjectList();
	}
}

void QwenManager::DeleteProject() {
	if (!projects.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int i = projects.Get("IDX");

	if (PromptYesNo("Delete project '" + state.projects[i].name + "'?")) {
		state.projects.Remove(i);
		PostCallback(THISBACK(Data));
		statusbar.Set("Deleted project");
	}
}

void QwenManager::OnProject() {
	// Basic requirements for this function
	if (!projects.IsCursor()) // any row in list must be selected
		return;

	// Get reference to the project which is selected in the project list
	QwenManagerState& state = QwenManagerState::Global();
	int i = projects.Get("IDX");
	auto& prj = state.projects[i];

	// Update status bar with project info
	statusbar.Set("Active project: " + prj.name + " | Server: " +
	              (prj.srv ? prj.srv->GetAddress() : "None"));

	// Do indirect data updating
	SetView(VIEW_QWEN_PROJECT);
	PostCallback(THISBACK(Data));
}


END_UPP_NAMESPACE
