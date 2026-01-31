#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
StageB.cpp
==========
Purpose:
- Stage B solver UI for intrinsics/extrinsics solve based on match pairs.
 - Does NOT capture frames or manage the camera pipeline.

Data flow:
- Reads AppModel.captured_frames and AppModel.project_state.
- Writes AppModel.last_calibration + report/math text.
*/

StageBWindow::StageBWindow() {
	Title("Stereo Calibration Tool - Stage B");
	Sizeable().Zoomable();
	AddFrame(status);
	BuildLayout();
}

// Binds AppModel and syncs UI state from it.
void StageBWindow::Init(AppModel& m) {
	model = &m;
	RefreshFromModel();
}

// Refreshes UI toggles and report buffers from AppModel.
void StageBWindow::RefreshFromModel() {
	if (!model)
		return;
	verbose_math_log <<= model->project_state.verbose_math_log;
	stage_b_compare_basic <<= model->project_state.compare_basic_params;
	report_text <<= model->report_text;
	math_text <<= model->math_text;
}

// Builds solver UI layout and report panes.
void StageBWindow::BuildLayout() {
	solve_calibration.SetLabel("Solve Intrinsics");
	solve_calibration <<= THISBACK(OnSolve);

    verbose_math_log.SetLabel("Verbose math log");
    verbose_math_log.WhenAction = THISBACK(OnReviewChanged);
    stage_b_compare_basic.SetLabel("Compare Basic Params");
    stage_b_compare_basic.WhenAction = THISBACK(OnReviewChanged);

	load_calibration.SetLabel("Load Calibration");
	load_calibration <<= THISBACK(LoadCalibration);
	export_calibration.SetLabel("Export Calibration");
	export_calibration <<= THISBACK(ExportCalibration);
	deploy_calibration.SetLabel("Deploy Calibration");
	deploy_calibration <<= THISBACK(DeployCalibration);

	report_text.SetReadOnly();
	math_text.SetReadOnly();
	math_text.SetFont(Courier(12));

	int y = 6;
	Add(solve_calibration.TopPos(y, 24).HSizePos(6, 6));
	y += 30;
	Add(verbose_math_log.TopPos(y, 20).HSizePos(6, 6));
	y += 24;
	Add(stage_b_compare_basic.TopPos(y, 20).HSizePos(6, 6));
	y += 30;
	Add(load_calibration.TopPos(y, 24).LeftPos(6, 130));
	Add(export_calibration.TopPos(y, 24).LeftPos(142, 130));
	y += 30;
	Add(deploy_calibration.TopPos(y, 24).LeftPos(6, 266));
	y += 32;
	Add(report_text.VSizePos(y, 160).HSizePos(6, 6));
	Add(math_text.BottomPos(6, 150).HSizePos(6, 6));
}

// UI handler: runs the solver and updates status.
void StageBWindow::OnSolve() {
	if (SolveCalibration())
		status.Set("Intrinsics solved.");
}

