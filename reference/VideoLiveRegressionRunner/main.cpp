#include "VideoLiveRegressionRunner.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoLiveRegressionRunner\n\n"
	       << "Usage: VideoLiveRegressionRunner [options]\n\n"
	       << "Options:\n"
	       << "  --host <host>       VideoServer host (default 127.0.0.1)\n"
	       << "  --port <port>       VideoServer port (default 8082)\n"
	       << "  --frames <count>    Frames to record (default 10)\n"
	       << "  --name <name>       Regression name (default live_smoke)\n"
	       << "  --out-root <dir>    Output root (default tmp)\n"
	       << "  --help, -h          Show help\n";
}

static LiveRegressionOptions ParseOptions(const Vector<String>& args)
{
	LiveRegressionOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--host" && i + 1 < args.GetCount())
			opt.host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount())
			opt.port = StrInt(args[++i]);
		else if(args[i] == "--frames" && i + 1 < args.GetCount())
			opt.frames = max(1, StrInt(args[++i]));
		else if(args[i] == "--name" && i + 1 < args.GetCount())
			opt.name = args[++i];
		else if(args[i] == "--out-root" && i + 1 < args.GetCount())
			opt.out_root = args[++i];
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.name.IsEmpty())
		opt.name = "live_smoke";
	if(opt.out_root.IsEmpty())
		opt.out_root = "tmp";
	return opt;
}

static int RunCommand(const String& exe, const Vector<String>& args)
{
	String printable = exe;
	for(const String& arg : args)
		printable << " " << arg;
	Cout() << "run: " << printable << "\n";
	String out;
	int code = Sys(exe, args, out);
	if(!out.IsEmpty())
		Cout() << out;
	Cout() << "exit: " << code << "\n";
	return code;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	LiveRegressionOptions opt = ParseOptions(CommandLine());
	if(opt.help) {
		PrintHelp();
		return;
	}

	String exe_dir = GetFileDirectory(GetExeFilePath());
	String recorder = AppendFileName(exe_dir, "VideoServerFrameRecorder.exe");
	String tracker = AppendFileName(exe_dir, "VideoWindowTracker.exe");
	String record_dir = AppendFileName(opt.out_root, opt.name);
	String tracked_dir = opt.name + "_tracked";
	tracked_dir = AppendFileName(opt.out_root, tracked_dir);

	RealizeDirectory(opt.out_root);
	DeleteFolderDeep(record_dir);
	DeleteFolderDeep(tracked_dir);

	Vector<String> recorder_args;
	recorder_args << "--host" << opt.host
	              << "--port" << AsString(opt.port)
	              << "--frames" << AsString(opt.frames)
	              << "--out" << record_dir;
	if(RunCommand(recorder, recorder_args) != 0) {
		Cerr() << "ERROR: recorder failed\n";
		SetExitCode(1);
		return;
	}

	Vector<String> tracker_args;
	tracker_args << "--input-dir" << record_dir
	             << "--out" << tracked_dir;
	if(RunCommand(tracker, tracker_args) != 0) {
		Cerr() << "ERROR: tracker failed\n";
		SetExitCode(1);
		return;
	}

	Cout() << "regression_name=" << opt.name << "\n";
	Cout() << "record_dir=" << record_dir << "\n";
	Cout() << "record_summary=" << AppendFileName(record_dir, "summary.json") << "\n";
	Cout() << "tracked_dir=" << tracked_dir << "\n";
	Cout() << "tracking_json=" << AppendFileName(tracked_dir, "tracking.json") << "\n";
	Cout() << "tracking_summary_json=" << AppendFileName(tracked_dir, "tracking_summary.json") << "\n";
	Cout() << "semantic_dir=" << AppendFileName(tracked_dir, "semantic") << "\n";
	Cout() << "overlays_dir=" << AppendFileName(tracked_dir, "overlays") << "\n";
}
