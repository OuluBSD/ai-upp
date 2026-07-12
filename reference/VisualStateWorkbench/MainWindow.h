#ifndef _VisualStateWorkbench_MainWindow_h_
#define _VisualStateWorkbench_MainWindow_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

#include "AppState.h"
#include "DebugTab.h"
#include "TexasHoldemSessionAdapter.h"
#include "TexasHoldemLayoutBindingAdapter.h"
#include "TexasHoldemLogicStateAdapter.h"
#include "TexasHoldemMismatchAdapter.h"
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
	// LEFT dock is split into two tiers (see OnResetDockLayout()):
	//   Primary (own pane, visible by default): annotation_dock_, model_dock_.
	//   Secondary (one shared tabbed dock slot, "Rules & Preprocessing", not
	//   the default front tab): pipeline_dock_, template_dock_, ocr_dock_.
	RegionPropsPanel      props_dock_;
	ReplayTimelinePanel   timeline_dock_;
	SessionInfoPanel      session_dock_;
	AnnotationEditorPanel annotation_dock_;
	PipelineEditorPanel   pipeline_dock_;
	TemplateRulePanel     template_dock_;
	OcrRulePanel          ocr_dock_;
	ModelStatePanel       model_dock_;
	LayoutBindingPanel    layout_dock_;   // task 0132: .form layout-binding view
	LogicStatePanel       logic_dock_;    // task 0133: derived logic-state timeline view
	MismatchPanel         mismatch_dock_; // task 0134: ground-truth mismatch panel

	// ---- Session storage + annotation layer + pipeline + rules + model runtime
	VsmSessionStore           session_store_;
	VsmSessionStoreSource     src_source_;     // for sessions opened via OnOpenSession

	// active_session_ is the SINGLE source of truth for "which session is
	// active" — it generalizes task 0057's has_src_session_ bool from two
	// states to three, keeping exactly one authoritative notion (never two
	// flags that could disagree, which is the BUG-A/BUG-B class 0057 fixed):
	//   ACTIVE_SAMPLE   => built-in sample replay session (A, replay_)
	//   ACTIVE_IMPORTED => opened/imported .vsm/JPEG session (B, src_source_)
	//   ACTIVE_TEXAS    => opened TexasHoldem M01/M02 session (C, th_session_)
	// The toolbar Step/Run All, region lookups, Reset, Run Pipeline and
	// ground-truth flows all branch on this. Set in LoadSampleSession()
	// (SAMPLE), OpenSessionPath() (IMPORTED) and OpenTexasHoldemSession()
	// (TEXAS).
	enum ActiveSession { ACTIVE_SAMPLE, ACTIVE_IMPORTED, ACTIVE_TEXAS };
	ActiveSession             active_session_ = ACTIVE_SAMPLE;
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

	// ---- TexasHoldem M01/M02 session (C) — task 0131. Loaded via the
	// GUI-independent adapter (TexasHoldemSessionAdapter.h). th_step_pos_ is the
	// currently displayed frame id; th_frame_nodes_ are the region nodes for
	// that frame, in the same order the FrameCanvas overlay draws them (so a
	// canvas "region-N" click maps straight back to a node).
	VsmTexasHoldemSession        th_session_;
	int                          th_step_pos_ = 0;
	Vector<const VsmRegionNode*> th_frame_nodes_;

	// ---- Layout-binding model (task 0132 / M06-02). th_layout_model_ is the
	// session's frame-independent `.form` layout candidate set (built once on
	// open, GUI-independent adapter); th_frame_bindings_ are the current frame's
	// region->element/sub-slot bindings (recomputed per frame in SetTexasFrame).
	// th_form_path_ is the resolved GameTable_<provider>.form path (empty if
	// none was found, in which case the layout view shows "unavailable").
	VsmSessionLayoutModel        th_layout_model_;
	Vector<VsmLayoutBinding>     th_frame_bindings_;
	String                       th_form_path_;

	// ---- Logic-state timeline model (task 0133 / M06-03). th_logic_model_
	// holds one derived TexasHoldemLogicState per frame for the whole session
	// (built once on open, from th_form_path_ — the same resolved .form path
	// the layout-binding model already uses), computed by the GUI-independent
	// TexasHoldemLogicStateAdapter, which calls the shared
	// uppsrc/VisualStateLogic/LogicCompare.h M05 derivation pipeline (task
	// 0133's extraction of reference/VisualStateLogicCompare's own scoring/
	// derivation code) directly — no logic duplicated. logic_dock_ is
	// refreshed to the current frame in SetTexasFrame(), mirroring
	// th_frame_bindings_/layout_dock_'s own per-frame refresh.
	VsmSessionLogicModel         th_logic_model_;

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
	// TexasHoldem M01/M02 session source (task 0131): loads a real
	// --record-session directory via the headless adapter and makes it the
	// active session (C). SetTexasFrame() displays a given frame (real image +
	// that frame's changed-region overlay); RebuildTexasRegionsList() fills the
	// Regions tab with the per-transition region records.
	void OpenTexasHoldemSession(const String& path);
	void SetTexasFrame(int frame_id);
	void RebuildTexasRegionsList();
	// Clears any active TexasHoldem session display state (frame image, info
	// line, region overlay/list bookkeeping). Called when switching away to the
	// sample or an imported session.
	void ClearTexasSession();
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
	// task 0132: a click in the Layout Bindings panel drives the SAME region
	// selection wiring as a canvas/Regions-list click (reuses OnRegionSelected).
	void OnLayoutBindingSelected(int region_index);
	// task 0134: "Prev/Next Mismatch" in the Ground-Truth Mismatch panel drives
	// the SAME frame navigation SetTexasFrame() already provides (only valid
	// while a TexasHoldem session (C) is active).
	void OnMismatchJumpToFrame(int frame_id);

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
