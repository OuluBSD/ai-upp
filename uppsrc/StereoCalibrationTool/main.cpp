#include "StereoCalibrationTool.h"
#include <cstring>

using namespace Upp;

GUI_APP_MAIN
{
	StereoCalibrationTool app;
	const Vector<String>& args = CommandLine();
	String usb_device;
	String project_dir;
	int usb_timeout_ms = 0;
	bool test_usb = false;
	bool test_hmd = false;
	bool test_live = false;
	bool solve_mode = false;
	int hmd_timeout_ms = 0;
	int live_timeout_ms = 0;
	bool use_ga = false;
	int ga_population = 30;
	int ga_generations = 20;
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
			       << "  --ga-generations=<n>     Set GA generations (default: 20)\n";
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
			app.SetVerbose(true);
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

	// Configure GA bootstrap if requested (before headless solve)
	if (use_ga)
		app.EnableGABootstrap(true, ga_population, ga_generations);

	// Headless solve mode
	if (solve_mode) {
		int result = app.SolveHeadless(project_dir);
		SetExitCode(result);
		return;
	}

	app.SetProjectDir(project_dir);

	if (test_usb)
		app.EnableUsbTest(usb_device, usb_timeout_ms);
	if (test_hmd)
		app.EnableHmdTest(hmd_timeout_ms);
	if (test_live)
		app.EnableLiveTest(live_timeout_ms);
	app.Run();
}
