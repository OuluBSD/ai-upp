#ifndef _QwenManager_App_h_
#define _QwenManager_App_h_


class QwenManager : public TopWindow {
	Splitter hsplit;
	Splitter lvsplit;
	Ctrl mainarea;
	Ptr<Ctrl> active_view; // pointer to active view
	Ptr<QwenProjectView> active_qwen_view;
	int view = -1;
	
	// Left vertical splitter area
	ArrayCtrl servers, projects;
	
	// View 1: horizontal split of qwen-view and ssh terminal
	ArrayMap<int64,QwenProjectView> qwen_views; // based on QwenProject::uniq
	
	enum {
		VIEW_SERVER,
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
};


#endif
