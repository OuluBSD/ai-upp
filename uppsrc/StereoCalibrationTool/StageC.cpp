#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
StageC.cpp
==========
Purpose:
- Stage C micro-refine UI for extrinsics refinement.
 - Does NOT capture frames or modify the capture list.

Data flow:
- Reads matches + Stage A parameters from AppModel.
- Writes refined calibration into AppModel.last_calibration.
*/

StageCWindow::StageCWindow() {
	Title("Stereo Calibration Tool - Stage C");
	Sizeable().Zoomable();
	AddFrame(status);
	BuildLayout();
}

// Binds AppModel and syncs Stage C controls.
void StageCWindow::Init(AppModel& m) {
	model = &m;
	RefreshFromModel();
}

// Refreshes UI control values from AppModel.project_state.
void StageCWindow::RefreshFromModel() {
	if (!model)
		return;
	const ProjectState& ps = model->project_state;
	enable_stage_c <<= ps.stage_c_enabled;
	stage_c_mode <<= ps.stage_c_mode;
	max_dyaw <<= ps.max_dyaw;
	max_dpitch <<= ps.max_dpitch;
	max_droll <<= ps.max_droll;
	lambda_edit <<= ps.lambda;
}

// Builds the Stage C UI and binds callbacks.
void StageCWindow::BuildLayout() {
	enable_stage_c.SetLabel("Enable micro-refine");
	enable_stage_c.WhenAction = THISBACK(SaveProjectState);

	stage_c_mode_lbl.SetLabel("Mode:");
	stage_c_mode.Add(0, "Relative-only");
	stage_c_mode.Add(1, "Per-eye");
	stage_c_mode <<= 0;
	stage_c_mode.WhenAction = THISBACK(SaveProjectState);

	max_dyaw_lbl.SetLabel("Max dYaw");
	max_dpitch_lbl.SetLabel("Max dPitch");
	max_droll_lbl.SetLabel("Max dRoll");
	lambda_lbl.SetLabel("Lambda");

	max_dyaw.SetInc(0.1);
	max_dpitch.SetInc(0.1);
	max_droll.SetInc(0.1);
	lambda_edit.SetInc(0.01);
	max_dyaw.WhenAction = THISBACK(SaveProjectState);
	max_dpitch.WhenAction = THISBACK(SaveProjectState);
	max_droll.WhenAction = THISBACK(SaveProjectState);
	lambda_edit.WhenAction = THISBACK(SaveProjectState);

	refine_btn.SetLabel("Run Stage C");
	refine_btn <<= THISBACK(OnRefine);

	report_text.SetReadOnly();
	math_text.SetReadOnly();
	math_text.SetFont(Courier(12));

	int y = 6;
	Add(enable_stage_c.TopPos(y, 20).HSizePos(6, 6));
	y += 24;
	Add(stage_c_mode_lbl.TopPos(y, 20).LeftPos(6, 60));
	Add(stage_c_mode.TopPos(y, 20).LeftPos(70, 160));
	y += 28;
	Add(max_dyaw_lbl.TopPos(y, 20).LeftPos(6, 80));
	Add(max_dyaw.TopPos(y, 20).LeftPos(90, 80));
	y += 24;
	Add(max_dpitch_lbl.TopPos(y, 20).LeftPos(6, 80));
	Add(max_dpitch.TopPos(y, 20).LeftPos(90, 80));
	y += 24;
	Add(max_droll_lbl.TopPos(y, 20).LeftPos(6, 80));
	Add(max_droll.TopPos(y, 20).LeftPos(90, 80));
	y += 24;
	Add(lambda_lbl.TopPos(y, 20).LeftPos(6, 80));
	Add(lambda_edit.TopPos(y, 20).LeftPos(90, 80));
	y += 28;
	Add(refine_btn.TopPos(y, 24).HSizePos(6, 6));
	y += 32;
	Add(report_text.VSizePos(y, 160).HSizePos(6, 6));
	Add(math_text.BottomPos(6, 150).HSizePos(6, 6));
}

