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


	servers.AddColumn("Name", 300);
	servers.AddColumn("Address", 200);
	servers.AddColumn("Status", 100);
	servers.AddIndex("IDX");
	servers.WhenCursor = THISBACK(OnServer);
	servers.WhenBar = [=](Bar& menu) { OnServerBar(menu); };

	projects.AddColumn("Name", 200);
	projects.AddColumn("Server", 200);
	projects.AddColumn("Git Origin", 300);
	projects.AddIndex("IDX");
	projects.WhenCursor = THISBACK(OnProject);
	projects.WhenBar = [=](Bar& menu) { OnProjectBar(menu); };

	statusbar.Set("Ready");
	
	// View 1
	server_vsplit.Vert(server_list, server_bottom);
	server_list.AddColumn("Name", 300); // TODO set column names - DONE
	server_list.AddColumn("Address", 200);
	server_list.AddColumn("Status", 100);
	server_list.AddIndex("IDX");

	// View 2
	prj_vsplit.Vert(prj_list, prj_bottom);
	prj_list.AddColumn("Name", 200);
	prj_list.AddColumn("Server", 200);
	prj_list.AddColumn("Git Origin", 300);
	prj_list.AddIndex("IDX");
	
	// View 3
	
	
	
	// Post-construct
	SetView(VIEW_QWEN_PROJECT);

	// Load existing state and populate lists
	PostCallback(THISBACK(Data));
}

