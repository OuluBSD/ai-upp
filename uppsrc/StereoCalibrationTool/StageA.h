#ifndef _StereoCalibrationTool_StageA_h_
#define _StereoCalibrationTool_StageA_h_

NAMESPACE_UPP

/*
StageA.h
--------
Purpose:
- Stage A basic alignment UI and match picking.
- Displays captured frame list and independent plotters for left/right eye.

Key classes:
- StageAWindow: TopWindow for Stage A controls and preview plotting.

Data flow:
- Reads captured frames from AppModel.
- Writes Stage A parameters into AppModel.project_state.

Gotchas / invariants:
- Stage A does NOT own camera start/stop or capture.
- When yaw/pitch/roll are all zero, preview should be identity.
*/

class StageAWindow : public TopWindow {
public:
	typedef StageAWindow CLASSNAME;
	StageAWindow();

	void Init(AppModel& model);
	void RefreshFromModel();
	
	virtual void MainMenu(Bar& bar);

private:
	AppModel* model = nullptr;
	
	MenuBar menu;

	// Stage A controls.
	Label calib_eye_lbl;
	EditDoubleSpin calib_eye_dist;
	LabelBox eye_l_group, eye_r_group;
	Label yaw_l_lbl, pitch_l_lbl, roll_l_lbl;
	Label yaw_r_lbl, pitch_r_lbl, roll_r_lbl;
	EditDoubleSpin yaw_l, pitch_l, roll_l;
	EditDoubleSpin yaw_r, pitch_r, roll_r;
	Option preview_extrinsics;
	Option preview_intrinsics;
	Option tint_overlay;
	Option show_crosshair;
	Label barrel_lbl, fov_lbl;
	EditDoubleSpin barrel_strength, fov_deg;
	Label cx_lbl, cy_lbl, k1_lbl, k2_lbl;
	EditDoubleSpin lens_cx, lens_cy, lens_k1, lens_k2;
	DocEdit basic_params_doc;
	Label tool_lbl;
	DropList tool_list;
	Button undo_btn;
    Label view_mode_lbl;
    DropList view_mode_list;
    Option overlay_eyes;
    Option overlay_swap;
    Option show_difference;
    Option show_epipolar;
	// Diagnostics
	LabelBox diagnostics_group;
	Option show_epipolar_lines;
	Option show_curvature_error;
	
	// Genetic Optimizer UI
	LabelBox ga_group;
	Button ga_start;
	Button ga_stop;
	Button ga_apply;
	Label ga_status_lbl;
	Label ga_cost_lbl;
	Label ga_diag_lbl; // New diagnostics label
	Label ga_pop_lbl, ga_gen_lbl;
	EditIntSpin ga_pop_edit;
	EditIntSpin ga_gen_edit;
	Option ga_use_all_frames;
	Option compare_ga_toggle; // Toggle to overlay GA result
	SliderCtrl ga_history_slider;
	Label ga_history_lbl;
	Vector<StereoCalibrationParams> ga_history;
	
	Label alpha_lbl;
	SliderCtrl alpha_slider;
	ParentCtrl controls;
	TabCtrl tab_data;
	ParentCtrl ga_tab_ctrl; // Container for GA controls

	// Capture + match lists.
	ArrayCtrl captures_list;
	ArrayCtrl matches_list;
	ArrayCtrl lines_list;
	EditDouble dist_l_editor;
	EditDouble dist_r_editor;
	Splitter captures_split;
	Splitter main_split;
	Splitter preview_split;
	Splitter list_split;
	Splitter details_split;

	// Plotters (per-eye, not shared with other modules).
	PreviewCtrl left_plot;
	PreviewCtrl right_plot;

	StatusBar status;

	LensPoly preview_lens;
	Size preview_lens_size = Size(0, 0);
	vec4 preview_lens_poly = vec4(0, 0, 0, 0);
	float preview_lens_outward = 0;
	vec2 preview_lens_pp = vec2(0, 0);
	vec2 preview_lens_tilt = vec2(0, 0);

	// Cached per-eye preview images (avoid big refactor; used for overlay/tint/crosshair)
	Image last_left_preview;
	Image last_right_preview;

	// Point picking state
	Pointf pending_left = Null;
	Pointf hover_point = Null; // Last hovered point (in rectified pixels)
	int hover_eye = -1;
	
