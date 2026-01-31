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
	BuildLayout();
}

// Binds AppModel and window references. Menu does not own these objects.
// Assumes the controller keeps all windows alive for the app lifetime.
void MenuWindow::Init(AppModel& m,
                      CameraWindow& cam,
                      StageAWindow& a,
                      StageBWindow& b,
                      StageCWindow& c,
                      LiveResultWindow& l) {
	model = &m;
	camera = &cam;
	stage_a = &a;
	stage_b = &b;
	stage_c = &c;
	live = &l;
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
	open_stage_a.SetLabel("Stage A (Basic)");
	open_stage_b.SetLabel("Stage B (Solve)");
	open_stage_c.SetLabel("Stage C (Refine)");
	open_live.SetLabel("Live Result");
	open_all.SetLabel("Open All");

	open_camera <<= THISBACK(OpenCamera);
	open_stage_a <<= THISBACK(OpenStageA);
	open_stage_b <<= THISBACK(OpenStageB);
	open_stage_c <<= THISBACK(OpenStageC);
	open_live <<= THISBACK(OpenLive);
	open_all <<= THISBACK(OpenAll);

	int y = 10;
	Add(title.TopPos(y, 28).HCenterPos(400));
	y += 34;
	Add(project_lbl.TopPos(y, 20).HSizePos(10, 10));
	y += 30;
	Add(open_camera.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_stage_a.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_stage_b.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_stage_c.TopPos(y, 28).HSizePos(10, 10));
	y += 34;
	Add(open_live.TopPos(y, 28).HSizePos(10, 10));
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

// Shows the Stage A window (does not create or destroy it).
void MenuWindow::OpenStageA() {
	if (stage_a) {
		stage_a->Open();
		stage_a->SetFocus();
	}
}

// Shows the Stage B window (does not create or destroy it).
void MenuWindow::OpenStageB() {
	if (stage_b) {
		stage_b->Open();
		stage_b->SetFocus();
	}
}

// Shows the Stage C window (does not create or destroy it).
void MenuWindow::OpenStageC() {
	if (stage_c) {
		stage_c->Open();
		stage_c->SetFocus();
	}
}

// Shows the Live Result window (does not create or destroy it).
void MenuWindow::OpenLive() {
	if (live) {
		live->Open();
		live->SetFocus();
	}
}

// Convenience helper to open all windows at once.
void MenuWindow::OpenAll() {
	OpenCamera();
	OpenStageA();
	OpenStageB();
	OpenStageC();
	OpenLive();
}

END_UPP_NAMESPACE
