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
// OcrRulePanel — display OCR rules and results

class OcrRulePanel : public DockableCtrl {
public:
	typedef OcrRulePanel CLASSNAME;

	OcrRulePanel();

	void SetRules(Vector<VsmOcrRule>* rules);
	void AddResult(const VsmOcrResult& result, const VsmOcrComparison& cmp);
	void ClearResults();

private:
	Vector<VsmOcrRule>* rules_ = nullptr;

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

private:
	Vector<VsmTemplateRule>* rules_ = nullptr;

	ArrayCtrl rules_list_;
	ArrayCtrl results_list_;
	Button    add_btn_, remove_btn_;
	Label     mode_lbl_, req_lbl_;
	DropList  mode_drop_, req_drop_;

	void OnAdd();
	void OnRemove();
	void RebuildRules();
};

// ---------------------------------------------------------------------------
// PipelineEditorPanel — edit preprocessing pipeline metadata

class PipelineEditorPanel : public DockableCtrl {
public:
	typedef PipelineEditorPanel CLASSNAME;

	PipelineEditorPanel();

	void SetPipeline(VsmPreprocessPipeline* pipeline);

	Event<> WhenPipelineChanged;

private:
	VsmPreprocessPipeline* pipeline_ = nullptr;

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

private:
	VsmAnnotationLayer* layer_  = nullptr;

	ArrayCtrl list_;
	Button    create_btn_, delete_btn_, save_btn_;
	EditString name_edit_;
	Label      name_lbl_, parent_lbl_;
	EditString parent_edit_;
	Label      rect_lbl_;
	EditInt    x_edit_, y_edit_, w_edit_, h_edit_;

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
	void Clear();

private:
	Label id_lbl_, source_lbl_, size_lbl_, created_lbl_, format_lbl_, assets_lbl_;
};

#endif
