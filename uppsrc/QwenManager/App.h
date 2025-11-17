#ifndef _QwenManager_App_h_
#define _QwenManager_App_h_


class QwenManager : public TopWindow {
	Splitter hsplit;
	Splitter lvsplit;
	Ctrl mainarea;
	Ptr<Ctrl> active_view; // pointer to active view
	Ptr<QwenProjectView> active_qwen_view;
	int view = -1;
	MenuBar menubar;
	StatusBar statusbar;
	
	// Left vertical splitter area
	ArrayCtrl servers, projects;

	// View 1: detailed serverlist
	Splitter server_vsplit;
	ArrayCtrl server_list;
	Ctrl server_bottom;
	
	// View 2: detailed project list
	Splitter prj_vsplit;
	ArrayCtrl prj_list;
	Ctrl prj_bottom;
	
	// View 3: horizontal split of qwen-view and ssh terminal
	ArrayMap<int64,QwenProjectView> qwen_views; // based on QwenProject::uniq

	enum {
		VIEW_DETAILED_SERVERLIST,
		VIEW_DETAILED_PROJECTLIST,
		VIEW_QWEN_PROJECT,
	};

	void SetView(int i);

public:
	typedef QwenManager CLASSNAME;
	QwenManager();

	void Data();
	void DataServerList();
	void DataProjectList();
	void UpdateProjectServerConnections();
	void OnServer();
	void OnProject();
	void OnMenuBar(Bar& b);
	void OnServerBar(Bar& menu);
	void OnProjectBar(Bar& menu);
	void OnServerContext();
	void OnProjectContext();
	void NewServer();
	void EditServer();
	void DeleteServer();
	void NewProject();
	void EditProject();
	void DeleteProject();
};


#endif
