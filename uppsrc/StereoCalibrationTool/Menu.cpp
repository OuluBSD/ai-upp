#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
Menu.cpp
========
Purpose:
- Minimal launcher window that only opens other modules.
 - Does NOT do capture, solve, or preview work itself.

Lifetime strategy:
- The controller owns all module windows. Menu only shows/hides them (Option A).
*/

MenuWindow::MenuWindow() {
	Title("Stereo Calibration Tool - Menu");
	Sizeable().Zoomable();
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	BuildLayout();
}

void MenuWindow::MainMenu(Bar& bar) {
	bar.Add("Help", THISBACK(SubMenuHelp));
}

void MenuWindow::SubMenuHelp(Bar& bar) {
	bar.Add("Instructions", [] { StereoCalibrationHelpers::ShowInstructions(); });
}

// Binds AppModel and window references. Menu does not own these objects.
// Assumes the controller keeps all windows alive for the app lifetime.
void MenuWindow::Init(AppModel& m,
                      CameraWindow& cam,
                      CalibrationWindow& c,
                      MainCalibWindow& mw) {
	model = &m;
	camera = &cam;
	calib = &c;
	main_win = &mw;
	RefreshProjectLabel();
}

// Refreshes any labels derived from the shared AppModel state.
void MenuWindow::RefreshFromModel() {
	RefreshProjectLabel();
}

// Builds the minimal launcher layout (buttons only, no app logic).
void MenuWindow::BuildLayout() {
	title.SetLabel("Stereo Calibration Tool");
	title.SetFont(Arial(20).Bold());
	project_lbl.SetLabel("Project: (not set)");

	open_camera.SetLabel("Camera / Capture");
	open_calib.SetLabel("Calibration");
	open_unified.SetLabel("Unified (New)");
	open_all.SetLabel("Open All");

	open_camera <<= THISBACK(OpenCamera);
	open_calib <<= THISBACK(OpenCalibration);
	open_unified <<= THISBACK(OpenUnified);
	open_all <<= THISBACK(OpenAll);

	int y = 10;
	Add(title.TopPos(y, 28).HCenterPos(400));
	y += 34;
	Add(project_lbl.TopPos(y, 20).HSizePos(10, 10));
	y += 30;
	Add(open_camera.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_calib.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_unified.TopPos(y, 28).HSizePos(10, 10));
	y += 40;
	Add(open_all.TopPos(y, 28).HSizePos(10, 10));

	SetRect(0, 0, 360, y + 50);
}

// Updates the project label based on AppModel.project_dir.
// Does not attempt to validate the path (handled elsewhere).
void MenuWindow::RefreshProjectLabel() {
	if (!model)
		return;
	String label = model->project_dir.IsEmpty() ? "Project: (not set)" : "Project: " + model->project_dir;
	project_lbl.SetLabel(label);
}

// Shows the Camera window (does not create or destroy it).
void MenuWindow::OpenCamera() {
	if (camera) {
		camera->Open();
		camera->SetFocus();
	}
}

// Shows the Calibration window (does not create or destroy it).
void MenuWindow::OpenCalibration() {
	if (calib) {
		calib->Open();
		calib->SetFocus();
	}
}

void MenuWindow::OpenUnified() {
	if (main_win) {
		main_win->Open();
		main_win->SetFocus();
	}
}

// Convenience helper to open all windows at once.
void MenuWindow::OpenAll() {
	OpenCamera();
	OpenCalibration();
	OpenUnified();
}

END_UPP_NAMESPACE