void QwenManager::OnMenuBar(Bar& b) {
	b.Sub("App", [this](Bar& b) {
		b.Add(t_("Exit"), [this]{this->Close();});
	});
	b.Sub("View", [this](Bar& b) {
		b.Add("Detailed server-list", THISBACK1(SetView, VIEW_DETAILED_SERVERLIST)).Key(K_CTRL|K_F1);
		b.Add("Detailed pproject-list", THISBACK1(SetView, VIEW_DETAILED_PROJECTLIST)).Key(K_CTRL|K_F2);
		b.Add("Project view", THISBACK1(SetView, VIEW_QWEN_PROJECT)).Key(K_CTRL|K_F3);
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
	
	if (new_view == VIEW_DETAILED_SERVERLIST) {
		mainarea.Add(server_vsplit.SizePos());
		active_view = &server_vsplit;
	}
	else if (new_view == VIEW_DETAILED_PROJECTLIST) {
		mainarea.Add(prj_vsplit.SizePos());
		active_view = &prj_vsplit;
	}
	else if (new_view == VIEW_QWEN_PROJECT) {
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
	
	PostCallback(THISBACK(Data));
}

void QwenManager::Data() {
	QwenManagerState& state = QwenManagerState::Global();

	// Initialize with sample data if empty
	if (state.servers.GetCount() == 0) {
		// Add a sample server
		QwenServerConnectionConf& sample_server = state.servers.Add();
		sample_server.name = "Local Qwen Server";
		sample_server.host = "localhost";
		sample_server.port = 8765;
		sample_server.connection_type = "tcp";
		sample_server.directory = "~/Dev/qwen-code";
	}

	if (state.projects.GetCount() == 0) {
		// Add a sample project
		QwenProject& sample_project = state.projects.Add();
		sample_project.uniq = GetTickCount();
		sample_project.name = "Sample Project";
		sample_project.preferred_connection_name = "Local Qwen Server";
		sample_project.git_origin_addr = "https://github.com/example/sample-project.git";
	}

	UpdateProjectServerConnections();
	DataServerList();
	DataProjectList();

	if (view == VIEW_DETAILED_PROJECTLIST)
		DataDetailedProjectList();
	if (view == VIEW_DETAILED_SERVERLIST)
		DataDetailedServerList();
	if (view == VIEW_QWEN_PROJECT)
		if (active_qwen_view)
			active_qwen_view->Data();

	// Start auto-start servers with logging
	StartAutoStartServers();
}

void QwenManager::StartAutoStartServers() {
	QwenManagerState& state = QwenManagerState::Global();

	for(int i = 0; i < state.servers.GetCount(); i++) {
		auto& srv = state.servers[i];

		if (srv.auto_start) {
			LOG("Starting auto-start server: " << srv.name << " at " << srv.GetAddress());

			// Create a temporary connection to start the server
			QwenConnection temp_conn;
			temp_conn.Init(srv);
			if (temp_conn.Connect()) {
				LOG("Successfully connected to server: " << srv.name);
			} else {
				LOG("Failed to connect to server: " << srv.name);
			}
		}
	}
}

void QwenManager::DataDetailedProjectList() {

	// Fill prj_list by column names
	QwenManagerState& state = QwenManagerState::Global();

	for(int i = 0; i < state.projects.GetCount(); i++) {
		auto& prj = state.projects[i];
		prj_list.Set(i, 0, prj.name);
		prj_list.Set(i, 1, prj.preferred_connection_name);
		prj_list.Set(i, 2, prj.git_origin_addr);
		prj_list.Set(i, "IDX", i);
	}
	prj_list.SetCount(state.projects.GetCount());

	DataDetailedProject();
}

void QwenManager::DataDetailedProject() {

	// See focused project by prj_list and fill custom view data
	if (!prj_list.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int idx = prj_list.Get("IDX");
	if (idx >= 0 && idx < state.projects.GetCount()) {
		auto& prj = state.projects[idx];

		// Update status bar with detailed project information
		String detail_info = "Project: " + prj.name + " | Connection: " + prj.preferred_connection_name +
		                    " | Git: " + prj.git_origin_addr + " | Sessions: " + IntStr(prj.session_ids.GetCount());
		statusbar.Set(detail_info);
	}
}

void QwenManager::DataDetailedServerList() {

	// Fill server_list by column names
	QwenManagerState& state = QwenManagerState::Global();

	for(int i = 0; i < state.servers.GetCount(); i++) {
		auto& srv = state.servers[i];
		server_list.Set(i, 0, srv.name);
		server_list.Set(i, 1, srv.GetAddress());
		server_list.Set(i, 2, srv.GetStatusString());
		server_list.Set(i, "IDX", i);
	}
	server_list.SetCount(state.servers.GetCount());

	DataDetailedServer();
}

void QwenManager::DataDetailedServer() {

	// See focused server by server_list and fill custom view data
	if (!server_list.IsCursor()) return;

	QwenManagerState& state = QwenManagerState::Global();
	int idx = server_list.Get("IDX");
	if (idx >= 0 && idx < state.servers.GetCount()) {
		auto& srv = state.servers[idx];

		// Update status bar with detailed server information
		String detail_info = "Server: " + srv.name + " | Addr: " + srv.GetAddress() +
		                    " | Conn: " + (srv.is_connected ? "Yes" : "No") +
		                    " | Status: " + srv.GetStatusString();
		statusbar.Set(detail_info);
	}
}

void QwenManager::DataServerList() {
	QwenManagerState& state = QwenManagerState::Global();

	for(int i = 0; i < state.servers.GetCount(); i++) {
		QwenServerConnectionConf& srv = state.servers[i];
		servers.Set(i, 0, srv.name);
		servers.Set(i, 1, srv.GetAddress());
		servers.Set(i, 2, srv.GetStatusString());
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
		projects.Set(i, 2, prj.git_origin_addr);
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

	// Create dialog with layout
	WithEditServer<TopWindow> form;
	CtrlLayoutOKCancel(form, "Edit Server");
	form.name <<= srv.name;
	form.directory <<= srv.directory;
	form.host <<= srv.host;
	form.port <<= srv.port;
	form.connection_type <<= srv.connection_type;
	form.auto_start <<= srv.auto_start;  // Set auto-start checkbox

	if (form.Execute() == IDOK) {
		srv.name = ~form.name;
		srv.directory = ~form.directory;
		srv.host = ~form.host;
		srv.port = ~form.port;
		srv.connection_type = ~form.connection_type;
		srv.auto_start = ~form.auto_start; // Get auto-start checkbox value
		PostCallback(THISBACK(Data));
		statusbar.Set("Updated server: " + srv.name);
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

	// Create dialog with layout
	WithEditProject<TopWindow> form;
	CtrlLayoutOKCancel(form, "Edit Project");
	form.name <<= prj.name;
	form.preferred_connection_name <<= prj.preferred_connection_name;
	form.git_origin_addr <<= prj.git_origin_addr;

	if (form.Execute() == IDOK) {
		prj.name = ~form.name;
		prj.preferred_connection_name = ~form.preferred_connection_name;
		prj.git_origin_addr = ~form.git_origin_addr;
		PostCallback(THISBACK(Data));
		statusbar.Set("Updated project: " + prj.name);
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
