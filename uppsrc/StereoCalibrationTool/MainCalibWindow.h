#ifndef _StereoCalibrationTool_MainCalibWindow_h_
#define _StereoCalibrationTool_MainCalibWindow_h_

NAMESPACE_UPP

struct AppModel;

class MainCalibWindow : public TopWindow {
public:
	typedef MainCalibWindow CLASSNAME;
	MainCalibWindow();

	void Init(AppModel& model);
	void RefreshFromModel();

private:
	AppModel* model = nullptr;

	MenuBar menu;
	StatusBar status;

	CameraPane camera_pane;
	CalibrationPane calib_pane;
	TabCtrl tabs;

	void BuildLayout();
	void MainMenu(Bar& bar);
	void SubMenuFile(Bar& bar);
	void SubMenuHelp(Bar& bar);
};

END_UPP_NAMESPACE

#endif
