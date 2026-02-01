#include "StereoCalibrationTool.h"
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Upp;

GUI_APP_MAIN
{
	SetLanguage(LNG_ENGLISH);
	const Vector<String>& args = CommandLine();
	String usb_device;
	String project_dir;
	int usb_timeout_ms = 0;
	bool test_usb = false;
	bool test_hmd = false;
	bool test_live = false;
	bool solve_mode = false;
	bool verbose = false;
	int hmd_timeout_ms = 0;
	int live_timeout_ms = 0;
	bool use_ga = false;
	int ga_population = 30;
	int ga_generations = 20;
	bool stagea_identity_test = false;
	bool stagea_regression = false;
	bool stagea_distortion_selfcheck = false;
	bool pipeline_selfcheck = false;
	bool stagea_ui_selfcheck = false;
	String test_image_path;
	String ga_run_phase;
	bool ga_run_mode = false;
	
	// Direct launch flags
	bool launch_camera = false;
	bool launch_stagea = false;
	bool launch_stageb = false;
	bool launch_stagec = false;
	bool launch_live = false;

	for (int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if (arg == "-h" || arg == "--help") {
			Cout() << "Stereo Calibration Tool\n"
			       << "Usage: StereoCalibrationTool [project_dir] [options]\n\n"
			       << "Options:\n"
			       << "  -h, --help               Show this help message\n"
			       << "  -v, --verbose            Enable verbose logging\n"
			       << "  --camera                 Open Camera window directly\n"
			       << "  --stagea                 Open Stage A window directly\n"
			       << "  --stageb                 Open Stage B window directly\n"
			       << "  --stagec                 Open Stage C window directly\n"
			       << "  --live                   Open Live Result window directly\n"
			       << "  --solve                  Run solver headlessly and output math log to stdout\n"
			       << "  --test-usb               Run automated USB stereo source test\n"
			       << "  --test-hmd               Run automated HMD stereo source test\n"
			       << "  --test-live              Run automated live capture test\n"
			       << "  --usb-device=<path>      Set USB video device path (default: /dev/video0)\n"
			       << "  --usb-timeout-ms=<ms>    Set timeout for USB test\n"
			       << "  --hmd-timeout-ms=<ms>    Set timeout for HMD test\n"
			       << "  --live-timeout-ms=<ms>   Set timeout for live test\n"
			       << "  --ga                     Enable genetic algorithm bootstrap for extrinsics\n"
			       << "  --ga_run <project_dir>   Run GA headlessly\n"
			       << "  --phase <ext|int|both_lens_then_pose> Phase for --ga_run\n"
			       << "  --ga-population=<n>      Set GA population size (default: 30)\n"
			       << "  --ga-generations=<n>     Set GA generations (default: 20)\n"
			       << "  --stagea_identity_test   Test Stage A preview identity at zero extrinsics\n"
			       << "  --stagea_regression      Run Stage A viewer regression suite (all invariants)\n"
			       << "  --stagea_distortion_selfcheck Run Stage A distortion sanity self-check\n"
			       << "  --pipeline_selfcheck     Verify calibration pipeline invariants\n"
			       << "  --stagea_ui_selfcheck    Verify Stage A UI data consistency\n"
			       << "  --image=<path>           Optional: specific image for identity test\n";
			return;
		}
		if (arg == "--ga_run")
			ga_run_mode = true;
		else if (arg == "--phase" && i + 1 < args.GetCount())
			ga_run_phase = args[++i];
		else if (arg.StartsWith("--phase="))
			ga_run_phase = arg.Mid(strlen("--phase="));
		else if (arg == "--ga-population" && i + 1 < args.GetCount())
			ga_population = atoi(args[++i]);
		else if (arg.StartsWith("--ga-population="))
			ga_population = atoi(arg.Mid(strlen("--ga-population=")));
		else if (arg == "--ga-generations" && i + 1 < args.GetCount())
			ga_generations = atoi(args[++i]);
		else if (arg.StartsWith("--ga-generations="))
			ga_generations = atoi(arg.Mid(strlen("--ga-generations=")));
		else if (arg == "--pipeline_selfcheck")
			pipeline_selfcheck = true;
		else if (arg == "--stagea_ui_selfcheck")
			stagea_ui_selfcheck = true;
		else if (arg == "--test-usb")
			test_usb = true;
		else if (arg == "--test-hmd")
			test_hmd = true;
		else if (arg == "--test-live")
			test_live = true;
		else if (arg == "--solve")
			solve_mode = true;
		else if (arg == "--camera")
			launch_camera = true;
		else if (arg == "--stagea")
			launch_stagea = true;
		else if (arg == "--stageb")
			launch_stageb = true;
		else if (arg == "--stagec")
			launch_stagec = true;
		else if (arg == "--live")
			launch_live = true;
		else if (arg == "-v" || arg == "--verbose")
			verbose = true;
		else if (arg.StartsWith("--usb-device="))
			usb_device = arg.Mid(strlen("--usb-device="));
		else if (arg.StartsWith("--usb-timeout-ms="))
			usb_timeout_ms = atoi(arg.Mid(strlen("--usb-timeout-ms=")));
		else if (arg.StartsWith("--hmd-timeout-ms="))
			hmd_timeout_ms = atoi(arg.Mid(strlen("--hmd-timeout-ms=")));
		else if (arg.StartsWith("--live-timeout-ms="))
			live_timeout_ms = atoi(arg.Mid(strlen("--live-timeout-ms=")));
		else if (arg == "--ga")
			use_ga = true;
		else if (arg == "--stagea_identity_test")
			stagea_identity_test = true;
		else if (arg == "--stagea_regression")
			stagea_regression = true;
		else if (arg == "--stagea_distortion_selfcheck")
			stagea_distortion_selfcheck = true;
		else if (arg.StartsWith("--image="))
			test_image_path = arg.Mid(strlen("--image="));
		else if (!arg.StartsWith("--"))
			project_dir = arg;
	}
	
	if (project_dir.IsEmpty()) {
		if (solve_mode) {
			Cerr() << "Error: --solve requires project directory argument\n";
			SetExitCode(1);
			return;
		}
		FileSel fs;
		if (fs.ExecuteSelectDir("Select Project Directory"))
			project_dir = fs.Get();
		else
			return;
	}

	AppModel model;
	MenuWindow menu;
	CameraWindow camera;
	StageAWindow stage_a;
	StageBWindow stage_b;
	StageCWindow stage_c;
	LiveResultWindow live;

	menu.Init(model, camera, stage_a, stage_b, stage_c, live);
	camera.Init(model);
	stage_a.Init(model);
	stage_b.Init(model);
	stage_c.Init(model);
	live.Init(model);

	if (verbose) {
		camera.SetVerbose(true);
		live.SetVerbose(true);
	}

	if (!usb_device.IsEmpty())
		camera.SetUsbDevicePath(usb_device);

	if (use_ga) {
		model.use_ga_bootstrap = true;
		model.ga_population = ga_population;
		model.ga_generations = ga_generations;
	}

	// Stage A identity test mode
	if (stagea_identity_test) {
		int result = TestStageAIdentity(model, project_dir, test_image_path);
		SetExitCode(result);
		return;
	}

	// Stage A regression suite mode
	if (stagea_regression) {
		int result = RunStageARegression(project_dir, verbose);
		SetExitCode(result);
		return;
	}

	// Stage A distortion self-check mode
	if (stagea_distortion_selfcheck) {
		int result = RunStageADistortionSelfCheck(verbose);
		SetExitCode(result);
		return;
	}

	// Pipeline self-check mode
	if (pipeline_selfcheck) {
		model.project_dir = project_dir;
		StereoCalibrationHelpers::LoadState(model);
		const ProjectState& ps = model.project_state;
		Cout() << "Pipeline Self-Check: " << project_dir << "\n";
		Cout() << "Current State: " << StereoCalibrationHelpers::GetCalibrationStateText(ps.calibration_state) << "\n";
		
		int errors = 0;
		if (ps.calibration_state >= CALIB_STAGE_B_SOLVED) {
			if (ps.stage_b_diag.final_reproj_rms > 5.0) {
				Cerr() << "ERROR: Stage B Reproj RMS too high: " << ps.stage_b_diag.final_reproj_rms << " px\n";
				errors++;
			}
		}
		if (ps.calibration_state >= CALIB_STAGE_C_REFINED) {
			if (ps.stage_c_diag.cost_after >= ps.stage_c_diag.cost_before) {
				Cerr() << "ERROR: Stage C cost did not improve.\n";
				errors++;
			}
		}
		
		if (errors > 0) {
			Cout() << "Pipeline self-check FAILED with " << errors << " error(s).\n";
			SetExitCode(1);
		} else {
			Cout() << "Pipeline self-check PASSED.\n";
			SetExitCode(0);
		}
		return;
	}

	// Stage A UI self-check mode
	if (stagea_ui_selfcheck) {
		model.project_dir = project_dir;
		StereoCalibrationHelpers::LoadState(model);
		Cout() << "Stage A UI Self-Check: " << project_dir << "\n";
		Cout() << "Frames: " << model.captured_frames.GetCount() << "\n";
		int total_matches = 0, total_lines = 0;
		for(const auto& f : model.captured_frames) {
			total_matches += f.matches.GetCount();
			total_lines += f.annotation_lines_left.GetCount() + f.annotation_lines_right.GetCount();
			for(const auto& c : f.annotation_lines_left) if(c.GetCount() < 2) { Cerr() << "ERROR: Short line found\n"; SetExitCode(1); return; }
			for(const auto& c : f.annotation_lines_right) if(c.GetCount() < 2) { Cerr() << "ERROR: Short line found\n"; SetExitCode(1); return; }
		}
		Cout() << "Matches: " << total_matches << ", Lines: " << total_lines << "\n";
		Cout() << "Stage A UI self-check PASSED.\n";
		SetExitCode(0);
		return;
	}

	// Headless solve mode
	if (solve_mode) {
		model.project_dir = project_dir;
		StereoCalibrationHelpers::LoadLastCalibration(model);
		StereoCalibrationHelpers::LoadState(model);
		stage_b.RefreshFromModel();
		int result = stage_b.SolveCalibration() ? 0 : 1;
		StereoCalibrationHelpers::SaveLastCalibration(model);
		StereoCalibrationHelpers::SaveState(model);
		SetExitCode(result);
		return;
	}

	// Headless GA run mode
	if (ga_run_mode) {
		model.project_dir = project_dir;
		StereoCalibrationHelpers::LoadLastCalibration(model);
		StereoCalibrationHelpers::LoadState(model);
		
		GAPhase phase = GA_PHASE_BOTH;
		if (ga_run_phase == "ext" || ga_run_phase == "extrinsics") phase = GA_PHASE_EXTRINSICS;
		else if (ga_run_phase == "int" || ga_run_phase == "intrinsics") phase = GA_PHASE_INTRINSICS;
		else if (ga_run_phase == "both" || ga_run_phase == "both_lens_then_pose") phase = GA_PHASE_BOTH;
		
		StereoCalibrationSolver solver;
		solver.eye_dist = model.project_state.eye_dist / 1000.0;
		solver.ga_population = ga_population;
		solver.ga_generations = ga_generations;
		solver.ga_use_trimmed_loss = model.project_state.ga_use_trimmed_loss;
		solver.ga_trim_percent = model.project_state.ga_trim_percent;
		
		solver.ga_bounds.yaw_deg = model.project_state.ga_bounds.yaw_deg;
		solver.ga_bounds.pitch_deg = model.project_state.ga_bounds.pitch_deg;
		solver.ga_bounds.roll_deg = model.project_state.ga_bounds.roll_deg;
		solver.ga_bounds_intr.fov_min = model.project_state.ga_bounds.fov_min;
		solver.ga_bounds_intr.fov_max = model.project_state.ga_bounds.fov_max;
		solver.ga_bounds_intr.cx_delta = model.project_state.ga_bounds.cx_delta;
		solver.ga_bounds_intr.cy_delta = model.project_state.ga_bounds.cy_delta;
		solver.ga_bounds_intr.k1_min = model.project_state.ga_bounds.k1_min;
		solver.ga_bounds_intr.k1_max = model.project_state.ga_bounds.k1_max;
		solver.ga_bounds_intr.k2_min = model.project_state.ga_bounds.k2_min;
		solver.ga_bounds_intr.k2_max = model.project_state.ga_bounds.k2_max;
		
		for (const auto& f : model.captured_frames) {
			Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : f.right_img.GetSize();
			if (sz.cx <= 0) sz = Size(1280, 720); // Fallback for headless if images not loaded
			for (const auto& m : f.matches) {
				auto& sm = solver.matches.Add();
				sm.left_px = vec2(m.left.x * sz.cx, m.left.y * sz.cy);
				sm.right_px = vec2(m.right.x * sz.cx, m.right.y * sz.cy);
				sm.image_size = sz;
				sm.dist_l = m.dist_l / 1000.0;
				sm.dist_r = m.dist_r / 1000.0;
			}
			for (const auto& line : f.annotation_lines_left) {
				auto& sl = solver.lines.Add();
				sl.eye = 0;
				for (const auto& p : line) sl.raw_norm.Add(vec2(p.x, p.y));
			}
			for (const auto& line : f.annotation_lines_right) {
				auto& sl = solver.lines.Add();
				sl.eye = 1;
				for (const auto& p : line) sl.raw_norm.Add(vec2(p.x, p.y));
			}
		}
		
		if (solver.matches.GetCount() < 5 && solver.lines.IsEmpty()) {
			Cerr() << "Error: Too few matches/lines for GA run\n";
			SetExitCode(1);
			return;
		}
		
		StereoCalibrationParams params;
		// Initialize from project state
		double w = solver.matches.GetCount() > 0 ? solver.matches[0].image_size.cx : 1280;
		double fov_rad = model.project_state.fov_deg * M_PI / 180.0;
		params.a = (w * 0.5) / tan(fov_rad * 0.5);
		params.cx = model.project_state.lens_cx > 0 ? model.project_state.lens_cx : w*0.5;
		params.cy = model.project_state.lens_cy > 0 ? model.project_state.lens_cy : (solver.matches.GetCount() > 0 ? solver.matches[0].image_size.cy*0.5 : 720*0.5);
		params.c = params.a * model.project_state.lens_k1;
		params.d = params.a * model.project_state.lens_k2;
		params.yaw_l = model.project_state.yaw_l * M_PI / 180.0;
		params.pitch_l = model.project_state.pitch_l * M_PI / 180.0;
		params.roll_l = model.project_state.roll_l * M_PI / 180.0;
		params.yaw = model.project_state.yaw_r * M_PI / 180.0;
		params.pitch = model.project_state.pitch_r * M_PI / 180.0;
		params.roll = model.project_state.roll_r * M_PI / 180.0;

		Cout() << "Running GA Phase: " << ga_run_phase << " (pop=" << ga_population << ", gens=" << ga_generations << ")...\n";
		solver.GABootstrapPipeline(params, phase);
		
		Cout() << "GA Run Finished.\n";
		Cout() << "Best Results:\n";
		Cout() << Format("  Yaw L/R: %.3f / %.3f\n", params.yaw_l * 180/M_PI, params.yaw * 180/M_PI);
		Cout() << Format("  Focal: %.2f, k1=%.4f, k2=%.4f\n", params.a, params.c/params.a, params.d/params.a);
		
		SetExitCode(0);
		return;
	}

	model.project_dir = project_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);
	camera.RefreshFromModel();
	stage_a.RefreshFromModel();
	stage_b.RefreshFromModel();
	stage_c.RefreshFromModel();
	menu.RefreshFromModel();

	if (test_usb || test_hmd || test_live) {
		PromptOK("Automated tests are only available via the disabled controller.");
		return;
	}

	// Direct window launch (skips menu)
	if (launch_camera) { camera.Run(); return; }
	if (launch_stagea) { stage_a.Run(); return; }
	if (launch_stageb) { stage_b.Run(); return; }
	if (launch_stagec) { stage_c.Run(); return; }
	if (launch_live) { live.Run(); return; }

	// Default: open menu
	menu.Run();
}