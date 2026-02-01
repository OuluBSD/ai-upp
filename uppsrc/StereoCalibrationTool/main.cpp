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
	String test_image_path;
	for (const String& arg : args) {
		if (arg == "-h" || arg == "--help") {
			Cout() << "Stereo Calibration Tool\n"
			       << "Usage: StereoCalibrationTool [project_dir] [options]\n\n"
			       << "Options:\n"
			       << "  -h, --help               Show this help message\n"
			       << "  -v, --verbose            Enable verbose logging\n"
			       << "  --solve                  Run solver headlessly and output math log to stdout\n"
			       << "  --test-usb               Run automated USB stereo source test\n"
			       << "  --test-hmd               Run automated HMD stereo source test\n"
			       << "  --test-live              Run automated live capture test\n"
			       << "  --usb-device=<path>      Set USB video device path (default: /dev/video0)\n"
			       << "  --usb-timeout-ms=<ms>    Set timeout for USB test\n"
			       << "  --hmd-timeout-ms=<ms>    Set timeout for HMD test\n"
			       << "  --live-timeout-ms=<ms>   Set timeout for live test\n"
			       << "  --ga                     Enable genetic algorithm bootstrap for extrinsics\n"
			       << "  --ga-population=<n>      Set GA population size (default: 30)\n"
			       << "  --ga-generations=<n>     Set GA generations (default: 20)\n"
			       << "  --stagea_identity_test   Test Stage A preview identity at zero extrinsics\n"
			       << "  --stagea_regression      Run Stage A viewer regression suite (all invariants)\n"
			       << "  --image=<path>           Optional: specific image for identity test\n";
			return;
		}
		if (arg == "--test-usb")
			test_usb = true;
		else if (arg == "--test-hmd")
			test_hmd = true;
		else if (arg == "--test-live")
			test_live = true;
		else if (arg == "--solve")
			solve_mode = true;
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

	menu.Run();
}
