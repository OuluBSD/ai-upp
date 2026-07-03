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
#include "JpegSequenceImporter.h"

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
	OcrRulePanel          ocr_dock_;
	ModelStatePanel       model_dock_;

	// ---- Session storage + annotation layer + pipeline + rules + model runtime
	VsmSessionStore           session_store_;
	VsmSessionStoreSource     src_source_;     // for sessions opened via OnOpenSession
	// has_src_session_ is the single source of truth for "which session is
	// active": true => the opened/imported session (B, src_source_) is what
	// the toolbar/region controls act on; false => the built-in sample
	// replay session (A, replay_) is active. See LoadSampleSession(),
	// OpenSessionPath(), and OnResetReplay().
	bool                      has_src_session_ = false;
	int                       src_step_pos_    = 0; // frames read from src_source_ so far (Step/Run All bookkeeping)
	// The real current frame image of the active session, in the same
	// headless pixel-buffer shape VsmSessionStoreSource/VsmSessionStore
	// already read/store it in. Only opened/imported sessions (B) have real
	// per-frame image bytes (see RunJpegImport()/RunVsmImport() ->
	// SaveFrameImage()); the built-in sample session (A, replay_) has none,
	// so this stays empty whenever has_src_session_ is false. Mirrors
	// whatever FrameCanvas's own current_frame_ index refers to -- updated
	// at the same call sites frame_canvas_.SetFrame() is (see
	// RefreshAfterSourceStep(), OnJumpToFrame()) plus session
	// load/open/reset points where the active session identity changes.
	VsmImageBuffer            current_frame_img_;
	VsmAnnotationLayer        annotation_layer_;
	String                    annotation_path_;
	VsmPreprocessPipeline     current_pipeline_;
	Vector<VsmTemplateRule>   template_rules_;
	Vector<VsmOcrRule>        ocr_rules_;
	VsmModelRuntime           model_runtime_;

	// ---- AppRegistry + state
	AppRegistry registry_;
	bool        loaded_                    = false;
	bool        had_prior_session_state_   = false;
	int         current_tab_               = 0;
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
	void RefreshAfterSourceStep();
	void RebuildRegionsList();
	// Single File-menu entry point (replaces the old separate "Open Session…"/
	// "Import Image Sequence…"/"Load Sample Session"/"Load E2E Sample Session"
	// items): shows a small modal dialog with a source-type chooser and
	// dispatches to whichever existing load path matches the chosen type.
	void OnOpenImportSession();
	// Shared dispatch logic for the "Image sequence" source type: detect
	// .vsm vs .jpg/.png in src_dir and route to the matching importer. Used
	// by OnOpenImportSession() (formerly duplicated inline in the old
	// OnImportImageSequence() menu handler, now folded into this one helper).
	void DispatchImageSequenceImport(const String& src_dir);
	// Shared "current frame" accessor for the LEFT dock panels (PipelineEditorPanel,
	// TemplateRulePanel, OcrRulePanel): pushes current_frame_img_ to each via
	// their SetCurrentFrame(). Call whenever current_frame_img_ changes.
	void PushCurrentFrameToPanels();
	void RunVsmImport(const String& src_dir);
	void RunJpegImport(const String& src_dir);
	void RunJpegSmokeTest();
	void OpenSessionPath(const String& path);

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

	// ---- Pipeline runner
	void OnRunPipeline();
	void OnCompareGroundTruth();
	void OnLoadE2ESample();
	void OnJumpToFrame(int frame);

	// ---- Cache
	VsmPipelineCache pipeline_cache_;
	void OnClearCache();

	// ---- Overlay toggles
	void LoadOverlayState();
	void SaveOverlayState();
	void OnToggleOverlay(int which);
};

#endif
