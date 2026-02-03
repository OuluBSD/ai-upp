#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
MainCalibWindow.cpp
===================
Purpose:
- Unified entry point for stereo calibration.
- Combines Camera capture and Calibration alignment/solve.

Data Flow:
1. Capture: CameraPane captures frames -> AppModel.captured_frames -> project.json (SaveState).
2. Sync: MainCalibWindow wires CameraPane.WhenChange to refresh CalibrationPane.
3. Calibration: CalibrationPane allows corner detection, manual alignment, and OpenCV solve.
4. Solve: SolveIntrinsics/SolveStereo update ProjectState and report.txt.
5. Review: Rectified overlay uses results of Stereo Solve to verify alignment.
*/

MainCalibWindow::MainCalibWindow()
{
	Title("Stereo Calibration Tool (Unified)");
	Sizeable().Zoomable();
}

void MainCalibWindow::Init(AppModel& m)
{
	model = &m;
	camera_pane.Init(m);
	calib_pane.Init(m);
	auto RefreshAll = [=] {
		camera_pane.RefreshFromModel();
		calib_pane.RefreshFromModel();
	};
	camera_pane.WhenChange = RefreshAll;
	calib_pane.WhenChange = RefreshAll;
	BuildLayout();
}

void MainCalibWindow::RefreshFromModel()
{
	camera_pane.RefreshFromModel();
	calib_pane.RefreshFromModel();
}

void MainCalibWindow::BuildLayout()
{
	AddFrame(menu);
	AddFrame(status);
	menu.Set(THISBACK(MainMenu));
	
	tabs.Add(camera_pane.SizePos(), "Camera");
	tabs.Add(calib_pane.SizePos(), "Calibration");
	
	Add(tabs.SizePos());
}

void MainCalibWindow::MainMenu(Bar& bar)
{
	bar.Add("File", THISBACK(SubMenuFile));
	bar.Add("Help", THISBACK(SubMenuHelp));
}

void MainCalibWindow::SubMenuFile(Bar& bar)
{
	bar.Add("Exit", THISBACK(Close));
}

void MainCalibWindow::SubMenuHelp(Bar& bar)
{
	bar.Add("About", []() { PromptOK("Stereo Calibration Tool v2.0"); });
}

END_UPP_NAMESPACE
