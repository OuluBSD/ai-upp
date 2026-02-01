#ifndef _StereoCalibrationTool_StageB_h_
#define _StereoCalibrationTool_StageB_h_

NAMESPACE_UPP

/*
StageB.h
--------
Purpose:
- Stage B solver UI. Runs the intrinsics solve and shows math/report output.

Key classes:
- StageBWindow: TopWindow with solver controls and report panes.

Data flow:
- Reads captured frames + Stage A params from AppModel.
- Writes solved calibration into AppModel.last_calibration.
- Updates AppModel.report_text and AppModel.math_text.

Gotchas / invariants:
- Requires at least 5 match pairs across all frames.
- Stage B assumes Stage A extrinsics are the starting point.
*/

class StageBWindow : public TopWindow {
public:
	typedef StageBWindow CLASSNAME;
	StageBWindow();

	void Init(AppModel& model);
	void RefreshFromModel();

	// Shared solver entry point (also used by headless controller).
	bool SolveCalibration();

private:
	AppModel* model = nullptr;

	Button solve_calibration;
	Option verbose_math_log;
	Option stage_b_compare_basic;
	Button load_calibration;
	Button export_calibration;
	Button deploy_calibration;
	DocEdit report_text;
	DocEdit math_text;
	StatusBar status;
	LabelBox sep_calib;
	Label calibration_preview;

	void BuildLayout();
	void OnSolve();
	void OnReviewChanged();
	void LoadCalibration();
	void ExportCalibration();
	void DeployCalibration();
	void SyncCalibrationFromEdits();
	void SyncEditsFromCalibration();
	void SaveProjectState();
};

END_UPP_NAMESPACE

#endif
