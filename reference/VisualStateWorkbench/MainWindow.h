#ifndef _VisualStateWorkbench_MainWindow_h_
#define _VisualStateWorkbench_MainWindow_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

#include "AppState.h"
#include "DebugTab.h"
#include "FrameCanvas.h"
#include "DockPanels.h"

class MainWindow : public DockWindow {
public:
	typedef MainWindow CLASSNAME;

	MainWindow();
	void DockInit() override;
	void Close() override;

	void Log(const String& msg) { LogInfo(log_, "App", msg); }

private:
	// ---- Structured log
	AppLog log_;

	// ---- Frames
	MenuBar menu_;
	ToolBar toolbar_;

	// ---- Main tabs
	TabCtrl     main_tabs_;
	FrameCanvas frame_canvas_;     // "Frame" tab
	ParentCtrl  regions_area_;     // "Regions" tab
	ArrayCtrl   regions_list_;     // inside regions_area_
	DebugLog    debug_tab_;        // "Debug" tab

	// ---- Dock panels (DockableCtrl subclasses)
	RegionPropsPanel      props_dock_;
	ReplayTimelinePanel   timeline_dock_;
	SessionInfoPanel      session_dock_;
	AnnotationEditorPanel annotation_dock_;
	PipelineEditorPanel   pipeline_dock_;
	TemplateRulePanel     template_dock_;

	// ---- Session storage + annotation layer + pipeline + template rules
	VsmSessionStore           session_store_;
	VsmAnnotationLayer        annotation_layer_;
	String                    annotation_path_;
	VsmPreprocessPipeline     current_pipeline_;
	Vector<VsmTemplateRule>   template_rules_;

	// ---- AppRegistry + state
	AppRegistry registry_;
	bool        loaded_      = false;
	int         current_tab_ = 0;
	String      default_layout_data_;

	// ---- VsmReplaySession
	VsmReplaySession replay_;

	// ---- Dock lifecycle
	void InitDockers();
	void OnResetDockLayout();
	void CacheDefaultLayout();
	void SetDefaultLayout();

	// ---- Persistence
	bool LoadUserLayout();
	void SaveUserLayout();
	void LoadAppState();
	void SaveAppState();
	void InitRegistry();

	// ---- Session
	void LoadSampleSession();
	void OnStep();
	void OnRunAll();
	void OnResetReplay();
	void RefreshAfterStep();
	void RebuildRegionsList();

	// ---- Tab/toolbar
	void UpdateToolBar(Bar& bar);
	void OnMainTabChanged();

	// ---- Menu callbacks
	void MainMenu(Bar& bar);
	void MenuFile(Bar& bar);
	void MenuView(Bar& bar);
	void MenuWindows(Bar& bar);

	// ---- Region selection
	void OnRegionSelected(const String& id);
	void OnRegionListSel();

	// ---- Annotation
	void LoadSampleAnnotation();
	void OnAnnotationChanged();
};

#endif
