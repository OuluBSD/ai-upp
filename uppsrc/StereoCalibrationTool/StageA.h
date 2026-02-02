#ifndef _StereoCalibrationTool_StageA_h_
#define _StereoCalibrationTool_StageA_h_

#include <CtrlLib/CtrlLib.h>

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
	Button undo_btn;
	Option overlay_eyes;
	Option overlay_swap;
	Option show_difference;
	Option show_epipolar;
	Option rectified_overlay;
	Label rectify_alpha_lbl;
	SliderCtrl rectify_alpha_slider;
	// Board Settings
	Label board_x_lbl, board_y_lbl, board_sz_lbl;
	EditInt board_x, board_y;
	EditDouble board_size;
	Button detect_btn;
	Option lock_intrinsics;
	Option lock_baseline;
	Option lock_yaw_symmetry;
	
	// Coverage & Solve
	Label coverage_lbl;
	Label coverage_score_lbl;
	ImageCtrl coverage_heat;
	Button solve_int_btn;
	Button solve_stereo_btn;

	// Epipolar Metrics (displayed after stereo solve)
	Label epipolar_metric_lbl;
	Label epipolar_value_lbl;
	
	// Viewer
	Option show_corners;
	Option show_reprojection;
	
	// Report
	DocEdit report_log;

	Label alpha_lbl;
	SliderCtrl alpha_slider;
	ParentCtrl controls;
	TabCtrl tab_data;
	
	ParentCtrl tab_frames, tab_board, tab_solve, tab_report;

	// Capture list
	ArrayCtrl captures_list;
	
	Splitter main_split;
	Splitter preview_split;
	Splitter right_split;

	// Plotters (per-eye, not shared with other modules).
	PreviewCtrl left_plot;
	PreviewCtrl right_plot;

	StatusBar status;
	Label pipeline_state_lbl;

	LensPoly preview_lens;
	Size preview_lens_size = Size(0, 0);
	vec4 preview_lens_poly = vec4(0, 0, 0, 0);
	float preview_lens_outward = 0;
	vec2 preview_lens_pp = vec2(0, 0);
	vec2 preview_lens_tilt = vec2(0, 0);

	// Cached per-eye preview images (avoid big refactor; used for overlay/tint/crosshair)
	Image last_left_preview;
	Image last_right_preview;
	
	// Undo state (simple one-step for Stage A params)
	ProjectState undo_state;
	bool has_undo = false;

	void BuildLayout();
	void BuildStageAControls();
	void BuildTabs();
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
	void OnUndo();
	void OnCapturesBar(Bar& bar);
	void OnDeleteCapture();
	void OnCaptureSelection();
	void SaveProjectState();
	void PushUndo();
	
	void OnDetect();
	void OnSolveIntrinsics();
	void OnSolveStereo();
	void OnExportYaml();
	bool CheckPoseDiversity();
	void UpdateCoverageHeatmap();

	// Stereo rectification pipeline
	void ComputeStereoRectification(const cv::Mat& K1, const cv::Mat& D1,
	                                 const cv::Mat& K2, const cv::Mat& D2,
	                                 const cv::Mat& R, const cv::Mat& T,
	                                 const Size& img_sz);
	void BuildRectificationMaps();
	void RebuildRectificationFromState();  // Rebuild rectification from saved ProjectState
	void ComputeEpipolarMetrics(const cv::Mat& K1, const cv::Mat& D1,
	                             const cv::Mat& K2, const cv::Mat& D2,
	                             const cv::Mat& R, const cv::Mat& T);
	void UpdateEpipolarDisplay();
	
	// Menu handlers
	void SubMenuEdit(Bar& bar);
	void SubMenuHelp(Bar& bar);

private:
};

END_UPP_NAMESPACE

#endif