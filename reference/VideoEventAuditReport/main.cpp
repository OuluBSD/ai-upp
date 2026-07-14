#include "VideoEventAuditReport.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoEventAuditReport\n\n"
	       << "Usage: VideoEventAuditReport --tracker-dir <dir> [--out <file>]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>   VideoWindowTracker output directory\n"
	       << "  --out <file>          Markdown output path (default <dir>/event_audit.md)\n"
	       << "  --help, -h            Show help\n";
}

static AuditReportOptions ParseOptions(const Vector<String>& args)
{
	AuditReportOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_path.IsEmpty() && !opt.tracker_dir.IsEmpty())
		opt.out_path = AppendFileName(opt.tracker_dir, "event_audit.md");
	return opt;
}

static String JsonStringValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsString(value) ? String(value) : AsString(value);
}

static int JsonIntValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsNumber(value) ? (int)value : StrInt(AsString(value));
}

static String RelPath(const String& from_file, const String& target)
{
	String from_dir = GetFileDirectory(NormalizePath(from_file));
	String normalized_target = NormalizePath(target);
	if(normalized_target.StartsWith(from_dir))
		return normalized_target.Mid(from_dir.GetCount());
	return normalized_target;
}

static String MarkdownLink(const String& label, const String& report_path, const String& path)
{
	if(path.IsEmpty() || !FileExists(path))
		return label + " missing";
	return "[" + label + "](" + RelPath(report_path, path) + ")";
}

static String SemanticCropPath(const String& tracker_dir, int frame_index, int table_id,
                               const String& semantic)
{
	String name = semantic;
	if(name == "seat_regions")
		name = "top_seat";
	String dir = AppendFileName(AppendFileName(tracker_dir, "semantic"),
	                            Format("frame_%06d_table_%d", frame_index, table_id));
	return AppendFileName(dir, name + ".jpg");
}

static String TableCropPath(const String& tracker_dir, int frame_index, int table_id)
{
	return AppendFileName(AppendFileName(tracker_dir, "crops"),
	                      Format("frame_%06d_table_%d.jpg", frame_index, table_id));
}

static String ChangeOverlayPath(const String& tracker_dir, int frame_index, int table_id)
{
	return AppendFileName(AppendFileName(tracker_dir, "overlays"),
	                      Format("frame_%06d_table_%d_changes.jpg", frame_index, table_id));
}

static String FrameOverlayPath(const String& tracker_dir, int frame_index)
{
	return AppendFileName(AppendFileName(tracker_dir, "overlays"),
	                      Format("frame_%06d_windows.jpg", frame_index));
}

static bool GenerateReport(const AuditReportOptions& opt)
{
	String events_path = AppendFileName(opt.tracker_dir, "events.json");
	String summary_path = AppendFileName(opt.tracker_dir, "tracking_summary.json");
	String events_text = LoadFile(events_path);
	if(events_text.IsVoid() || events_text.IsEmpty()) {
		Cerr() << "ERROR: missing or empty " << events_path << "\n";
		return false;
	}
	Value root_value = ParseJSON(events_text);
	if(IsError(root_value)) {
		Cerr() << "ERROR: failed to parse " << events_path << "\n";
		return false;
	}
	ValueMap root = root_value;
	ValueArray events = root.Get("events", ValueArray());

	String md;
	md << "# Video Event Audit\n\n";
	md << "- tracker_dir: `" << opt.tracker_dir << "`\n";
	md << "- events: " << events.GetCount() << "\n";
	md << "- " << MarkdownLink("events.json", opt.out_path, events_path) << "\n";
	md << "- " << MarkdownLink("tracking_summary.json", opt.out_path, summary_path) << "\n\n";

	if(events.IsEmpty()) {
		md << "No event candidates emitted.\n";
	}
	for(int i = 0; i < events.GetCount(); i++) {
		ValueMap event = events[i];
		int frame_index = JsonIntValue(event, "frame_index");
		int table_id = JsonIntValue(event, "table_id");
		String type = JsonStringValue(event, "type");
		String reason = JsonStringValue(event, "reason");
		String confidence = JsonStringValue(event, "confidence");
		ValueMap evidence = event.Get("evidence", ValueMap());
		String semantic = JsonStringValue(evidence, "semantic");
		int change_blocks = JsonIntValue(evidence, "change_blocks");

		md << "## Event " << i << ": `" << type << "`\n\n";
		md << "- frame_index: " << frame_index << "\n";
		md << "- table_id: " << table_id << "\n";
		md << "- confidence: " << confidence << "\n";
		md << "- reason: `" << reason << "`\n";
		md << "- evidence: `" << semantic << "` blocks=" << change_blocks << "\n";
		md << "- " << MarkdownLink("frame overlay", opt.out_path, FrameOverlayPath(opt.tracker_dir, frame_index)) << "\n";
		md << "- " << MarkdownLink("table crop", opt.out_path, TableCropPath(opt.tracker_dir, frame_index, table_id)) << "\n";
		md << "- " << MarkdownLink("semantic crop", opt.out_path, SemanticCropPath(opt.tracker_dir, frame_index, table_id, semantic)) << "\n";
		md << "- " << MarkdownLink("change overlay", opt.out_path, ChangeOverlayPath(opt.tracker_dir, frame_index, table_id)) << "\n\n";
	}

	if(!SaveFile(opt.out_path, md)) {
		Cerr() << "ERROR: failed to write " << opt.out_path << "\n";
		return false;
	}
	Cout() << "audit_report=" << opt.out_path << "\n";
	return true;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	AuditReportOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!GenerateReport(opt))
		SetExitCode(1);
}
