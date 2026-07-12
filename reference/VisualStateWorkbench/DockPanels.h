#ifndef _VisualStateWorkbench_DockPanels_h_
#define _VisualStateWorkbench_DockPanels_h_

// ---------------------------------------------------------------------------
// RegionPropsPanel — shows properties of the selected region

class RegionPropsPanel : public DockableCtrl {
public:
	typedef RegionPropsPanel CLASSNAME;

	RegionPropsPanel();
	void SetRegion(const String& id, const VsmRegionNode* node);
	void Clear();

private:
	Label id_lbl_, rect_lbl_, fp_lbl_, action_lbl_;
};

// ---------------------------------------------------------------------------
// ReplayTimelinePanel — replay controls

class ReplayTimelinePanel : public DockableCtrl {
public:
	typedef ReplayTimelinePanel CLASSNAME;

	ReplayTimelinePanel();

	Event<> WhenStep;
	Event<> WhenRunAll;
	Event<> WhenReset;

	void SetProgress(int pos, int total);

private:
	Button step_btn_, run_btn_, reset_btn_;
	Label  progress_lbl_;
};

// ---------------------------------------------------------------------------
// ModelStatePanel — displays VsmModelRuntime state, transitions, divergences

class ModelStatePanel : public DockableCtrl {
public:
	typedef ModelStatePanel CLASSNAME;

	ModelStatePanel();

	void SetRuntime(VsmModelRuntime* rt);
	void Refresh();

	Event<int> WhenJumpToFrame;

private:
	VsmModelRuntime* rt_ = nullptr;

	TabCtrl     tabs_;
	ParentCtrl  objects_area_, transitions_area_, divergences_area_;
	ArrayCtrl   objects_list_, props_list_, transitions_list_, divergences_list_;
	Splitter    divergences_split_;
	DocEdit     divergence_detail_;
	Button      run_sample_btn_;

	void OnRunSample();
	void OnDivergenceSel();
};

// ---------------------------------------------------------------------------
// OcrRulePanel — display OCR rules and results

class OcrRulePanel : public DockableCtrl {
public:
	typedef OcrRulePanel CLASSNAME;

	OcrRulePanel();

	void SetRules(Vector<VsmOcrRule>* rules);
	void AddResult(const VsmOcrResult& result, const VsmOcrComparison& cmp);
	void ClearResults();

	// Pushed by MainWindow whenever the active session's current frame
	// changes (Step/Run All/jump/session open) — see FrameCanvas's own
	// notion of "current frame" in MainWindow.cpp. Empty when no real frame
	// is available yet.
	void SetCurrentFrame(const VsmImageBuffer& img);

private:
	Vector<VsmOcrRule>* rules_ = nullptr;
	int next_ocr_id_ = 0;  // monotonic counter for rule IDs
	VsmImageBuffer current_frame_;

	ArrayCtrl rules_list_;
	ArrayCtrl results_list_;
	Button    add_btn_, remove_btn_, run_btn_;
	EditString expected_edit_;
	Label      expected_lbl_, status_lbl_;

	void OnAdd();
	void OnRemove();
	void OnRun();
	void RebuildRules();
};

// ---------------------------------------------------------------------------
// TemplateRulePanel — display/edit template rules and show match results

class TemplateRulePanel : public DockableCtrl {
public:
	typedef TemplateRulePanel CLASSNAME;

	TemplateRulePanel();

	void SetRules(Vector<VsmTemplateRule>* rules);
	void AddMatchResult(const VsmTemplateMatchResult& res);

	// See OcrRulePanel::SetCurrentFrame() — same contract.
	void SetCurrentFrame(const VsmImageBuffer& img);

private:
	Vector<VsmTemplateRule>* rules_ = nullptr;
	int next_rule_id_ = 0;  // monotonic counter for rule IDs
	VsmImageBuffer current_frame_;

	ArrayCtrl rules_list_;
	ArrayCtrl results_list_;
	Button    add_btn_, remove_btn_, run_btn_;
	Label     mode_lbl_, req_lbl_, status_lbl_;
	DropList  mode_drop_, req_drop_;

	void OnAdd();
	void OnRemove();
	void OnRun();
	void RebuildRules();
};

// ---------------------------------------------------------------------------
// PipelineEditorPanel — edit preprocessing pipeline metadata

class PipelineEditorPanel : public DockableCtrl {
public:
	typedef PipelineEditorPanel CLASSNAME;

