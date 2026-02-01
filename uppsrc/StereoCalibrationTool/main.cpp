#include "StereoCalibrationTool.h"
#include <cstring>

using namespace Upp;

GUI_APP_MAIN
{
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
	String test_image_path;
	String ga_run_phase;
	bool ga_run_mode = false;
	
	// Direct launch flags
	bool launch_camera = false;
	bool launch_stagea = false;
	bool launch_stageb = false;
	bool launch_stagec = false;
	bool launch_live = false;

	for (const String& arg : args) {
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
			       << "  --phase <ext|int|both>   Phase for --ga_run\n"
			       << "  --ga-population=<n>      Set GA population size (default: 30)\n"
			       << "  --ga-generations=<n>     Set GA generations (default: 20)\n"
			       << "  --stagea_identity_test   Test Stage A preview identity at zero extrinsics\n"
			       << "  --stagea_regression      Run Stage A viewer regression suite (all invariants)\n"
			       << "  --stagea_distortion_selfcheck Run Stage A distortion sanity self-check\n"
			       << "  --image=<path>           Optional: specific image for identity test\n";
			return;
		}
		if (arg == "--ga_run")
			ga_run_mode = true;
		else if (arg.StartsWith("--phase="))
			ga_run_phase = arg.Mid(strlen("--phase="));
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
			verbose = true; // Applied after windows are created.
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
		else if (arg.StartsWith("--ga-population="))
			ga_population = atoi(arg.Mid(strlen("--ga-population=")));
		else if (arg.StartsWith("--ga-generations="))
			ga_generations = atoi(arg.Mid(strlen("--ga-generations=")));
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
		if (ga_run_phase == "ext") phase = GA_PHASE_EXTRINSICS;
		else if (ga_run_phase == "int") phase = GA_PHASE_INTRINSICS;
		
		StereoCalibrationSolver solver;
		solver.eye_dist = model.project_state.eye_dist / 1000.0;
		solver.ga_population = ga_population;
		solver.ga_generations = ga_generations;
		solver.ga_use_trimmed_loss = model.project_state.ga_use_trimmed_loss;
		solver.ga_trim_percent = model.project_state.ga_trim_percent;
		
		for (const auto& f : model.captured_frames) {
			Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : f.right_img.GetSize();
			if (sz.cx <= 0) continue;
			for (const auto& m : f.matches) {
				auto& sm = solver.matches.Add();
				sm.left_px = vec2(m.left.x * sz.cx, m.left.y * sz.cy);
				sm.right_px = vec2(m.right.x * sz.cx, m.right.y * sz.cy);
				sm.image_size = sz;
				sm.dist_l = m.dist_l / 1000.0;
				sm.dist_r = m.dist_r / 1000.0;
			}
		}
		
		if (solver.matches.GetCount() < 5) {
			Cerr() << "Error: Too few matches for GA run\n";
			SetExitCode(1);
			return;
		}
		
		StereoCalibrationParams params;
		// Initialize from project state
		double w = solver.matches[0].image_size.cx;
		double fov_rad = model.project_state.fov_deg * M_PI / 180.0;
		params.a = (w * 0.5) / tan(fov_rad * 0.5);
		params.cx = model.project_state.lens_cx > 0 ? model.project_state.lens_cx : w*0.5;
		params.cy = model.project_state.lens_cy > 0 ? model.project_state.lens_cy : solver.matches[0].image_size.cy*0.5;
		params.c = params.a * model.project_state.lens_k1;
		params.d = params.a * model.project_state.lens_k2;
		params.yaw_l = model.project_state.yaw_l * M_PI / 180.0;
		params.pitch_l = model.project_state.pitch_l * M_PI / 180.0;
		params.roll_l = model.project_state.roll_l * M_PI / 180.0;
		params.yaw = model.project_state.yaw_r * M_PI / 180.0;
		params.pitch = model.project_state.pitch_r * M_PI / 180.0;
		params.roll = model.project_state.roll_r * M_PI / 180.0;

		Cout() << "Running GA Phase: " << ga_run_phase << "...\n";
		solver.GABootstrapPipeline(params, phase);
		
		Cout() << "GA Run Finished.\n";
		Cout() << "Best Results:\n";
		Cout() << "  Yaw L/R: " << params.yaw_l * 180/M_PI << " / " << params.yaw * 180/M_PI << "\n";
		Cout() << "  Focal: " << params.a << ", k1=" << params.c/params.a << ", k2=" << params.d/params.a << "\n";
		
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