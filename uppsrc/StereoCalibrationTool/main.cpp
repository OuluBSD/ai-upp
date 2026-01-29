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
	int hmd_timeout_ms = 0;
	int live_timeout_ms = 0;
	for (const String& arg : args) {
		if (arg == "--test-usb")
			test_usb = true;
		else if (arg == "--test-hmd")
			test_hmd = true;
		else if (arg == "--test-live")
			test_live = true;
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
		else if (!arg.StartsWith("--"))
			project_dir = arg;
	}
	
	if (project_dir.IsEmpty() && args.IsEmpty()) {
		String default_dir = "share/calibration/hp_vr1000/";
		if (DirectoryExists(default_dir))
			project_dir = default_dir;
	}
	
	if (project_dir.IsEmpty()) {
		FileSel fs;
		if (fs.ExecuteSelectDir("Select Project Directory"))
			project_dir = fs.Get();
		else
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
