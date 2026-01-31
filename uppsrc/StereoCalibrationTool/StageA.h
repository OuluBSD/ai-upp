#ifndef _StereoCalibrationTool_StageA_h_
#define _StereoCalibrationTool_StageA_h_

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

private:
	AppModel* model = nullptr;

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
	Label barrel_lbl, fov_lbl;
	EditDoubleSpin barrel_strength, fov_deg;
	DocEdit basic_params_doc;
	Button yaw_center_btn, pitch_center_btn;
    Label view_mode_lbl;
    DropList view_mode_list;
    Option overlay_eyes;
    Option overlay_swap;
    Option show_difference;
    Option show_epipolar;
	Label alpha_lbl;
	SliderCtrl alpha_slider;
	ParentCtrl controls;

	// Capture + match lists.
	ArrayCtrl captures_list;
	ArrayCtrl matches_list;
	EditDouble dist_l_editor;
	EditDouble dist_r_editor;
	Splitter captures_split;
	Splitter main_split;
	Splitter preview_split;
	Splitter right_split;
	Splitter list_split;

	// Plotters (per-eye, not shared with other modules).
	ImageCtrl left_plot;
	ImageCtrl right_plot;

	StatusBar status;

	LensPoly preview_lens;
	Size preview_lens_size = Size(0, 0);
	vec4 preview_lens_poly = vec4(0, 0, 0, 0);
	float preview_lens_outward = 0;
	vec2 preview_lens_pp = vec2(0, 0);
	vec2 preview_lens_tilt = vec2(0, 0);

	void BuildLayout();
	void BuildStageAControls();
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
	void OnReviewChanged();
	void OnYawCenter();
	void OnPitchCenter();
	void OnCaptureSelection();
	void OnMatchEdited();
	void SaveProjectState();
};

#endif
