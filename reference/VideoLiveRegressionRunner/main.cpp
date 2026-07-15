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

static String JsonString(const String& s)
{
	String out;
	for(int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if(c == '\\')
			out << "\\\\";
		else if(c == '"')
			out << "\\\"";
		else if(c == '\n')
			out << "\\n";
		else if(c == '\r')
			out << "\\r";
		else if(c == '\t')
			out << "\\t";
		else
			out.Cat(c);
	}
	return out;
}

static void AppendJsonPath(String& json, const char *key, const String& path, bool comma = true)
{
	json << "    \"" << key << "\": \"" << JsonString(path) << "\"";
	if(comma)
		json << ",";
	json << "\n";
}

static int JsonIntFromFile(const String& path, const char *key, int fallback = -1)
{
	String text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty())
		return fallback;
	Value value = ParseJSON(text);
	if(IsError(value))
		return fallback;
	ValueMap map = value;
	Value field = map.Get(key, Value());
	return IsNumber(field) ? (int)field : fallback;
}

static int JsonIntFromFileAny(const String& path, const char *key_a, const char *key_b,
                              int fallback = -1)
{
	int value = JsonIntFromFile(path, key_a, fallback);
	return value != fallback ? value : JsonIntFromFile(path, key_b, fallback);
}

static String StageStatus(int code, bool available, bool fatal)
{
	if(!available)
		return "skipped";
	if(code == 0)
		return "ok";
	return fatal ? "failed" : "failed_nonfatal";
}

