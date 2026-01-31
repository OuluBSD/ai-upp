#ifndef _StereoCalibrationTool_Menu_h_
#define _StereoCalibrationTool_Menu_h_

/*
Menu.h
------
Purpose:
- Minimal launcher TopWindow. Owns NO calibration logic.
- Opens the other module windows (Camera/StageA/StageB/StageC/LiveResult).

Key classes:
- MenuWindow: A light-weight entry point with buttons.

Data flow:
- Receives references to the AppModel and other windows at Init().
- Does not mutate AppModel directly (ownership stays with modules).

Gotchas / invariants:
- Menu only launches windows; it must not capture frames or solve calibration.
*/

class MenuWindow : public TopWindow {
public:
	typedef MenuWindow CLASSNAME;
	MenuWindow();

	// Ownership model: windows are owned by the controller; Menu only shows/hides them.
	void Init(AppModel& model,
	          CameraWindow& camera,
	          StageAWindow& stage_a,
	          StageBWindow& stage_b,
	          StageCWindow& stage_c,
	          LiveResultWindow& live);
	void RefreshFromModel();

private:
	AppModel* model = nullptr;
	CameraWindow* camera = nullptr;
	StageAWindow* stage_a = nullptr;
	StageBWindow* stage_b = nullptr;
	StageCWindow* stage_c = nullptr;
	LiveResultWindow* live = nullptr;

	Label title;
	Label project_lbl;
	Button open_camera;
	Button open_stage_a;
	Button open_stage_b;
	Button open_stage_c;
	Button open_live;
	Button open_all;

	void BuildLayout();
	void RefreshProjectLabel();
	void OpenCamera();
	void OpenStageA();
	void OpenStageB();
	void OpenStageC();
	void OpenLive();
	void OpenAll();
};

#endif