	PipelineEditorPanel();

	void SetPipeline(VsmPreprocessPipeline* pipeline);

	// See OcrRulePanel::SetCurrentFrame() — same contract.
	void SetCurrentFrame(const VsmImageBuffer& img);

	Event<> WhenPipelineChanged;

private:
	VsmPreprocessPipeline* pipeline_ = nullptr;
	VsmImageBuffer current_frame_;

	ArrayCtrl steps_list_;
	Button    add_gray_btn_, add_inv_btn_, add_thresh_btn_, add_norm_btn_, remove_btn_, run_btn_;
	Label     result_lbl_;

	void OnAdd(int type);
	void OnRemove();
	void OnRun();
	void RebuildList();

	static const char* StepName(int type);
};

// ---------------------------------------------------------------------------
// AnnotationEditorPanel — create/edit/delete region annotations

class AnnotationEditorPanel : public DockableCtrl {
public:
	typedef AnnotationEditorPanel CLASSNAME;

	AnnotationEditorPanel();

	// Called when the workbench loads/saves annotation layer
	void SetLayer(VsmAnnotationLayer* layer);

	Event<> WhenLayerChanged;

	// Called by MainWindow after OnAnnotationChanged() persists or fails to persist.
	// success=true if annotation_layer_.Save() completed successfully.
	// path_empty=true if annotation_path_ was empty at time of attempted save.
	void NotifySaveResult(bool success, bool path_empty);

private:
	VsmAnnotationLayer* layer_  = nullptr;

	ArrayCtrl list_;
	Button    create_btn_, delete_btn_, save_btn_;
	EditString name_edit_;
	Label      name_lbl_, parent_lbl_;
	EditString parent_edit_;
	Label      rect_lbl_;
	EditInt    x_edit_, y_edit_, w_edit_, h_edit_;
	Label      saved_status_lbl_;

	void OnCreate();
	void OnDelete();
	void OnSave();
	void OnSel();
	void RebuildList();
	void FillFields(const VsmRegionAnnotation* a);
	void ApplyFields();
};

// ---------------------------------------------------------------------------
// SessionInfoPanel — shows VsmSessionManifest fields

class SessionInfoPanel : public DockableCtrl {
public:
	typedef SessionInfoPanel CLASSNAME;

	SessionInfoPanel();
	void SetManifest(const VsmSessionManifest& m);
	// task 0131: populate from a TexasHoldem M01/M02 session's metadata.json
	// (reuses the same six labels; provider takes the "Source" slot).
	void SetTexasHoldemInfo(const VsmM01M02SessionInfo& info);
	void Clear();

private:
	Label id_lbl_, source_lbl_, size_lbl_, created_lbl_, format_lbl_, assets_lbl_;
};

// ---------------------------------------------------------------------------
// LayoutBindingPanel — visualizes M04's `.form`-driven layout model (task
// 0132): the parsed layout element/sub-slot candidates and, per frame, which
// detected changed region got matched to which element/sub-slot, with role /
// seat_index / card_index / overlap. All data is computed GUI-independently by
// TexasHoldemLayoutBindingAdapter (VsmSessionLayoutModel / VsmLayoutBinding),
// which reuses M04's shared FormLayout/LayoutProfile/RegionAssign functions.
// Only ever populated for an active TexasHoldem session (source C).

class LayoutBindingPanel : public DockableCtrl {
public:
	typedef LayoutBindingPanel CLASSNAME;

	LayoutBindingPanel();

	// Header info from the session's frame-independent layout model (form name,
	// design-space/frame size, scale, candidate counts). nullptr / unloaded
	// model shows the "unavailable" state.
	void SetModel(const VsmSessionLayoutModel* model);

	// Populate the per-region binding rows for the current frame.
	void SetFrameBindings(int frame_id, const Vector<VsmLayoutBinding>& bindings);

	void Clear();

	// Fires with the clicked binding's region_index (the same "region-N" index
	// FrameCanvas::WhenRegionSelected uses), so MainWindow can drive the SAME
	// existing region-selection wiring.
	Event<int> WhenBindingSelected;

private:
	const VsmSessionLayoutModel* model_ = nullptr;

	Label     info_lbl_, scale_lbl_, frame_lbl_;
	ArrayCtrl list_;
	Vector<int> row_region_index_; // list row -> region_index

	void OnSel();
};

#endif