static bool WritePipelineSummary(const LiveRegressionOptions& opt, const String& record_dir,
                                 const String& tracked_dir, int recorder_code, int tracker_code,
                                 int audit_code, bool audit_available, int ocr_code,
                                 bool ocr_available)
{
	String summary_path = AppendFileName(tracked_dir, "pipeline_summary.json");
	String record_summary = AppendFileName(record_dir, "summary.json");
	String tracking_summary = AppendFileName(tracked_dir, "tracking_summary.json");
	String events_json_path = AppendFileName(tracked_dir, "events.json");
	String ocr_json_path = AppendFileName(tracked_dir, "ocr_probe.json");
	String json;
	json << "{\n";
	json << "  \"runner\": \"VideoLiveRegressionRunner\",\n";
	json << "  \"status\": \"ok\",\n";
	json << "  \"regression_name\": \"" << JsonString(opt.name) << "\",\n";
	json << "  \"host\": \"" << JsonString(opt.host) << "\",\n";
	json << "  \"port\": " << opt.port << ",\n";
	json << "  \"frames_requested\": " << opt.frames << ",\n";
	json << "  \"record_dir\": \"" << JsonString(record_dir) << "\",\n";
	json << "  \"tracked_dir\": \"" << JsonString(tracked_dir) << "\",\n";
	json << "  \"record_dir_abs\": \"" << JsonString(NormalizePath(record_dir)) << "\",\n";
	json << "  \"tracked_dir_abs\": \"" << JsonString(NormalizePath(tracked_dir)) << "\",\n";
	json << "  \"stages\": {\n";
	json << "    \"recorder\": {\"status\": \"" << StageStatus(recorder_code, true, true)
	     << "\", \"exit_code\": " << recorder_code << ", \"fatal\": true},\n";
	json << "    \"tracker\": {\"status\": \"" << StageStatus(tracker_code, true, true)
	     << "\", \"exit_code\": " << tracker_code << ", \"fatal\": true},\n";
	json << "    \"audit_report\": {\"available\": " << (audit_available ? "true" : "false")
	     << ", \"status\": \"" << StageStatus(audit_code, audit_available, true)
	     << "\", \"exit_code\": " << audit_code << ", \"fatal\": true},\n";
	json << "    \"ocr_probe\": {\"available\": " << (ocr_available ? "true" : "false")
	     << ", \"status\": \"" << StageStatus(ocr_code, ocr_available, false)
	     << "\", \"exit_code\": " << ocr_code << ", \"fatal\": false}\n";
	json << "  },\n";
	json << "  \"observed\": {\n";
	json << "    \"frames_recorded\": "
	     << JsonIntFromFileAny(record_summary, "frame_count", "frames_saved") << ",\n";
	json << "    \"frames_tracked\": " << JsonIntFromFile(tracking_summary, "frame_count") << ",\n";
	json << "    \"events\": " << JsonIntFromFile(events_json_path, "event_count") << ",\n";
	json << "    \"ocr_crop_count\": " << JsonIntFromFile(ocr_json_path, "crop_count") << "\n";
	json << "  },\n";
	json << "  \"artifacts\": {\n";
	AppendJsonPath(json, "record_summary", record_summary);
	AppendJsonPath(json, "tracking_json", AppendFileName(tracked_dir, "tracking.json"));
	AppendJsonPath(json, "tracking_summary_json", tracking_summary);
	AppendJsonPath(json, "events_json", events_json_path);
	AppendJsonPath(json, "event_audit_report", AppendFileName(tracked_dir, "event_audit.md"));
	AppendJsonPath(json, "ocr_probe_json", ocr_json_path);
	AppendJsonPath(json, "semantic_dir", AppendFileName(tracked_dir, "semantic"));
	AppendJsonPath(json, "overlays_dir", AppendFileName(tracked_dir, "overlays"), false);
	json << "  }\n";
	json << "}\n";
	return SaveFile(summary_path, json);
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
	String audit_report = AppendFileName(exe_dir, "VideoEventAuditReport.exe");
	String ocr_probe = AppendFileName(exe_dir, "VideoSemanticOcrProbe.exe");
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
	int recorder_code = RunCommand(recorder, recorder_args);
	if(recorder_code != 0) {
		Cerr() << "ERROR: recorder failed\n";
		SetExitCode(1);
		return;
	}

	Vector<String> tracker_args;
	tracker_args << "--input-dir" << record_dir
	             << "--out" << tracked_dir;
	int tracker_code = RunCommand(tracker, tracker_args);
	if(tracker_code != 0) {
		Cerr() << "ERROR: tracker failed\n";
		SetExitCode(1);
		return;
	}

	String audit_path = AppendFileName(tracked_dir, "event_audit.md");
	bool audit_available = FileExists(audit_report);
	int audit_code = 0;
	if(audit_available) {
		Vector<String> audit_args;
		audit_args << "--tracker-dir" << tracked_dir;
		audit_code = RunCommand(audit_report, audit_args);
		if(audit_code != 0) {
			Cerr() << "ERROR: audit report failed\n";
			SetExitCode(1);
			return;
		}
	}
	else {
		Cout() << "audit_report_tool_missing=" << audit_report << "\n";
	}

	String ocr_probe_path = AppendFileName(tracked_dir, "ocr_probe.json");
	bool ocr_available = FileExists(ocr_probe);
	int ocr_code = 0;
	if(ocr_available) {
		Vector<String> ocr_args;
		ocr_args << "--tracker-dir" << tracked_dir << "--max-crops" << "12";
		ocr_code = RunCommand(ocr_probe, ocr_args);
		Cout() << "ocr_probe_exit=" << ocr_code << "\n";
		if(ocr_code != 0)
			Cout() << "ocr_probe_nonfatal=true\n";
	}
	else {
		Cout() << "ocr_probe_tool_missing=" << ocr_probe << "\n";
	}

	String pipeline_summary = AppendFileName(tracked_dir, "pipeline_summary.json");
	if(!WritePipelineSummary(opt, record_dir, tracked_dir, recorder_code, tracker_code,
	                         audit_code, audit_available, ocr_code, ocr_available)) {
		Cerr() << "ERROR: failed to write pipeline summary\n";
		SetExitCode(1);
		return;
	}

	Cout() << "regression_name=" << opt.name << "\n";
	Cout() << "record_dir=" << record_dir << "\n";
	Cout() << "record_summary=" << AppendFileName(record_dir, "summary.json") << "\n";
	Cout() << "tracked_dir=" << tracked_dir << "\n";
	Cout() << "tracking_json=" << AppendFileName(tracked_dir, "tracking.json") << "\n";
	Cout() << "tracking_summary_json=" << AppendFileName(tracked_dir, "tracking_summary.json") << "\n";
	Cout() << "events_json=" << AppendFileName(tracked_dir, "events.json") << "\n";
	Cout() << "event_audit_report=" << audit_path << "\n";
	Cout() << "ocr_probe_json=" << ocr_probe_path << "\n";
	Cout() << "pipeline_summary_json=" << pipeline_summary << "\n";
	Cout() << "semantic_dir=" << AppendFileName(tracked_dir, "semantic") << "\n";
	Cout() << "overlays_dir=" << AppendFileName(tracked_dir, "overlays") << "\n";
}
