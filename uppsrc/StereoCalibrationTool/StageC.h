#ifndef _StereoCalibrationTool_StageC_h_
#define _StereoCalibrationTool_StageC_h_

NAMESPACE_UPP

/*
StageC.h
--------
Purpose:
- Stage C micro-refine UI (extrinsics refinement).

Key classes:
- StageCWindow: TopWindow for Stage C settings + refine action.

Data flow:
- Reads Stage A and Stage B parameters from AppModel.
- Writes refined calibration into AppModel.last_calibration.

Gotchas / invariants:
- Should not run if Stage C is disabled.
*/

class StageCWindow : public TopWindow {
public:
	typedef StageCWindow CLASSNAME;
	StageCWindow();

	void Init(AppModel& model);
	void RefreshFromModel();
	bool RefineExtrinsics();

private:
	AppModel* model = nullptr;

	Option enable_stage_c;
	Label stage_c_mode_lbl;
	Switch stage_c_mode;
	Label max_dyaw_lbl, max_dpitch_lbl, max_droll_lbl;
	EditDoubleSpin max_dyaw, max_dpitch, max_droll;
	Label lambda_lbl;
	EditDoubleSpin lambda_edit;
	Button refine_btn;
	DocEdit report_text;
	DocEdit math_text;
	StatusBar status;

	void BuildLayout();
	void OnRefine();
	void SaveProjectState();
};

END_UPP_NAMESPACE

#endif