// Core Stage B solver: builds match list, solves, writes AppModel results.
bool StageBWindow::SolveCalibration() {
	if (!model)
		return false;
	model->project_state.verbose_math_log = (bool)verbose_math_log;
	model->project_state.compare_basic_params = (bool)stage_b_compare_basic;
	String math_log;
	math_log << "Stereo Calibration Math Report (Stage B)\n";
	math_log << "========================================\n\n";

	StereoCalibrationSolver solver;
	solver.log = &math_log;
	solver.eye_dist = model->project_state.eye_dist / 1000.0; // mm -> m
	solver.EnableTrace((bool)verbose_math_log, 2, 20000);

	for (const auto& f : model->captured_frames) {
		Size sz = f.left_img.GetSize();
		if (sz.cx <= 0)
			continue;
		for (const auto& m : f.matches) {
			if (IsNull(m.left) || IsNull(m.right))
				continue;
			auto& p = solver.matches.Add();
			p.left_px = vec2((float)(m.left.x * sz.cx), (float)(m.left.y * sz.cy));
			p.right_px = vec2((float)(m.right.x * sz.cx), (float)(m.right.y * sz.cy));
			p.image_size = sz;
			p.dist_l = m.dist_l / 1000.0;
			p.dist_r = m.dist_r / 1000.0;
		}
	}

	if (solver.matches.GetCount() < 5) {
		PromptOK("At least 5 match pairs across all frames are required.");
		return false;
	}

	StereoCalibrationParams params;
	params.yaw = model->project_state.yaw_r - model->project_state.yaw_l;
	params.pitch = model->project_state.pitch_r - model->project_state.pitch_l;
	params.roll = model->project_state.roll_r - model->project_state.roll_l;

	Size init_sz = solver.matches[0].image_size;
	double fov_rad = model->project_state.fov_deg * M_PI / 180.0;
	params.a = (init_sz.cx * 0.5) / (fov_rad * 0.5);
	double s = model->project_state.barrel_strength * 0.01;
	params.b = params.a * s;
	params.c = params.a * s;
	params.d = params.a * s;
	params.cx = init_sz.cx * 0.5;
	params.cy = init_sz.cy * 0.5;

	status.Set("Solving intrinsics (extrinsics locked from Stage A)...");
	if (solver.SolveIntrinsicsOnly(params)) {
		model->last_calibration.is_enabled = true;
		model->last_calibration.eye_dist = (float)solver.eye_dist;
		model->last_calibration.outward_angle = (float)params.yaw;
		model->last_calibration.right_pitch = (float)params.pitch;
		model->last_calibration.right_roll = (float)params.roll;
		model->last_calibration.principal_point = vec2((float)params.cx, (float)params.cy);
		model->last_calibration.angle_to_pixel = vec4((float)params.a, (float)params.b, (float)params.c, (float)params.d);

		StereoCalibrationDiagnostics diag;
		solver.ComputeDiagnostics(params, diag);
		String report;
		report << "Stage B Solve Success.\n";
		report << Format("Reproj RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		model->report_text = report;
		model->math_text = math_log + solver.GetTraceText();
		report_text <<= model->report_text;
		math_text <<= model->math_text;
		StereoCalibrationHelpers::SaveLastCalibration(*model);
		SaveProjectState();
		return true;
	}

	PromptOK("Solver failed: " + solver.last_failure_reason);
	return false;
}

// Loads a calibration file into AppModel.last_calibration.
void StageBWindow::LoadCalibration() {
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteOpen("Load Stereo Calibration"))
		return;
	StereoCalibrationData data;
	if (!HMD::StereoTracker::LoadCalibrationFile(fs, data)) {
		PromptOK("Failed to load calibration.");
		return;
	}
	model->last_calibration = data;
	status.Set("Calibration loaded.");
	StereoCalibrationHelpers::SaveLastCalibration(*model);
}

// Exports the current calibration to a .stcal file.
void StageBWindow::ExportCalibration() {
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteSaveAs("Export Stereo Calibration"))
		return;
	StereoCalibrationData data = model->last_calibration;
	if (!HMD::StereoTracker::SaveCalibrationFile(fs, data)) {
		PromptOK("Failed to export calibration.");
		return;
	}
	PromptOK("Calibration exported.");
}

// Deploys calibration to the default share/calibration path.
void StageBWindow::DeployCalibration() {
	if (!PromptYesNo("Deploy current calibration to share/calibration/hp_vr1000/calibration.stcal?"))
		return;
	String path = "share/calibration/hp_vr1000/calibration.stcal";
	RealizeDirectory(GetFileFolder(path));
	StereoCalibrationData data = model->last_calibration;
	if (!HMD::StereoTracker::SaveCalibrationFile(path, data)) {
		PromptOK("Failed to deploy calibration.");
		return;
	}
	PromptOK("Calibration deployed.");
}

// Updates AppModel.project_state from solver UI toggles.
void StageBWindow::OnReviewChanged() {
	if (!model)
		return;
	model->project_state.verbose_math_log = (bool)verbose_math_log;
	model->project_state.compare_basic_params = (bool)stage_b_compare_basic;
	SaveProjectState();
}

// Placeholder for future calibration edits in Stage B.
void StageBWindow::SyncCalibrationFromEdits() {
	// No editable calibration fields in Stage B yet. Placeholder for future UI.
}

// Placeholder for future calibration edits in Stage B.
void StageBWindow::SyncEditsFromCalibration() {
	// No editable calibration fields in Stage B yet. Placeholder for future UI.
}

// Persists AppModel to disk.
void StageBWindow::SaveProjectState() {
	if (!model || model->project_dir.IsEmpty())
		return;
	StereoCalibrationHelpers::SaveState(*model);
}

END_UPP_NAMESPACE
