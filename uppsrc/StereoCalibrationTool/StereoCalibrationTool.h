#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>

NAMESPACE_UPP

struct StereoCalibrationTool : TopWindow {
	MenuBar menu;
	TabCtrl tabs;
	ParentCtrl source_tab;
	ParentCtrl calibration_tab;
	Label source_info;
	Label calibration_info;

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();

	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

END_UPP_NAMESPACE

#endif