// UI handler: runs the refinement and updates status text.
void StageCWindow::OnRefine() {
	if (RefineExtrinsics())
		status.Set("Extrinsics refined.");
}

// Core Stage C refinement: uses current matches to solve extrinsic deltas.
bool StageCWindow::RefineExtrinsics() {
	if (!model)
		return false;
	if (!(bool)enable_stage_c) {
		PromptOK("Enable Stage C first.");
		return false;
	}

	String math_log;
	math_log << "Stage C Micro-Refine\n====================\n\n";

	StereoCalibrationSolver solver;
	solver.log = &math_log;
	solver.eye_dist = model->project_state.eye_dist / 1000.0;
	solver.EnableTrace(model->project_state.verbose_math_log, 2, 20000);

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
		PromptOK("Need at least 5 matches.");
		return false;
	}

	StereoCalibrationParams params;
	params.a = model->last_calibration.angle_to_pixel[0];
	params.b = model->last_calibration.angle_to_pixel[1];
	params.c = model->last_calibration.angle_to_pixel[2];
	params.d = model->last_calibration.angle_to_pixel[3];
	params.cx = model->last_calibration.principal_point[0];
	params.cy = model->last_calibration.principal_point[1];

	double base_yaw_l = model->project_state.yaw_l;
	double base_pitch_l = model->project_state.pitch_l;
	double base_roll_l = model->project_state.roll_l;
	double base_yaw_r = model->project_state.yaw_r;
	double base_pitch_r = model->project_state.pitch_r;
	double base_roll_r = model->project_state.roll_r;

	params.yaw_l = base_yaw_l;
	params.pitch_l = base_pitch_l;
	params.roll_l = base_roll_l;
	params.yaw = base_yaw_r;
	params.pitch = base_pitch_r;
	params.roll = base_roll_r;

	vec3 bounds((double)max_dyaw, (double)max_dpitch, (double)max_droll);
	double lambda = (double)lambda_edit;
	bool per_eye = ((int)stage_c_mode == 1);

	status.Set("Refining extrinsics (Stage C)...");
	if (solver.SolveExtrinsicsOnlyMicroRefine(params, bounds, lambda, per_eye)) {
		model->dyaw_c = (float)(params.yaw - base_yaw_r);
		model->dpitch_c = (float)(params.pitch - base_pitch_r);
		model->droll_c = (float)(params.roll - base_roll_r);

		model->last_calibration.outward_angle = (float)(params.yaw - params.yaw_l);
		model->last_calibration.right_pitch = (float)(params.pitch - params.pitch_l);
		model->last_calibration.right_roll = (float)(params.roll - params.roll_l);

		StereoCalibrationDiagnostics diag;
		solver.ComputeDiagnostics(params, diag);

		String report;
		report << "Stage C Refinement Success.\n";
		report << Format("Mode: %s\n", per_eye ? "Per-eye" : "Relative-only");
		report << Format("Bounds: %.1f, %.1f, %.1f deg\n", bounds[0], bounds[1], bounds[2]);
		report << Format("Lambda: %.4f\n", lambda);
		report << Format("Reproj RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		report << Format("Dist RMS (L/R): %.3f / %.3f mm\n", diag.dist_rms_l, diag.dist_rms_r);

		report_text <<= report;
		math_text <<= math_log + solver.GetTraceText();
		model->report_text = report;
		model->math_text = math_log + solver.GetTraceText();
		StereoCalibrationHelpers::SaveLastCalibration(*model);
		SaveProjectState();
		return true;
	}

	PromptOK("Refinement failed.");
	return false;
}

// Persists Stage C settings to AppModel + disk.
void StageCWindow::SaveProjectState() {
	if (!model)
		return;
	ProjectState& ps = model->project_state;
	ps.stage_c_enabled = (bool)enable_stage_c;
	ps.stage_c_mode = (int)stage_c_mode;
	ps.max_dyaw = (double)max_dyaw;
	ps.max_dpitch = (double)max_dpitch;
	ps.max_droll = (double)max_droll;
	ps.lambda = (double)lambda_edit;
	StereoCalibrationHelpers::SaveState(*model);
}

END_UPP_NAMESPACE
