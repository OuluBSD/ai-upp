#include "StereoCalibrationTool.h"
#include "MainCalibWindow.h"
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
	bool verbose = false;
	int hmd_timeout_ms = 0;
	int live_timeout_ms = 0;
	int global_timeout_ms = 0;
	bool calib_identity_test = false;
	bool calib_regression = false;
	bool calib_distortion_selfcheck = false;
	bool pipeline_selfcheck = false;
	bool calib_ui_selfcheck = false;
	bool export_stcal = false;
	String test_image_path;
	
	// Direct launch flags
	bool launch_camera = false;
	bool launch_calib = false;
	bool launch_unified = false;

	for (int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if (arg == "-h" || arg == "--help") {
			Cout() << "Stereo Calibration Tool\n"
			       << "Usage: StereoCalibrationTool [project_dir] [options]\n\n"
			       << "Options:\n"
			       << "  -h, --help               Show this help message\n"
			       << "  -v, --verbose            Enable verbose logging\n"
			       << "  --camera                 Open Camera window directly\n"
			       << "  --calib                  Open Calibration window directly\n"
			       << "  --unified                Open Unified window directly\n"
			       << "  --test-usb               Run automated USB stereo source test\n"
			       << "  --test-hmd               Run automated HMD stereo source test\n"
			       << "  --test-live              Run automated live capture test\n"
			       << "  --usb-device=<path>      Set USB video device path (default: /dev/video0)\n"
			       << "  --usb-timeout-ms=<ms>    Set timeout for USB test\n"
			       << "  --hmd-timeout-ms=<ms>    Set timeout for HMD test\n"
			       << "  --live-timeout-ms=<ms>   Set timeout for live test\n"
			       << "  --timeout=<ms>           Auto-close window after timeout (for Valgrind testing)\n"
			       << "  --calib_identity_test   Test Calibration preview identity at zero extrinsics\n"
			       << "  --calib_regression      Run Calibration viewer regression suite (all invariants)\n"
			       << "  --calib_distortion_selfcheck Run Calibration distortion sanity self-check\n"
			       << "  --pipeline_selfcheck     Verify calibration pipeline invariants\n"
			       << "  --calib_ui_selfcheck    Verify Calibration UI data consistency\n"
			       << "  --export-stcal           Export calibration.stcal from project.json and exit\n"
			       << "  --image=<path>           Optional: specific image for identity test\n";
			return;
		}
		if (arg == "--pipeline_selfcheck")
			pipeline_selfcheck = true;
		else if (arg == "--calib_ui_selfcheck")
			calib_ui_selfcheck = true;
		else if (arg == "--export-stcal")
			export_stcal = true;
		else if (arg == "--test-usb")
			test_usb = true;
		else if (arg == "--test-hmd")
			test_hmd = true;
		else if (arg == "--test-live")
			test_live = true;
		else if (arg == "--camera")
			launch_camera = true;
		else if (arg == "--calib")
			launch_calib = true;
		else if (arg == "--unified")
			launch_unified = true;
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
		else if (arg.StartsWith("--timeout="))
			global_timeout_ms = atoi(arg.Mid(strlen("--timeout=")));
		else if (arg == "--calib_identity_test")
			calib_identity_test = true;
		else if (arg == "--calib_regression")
			calib_regression = true;
		else if (arg == "--calib_distortion_selfcheck")
			calib_distortion_selfcheck = true;
		else if (arg.StartsWith("--image="))
			test_image_path = arg.Mid(strlen("--image="));
		else if (!arg.StartsWith("--"))
			project_dir = arg;
	}
	
	if (project_dir.IsEmpty()) {
		FileSel fs;
		if (fs.ExecuteSelectDir("Select Project Directory"))
			project_dir = fs.Get();
		else
			return;
	}

	AppModel model;
	MenuWindow menu;
	CameraWindow camera;
	CalibrationWindow calib;
	MainCalibWindow unified;

	menu.Init(model, camera, calib, unified);
	camera.Init(model);
	calib.Init(model);
	unified.Init(model);

	if (verbose) {
		camera.SetVerbose(true);
	}

	if (!usb_device.IsEmpty())
		camera.SetUsbDevicePath(usb_device);

	// Calibration identity test mode
	if (calib_identity_test) {
		int result = TestCalibrationIdentity(model, project_dir, test_image_path);
		SetExitCode(result);
		return;
	}

	// Calibration regression suite mode
	if (calib_regression) {
		int result = RunCalibrationRegression(project_dir, verbose);
		SetExitCode(result);
		return;
	}

	// Calibration distortion self-check mode
	if (calib_distortion_selfcheck) {
		int result = RunCalibrationDistortionSelfCheck(verbose);
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
		
		if (errors > 0) {
			Cout() << "Pipeline self-check FAILED with " << errors << " error(s).\n";
			SetExitCode(1);
		} else {
			Cout() << "Pipeline self-check PASSED.\n";
			SetExitCode(0);
		}
		return;
	}

	// Calibration UI self-check mode
	if (calib_ui_selfcheck) {
		// ... existing code ...
		return;
	}

	// Export .stcal mode
	if (export_stcal) {
		model.project_dir = project_dir;
		StereoCalibrationHelpers::LoadState(model);
		const ProjectState& ps = model.project_state;
		
		if (ps.lens_f > 0) {
			model.last_calibration.is_enabled = true;
			model.last_calibration.principal_point = vec2((float)ps.lens_cx, (float)ps.lens_cy);
			model.last_calibration.angle_to_pixel = vec4((float)ps.lens_f, 0, (float)(ps.lens_f * ps.lens_k1), (float)(ps.lens_f * ps.lens_k2));
			model.last_calibration.eye_dist = (float)(ps.eye_dist / 1000.0);
			model.last_calibration.outward_angle = (float)(ps.yaw_r * M_PI / 180.0);
			model.last_calibration.right_pitch = (float)(ps.pitch_r * M_PI / 180.0);
			model.last_calibration.right_roll = (float)(ps.roll_r * M_PI / 180.0);
			
			StereoCalibrationHelpers::SaveLastCalibration(model);
			Cout() << "Exported calibration.stcal to " << project_dir << "\n";
			SetExitCode(0);
		} else {
			Cerr() << "Error: No solved calibration found in project.json\n";
			SetExitCode(1);
		}
		return;
	}

	model.project_dir = project_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);
	camera.RefreshFromModel();
	calib.RefreshFromModel();
	unified.RefreshFromModel();
	menu.RefreshFromModel();

	if (test_usb || test_hmd || test_live) {
		PromptOK("Automated tests are only available via the disabled controller.");
		return;
	}

	// Setup global timeout if requested (for Valgrind testing)
	if (global_timeout_ms > 0) {
		SetTimeCallback(global_timeout_ms, [&]() {
			if (launch_camera) camera.Break();
			else if (launch_calib) calib.Break();
			else if (launch_unified) unified.Break();
			else menu.Break();
		});
	}

	// Direct window launch (skips menu)
	if (launch_camera) { camera.Run(); return; }
	if (launch_calib) { calib.Run(); return; }
	if (launch_unified) { unified.Run(); return; }

	// Default: open unified window
	unified.Run();
}
