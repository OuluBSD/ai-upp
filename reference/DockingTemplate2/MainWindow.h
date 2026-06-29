#ifndef _DockingTemplate2_MainWindow_h_
#define _DockingTemplate2_MainWindow_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>

using namespace Upp;

#include "AppState.h"
#include "EditMode.h"
#include "DebugTab.h"
#include "DockViews.h"

class MainWindow : public DockWindow {
public:
	typedef MainWindow CLASSNAME;

	MainWindow();
	void DockInit() override;
	void Close() override;

	void Log(const String& msg) { log_.Add(APPLOG_LEVEL_INFO, "App", msg); }

private:
	// ---- Structured log (owned here; wired to debug_tab_ in DockInit)
	AppLog log_;

	// ---- Frames
	MenuBar menu_;
	ToolBar toolbar_;

	// ---- Main area
	TabCtrl    main_tabs_;
	ParentCtrl tab1_area_;
	ParentCtrl tab2_area_;
	DebugLog   debug_tab_;
	Label      tab1_label_;
	Label      tab2_label_;

	// ---- Dock panels (DockableCtrl-derived members)
	DockViewA dock_a_;
	DockViewB dock_b_;
	DockViewC dock_c_;

	// ---- Mode and state
	ModeManager mode_;
	bool        loaded_       = false;
	int         current_tab_  = 0;
	String      default_layout_data_;
	AppRegistry registry_;

	// ---- Dock lifecycle
	void InitDockers();
	void OnResetDockLayout();
	void CacheDefaultLayout();
	void SetDefaultLayout();
	void SetDockVisible(DockableCtrl& dc, bool visible);

	// ---- Persistence
	bool LoadUserLayout();
	void SaveUserLayout();
	void LoadAppState();
	void SaveAppState();
	void InitRegistry();

	// ---- Tab/toolbar coordination
	void UpdateToolBar(Bar& bar);
	void OnMainTabChanged();

	// ---- Per-tab toolbar builders (also used by Edit menu for mirroring)
	void ToolBarTab1(Bar& bar);
	void ToolBarTab2(Bar& bar);
	void ToolBarTabDebug(Bar& bar);

	// ---- Menu callbacks
	void MainMenu(Bar& bar);
	void MenuApp(Bar& bar);
	void MenuEdit(Bar& bar);
	void MenuView(Bar& bar);
	void MenuWindows(Bar& bar);
	void MenuModes(Bar& bar);
	void MenuHelp(Bar& bar);

	// ---- Mode actions
	void SetViewMode() { mode_.SetMode(ModeManager::MODE_VIEW); }
	void SetEditMode() { mode_.SetMode(ModeManager::MODE_EDIT); }

	// ---- Layout actions
	void OnSaveLayoutAs();
	void OnLoadLayout();
};

#endif
