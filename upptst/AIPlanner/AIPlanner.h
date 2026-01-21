#ifndef _AIPlanner_AIPlanner_h_
#define _AIPlanner_AIPlanner_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

using namespace Upp;

class AIPlanner : public TopWindow {
	MenuBar    menu;
	TabCtrl    tabs;
	
	// Tracks Tab
	Splitter   track_split;
	TreeCtrl   track_tree;
	RichTextCtrl track_detail;
	
	// Runbooks Tab
	Splitter   runbook_split;
	ArrayCtrl  runbook_list;
	RichTextCtrl runbook_detail;
	
	// Workflows Tab
	Splitter   workflow_split;
	ArrayCtrl  workflow_list;
	RichTextCtrl workflow_detail;
	
	Array<Track> tracks;
	Array<Runbook> runbooks;
	Array<WorkGraph> workgraphs;
	
	RecentConfig config;
	String       current_root;
	
	void OnSelectRoot();
	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	
	void LoadData();
	void OnTrackSelect();
	void OnRunbookSelect();
	void OnWorkflowSelect();

public:
	typedef AIPlanner CLASSNAME;
	AIPlanner();
};

#endif