	// Undo state (simple one-step for Stage A params)
	ProjectState undo_state;
	bool has_undo = false;

	void BuildLayout();
	void BuildStageAControls();
	void BuildGAPanel();
	void BuildCaptureLists();
	void BuildPlotters();
	void SyncStageA();
	void UpdatePreview();
	void UpdatePlotters();
	void UpdateReviewEnablement();
	bool PreparePreviewLens(const Size& sz, LensPoly& out_lens, vec2& out_tilt);
#if 0
	bool BuildUndistortCache(CapturedFrame& frame, const LensPoly& lens, float linear_scale); // DEPRECATED
#endif
	void ApplyPreviewImages(CapturedFrame& frame, const LensPoly& lens, float linear_scale);
	void ComposeFinalDisplayImages();
	void OnReviewChanged();
	void OnToolAction();
	void OnUndo();
	void OnPickMatchTool(int eye, Pointf p);
	void OnFinalizeLine(int eye, const Vector<Pointf>& chain);
	void OnHoverPoint(Pointf p_rect, int eye);
	void OnCapturesBar(Bar& bar);
	void OnDeleteCapture();
	void OnLinesBar(Bar& bar);
	void OnDeleteLine();
	void OnMatchesBar(Bar& bar);
	void OnDeleteMatch();
	void OnDeleteAllMatches();
	void OnDeleteAllLines();
	void OnCaptureSelection();
	void OnMatchEdited();
	void SaveProjectState();
	void PushUndo();
	
	// Menu handlers
	void SubMenuEdit(Bar& bar);
	
	// GA handlers
	void OnGAStart();
	void OnGAStop();
	void OnGAApply();
	void OnGAStep(int gen, double best_cost, StereoCalibrationParams best_p); // Callback from thread
	void OnGAFinished(); // Callback from thread
	void OnGAHistoryScrub();

private:
	struct CostPlot : public Ctrl {
		Vector<double> history;
		void AddValue(double v) { history.Add(v); Refresh(); }
		void Clear() { history.Clear(); Refresh(); }
		virtual void Paint(Draw& w) override {
			Size sz = GetSize();
			w.DrawRect(sz, White());
			if (history.IsEmpty()) return;
			double min_v = history[0], max_v = history[0];
			for(double v : history) { min_v = min(min_v, v); max_v = max(max_v, v); }
			if (abs(max_v - min_v) < 1e-9) max_v += 1.0;
			
			w.DrawText(2, 2, Format("Min: %.4f", min_v), Arial(10), Gray());
			
			Vector<Point> pts;
			for(int i=0; i<history.GetCount(); i++) {
				double x = (double)i / max(1, history.GetCount()-1) * sz.cx;
				// Invert Y because screen Y is down, but we want higher cost (bad) up? 
				// Usually plots: Y up. Screen: Y down.
				// Cost is minimization. So lower is better.
				// Let's put lower cost at BOTTOM (high Y) or TOP?
				// Standard plot: Y axis goes UP. So lower value = lower pixel (higher Y).
				// We want minimization curve to go DOWN visually.
				// So Max cost at Top (Y=0), Min cost at Bottom (Y=H).
				double norm = (history[i] - min_v) / (max_v - min_v);
				double y = (1.0 - norm) * (sz.cy - 10) + 5; // 1.0 (max val) -> 5 (top). 0.0 (min val) -> H-5 (bottom).
				// Wait, 1.0 - norm means:
				// If val = max, norm=1, y = 5 (top).
				// If val = min, norm=0, y = H-5 (bottom).
				// So curve goes DOWN as cost decreases. Correct.
				pts.Add(Point(x, y));
			}
			w.DrawPolyline(pts, 1, Blue());
			w.DrawText(2, sz.cy - 14, Format("Max: %.4f", max_v), Arial(10), Gray());
		}
	};
	CostPlot ga_plot;

	Thread ga_thread;
	bool ga_running = false;
	Atomic ga_cancel;
	ProjectState ga_best_state; // To store best result
	StereoCalibrationParams ga_best_params;
	Vector<StereoCalibrationMatch> ga_input_matches; // Staging for thread
};

END_UPP_NAMESPACE

#endif