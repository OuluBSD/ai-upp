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
