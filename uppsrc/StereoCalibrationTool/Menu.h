#ifndef _StereoCalibrationTool_Menu_h_
#define _StereoCalibrationTool_Menu_h_

NAMESPACE_UPP

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

class CameraWindow;
class CalibrationWindow;
class MainCalibWindow;

// ------------------------------------------------------------

class MenuWindow : public TopWindow {
public:
	typedef MenuWindow CLASSNAME;
	MenuWindow();

	// Ownership model: windows are owned by the controller; Menu only shows/hides them.
	void Init(AppModel& model,
	          CameraWindow& camera,
	          CalibrationWindow& calib,
	          MainCalibWindow& main_win);
	void RefreshFromModel();

	virtual void MainMenu(Bar& bar);

private:
	AppModel* model = nullptr;
	
	MenuBar menu;

	CameraWindow* camera = nullptr;
	CalibrationWindow* calib = nullptr;
	MainCalibWindow* main_win = nullptr;

	Label title;
	Label project_lbl;
	Button open_camera;
	Button open_calib;
	Button open_unified;
	Button open_all;

	void BuildLayout();
	void RefreshProjectLabel();
	void OpenCamera();
	void OpenCalibration();
	void OpenUnified();
	void OpenAll();
	
	void SubMenuHelp(Bar& bar);
};

END_UPP_NAMESPACE

#endif
