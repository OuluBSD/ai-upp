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

static String OneLine(String text)
{
	text.Replace("\r", " ");
	text.Replace("\n", " ");
	text.Replace("\t", " ");
	while(text.Find("  ") >= 0)
		text.Replace("  ", " ");
	return TrimBoth(text);
}

static String JsonTextValue(ValueMap map, const char *key)
{
	return JsonStringValue(map, key);
}

static bool JsonBoolValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	if(IsNumber(value))
		return (int)value != 0;
	String text = ToLower(AsString(value));
	return text == "true" || text == "1";
}

static void AppendOcrSummary(String& md, const AuditReportOptions& opt)
{
	String ocr_path = AppendFileName(opt.tracker_dir, "ocr_probe.json");
	if(!FileExists(ocr_path)) {
		md << "## OCR Probe\n\n";
		md << "- status: missing\n";
		md << "- " << MarkdownLink("ocr_probe.json", opt.out_path, ocr_path) << "\n\n";
		return;
	}
	String text = LoadFile(ocr_path);
	if(text.IsVoid() || text.IsEmpty()) {
		md << "## OCR Probe\n\n";
		md << "- status: empty\n";
		md << "- " << MarkdownLink("ocr_probe.json", opt.out_path, ocr_path) << "\n\n";
		return;
	}
	Value value = ParseJSON(text);
	if(IsError(value)) {
		md << "## OCR Probe\n\n";
		md << "- status: parse_error\n";
		md << "- " << MarkdownLink("ocr_probe.json", opt.out_path, ocr_path) << "\n\n";
		return;
	}
	ValueMap root = value;
	bool ok = JsonBoolValue(root, "ok");
	int crop_count = JsonIntValue(root, "crop_count");
	String error = JsonTextValue(root, "error");
	ValueArray results = root.Get("results", ValueArray());
	md << "## OCR Probe\n\n";
	md << "- status: " << (ok ? "ok" : "failed_nonfatal") << "\n";
	md << "- crop_count: " << crop_count << "\n";
	if(!error.IsEmpty())
		md << "- error: `" << error << "`\n";
	md << "- " << MarkdownLink("ocr_probe.json", opt.out_path, ocr_path) << "\n";
	int shown = 0;
	for(int i = 0; i < results.GetCount() && shown < 8; i++) {
		ValueMap result = results[i];
		String result_text = OneLine(JsonTextValue(result, "text"));
		if(result_text.IsEmpty())
			continue;
		md << "- text[" << shown << "]: frame=" << JsonIntValue(result, "frame_index")
		   << " table=" << JsonIntValue(result, "table_id")
		   << " semantic=`" << JsonTextValue(result, "semantic")
		   << "` exit=" << JsonIntValue(result, "exit_code")
		   << " `" << result_text << "`\n";
		shown++;
	}
	if(ok && shown == 0)
		md << "- text: no non-empty OCR snippets\n";
	md << "\n";
}

static void AppendCorrelatedSummary(String& md, const AuditReportOptions& opt)
{
	String path = AppendFileName(opt.tracker_dir, "correlated_events.json");
	if(!FileExists(path))
		return;
	String text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty())
		return;
	Value value = ParseJSON(text);
	if(IsError(value))
		return;
	ValueMap root = value;
	ValueArray events = root.Get("events", ValueArray());
	md << "## Correlated Events\n\n";
	md << "- correlated_event_count: " << JsonIntValue(root, "correlated_event_count") << "\n";
	md << "- raw_event_count: " << JsonIntValue(root, "raw_event_count") << "\n";
	md << "- " << MarkdownLink("correlated_events.json", opt.out_path, path) << "\n";
	for(int i = 0; i < events.GetCount(); i++) {
		ValueMap event = events[i];
		md << "- transition[" << i << "]: kind=`" << JsonTextValue(event, "kind")
		   << "` table=" << JsonIntValue(event, "table_id")
		   << " frames=" << JsonIntValue(event, "start_frame")
		   << ".." << JsonIntValue(event, "end_frame")
		   << " raw=" << JsonIntValue(event, "raw_event_count")
		   << " confidence=" << JsonTextValue(event, "confidence")
		   << " types=`" << OneLine(AsString(event.Get("types", ValueArray()))) << "`\n";
	}
	md << "\n";
}

static void AppendTableQualitySummary(String& md, const AuditReportOptions& opt)
{
	String path = AppendFileName(opt.tracker_dir, "table_quality.json");
	if(!FileExists(path))
		return;
	String text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty())
		return;
	Value value = ParseJSON(text);
	if(IsError(value))
		return;
	ValueMap root = value;
	ValueArray tables = root.Get("tables", ValueArray());
	md << "## Table Quality\n\n";
	md << "- usable_table_count: " << JsonIntValue(root, "usable_table_count") << "\n";
	md << "- max_usable_tables: " << JsonIntValue(root, "max_usable_tables") << "\n";
	md << "- obstructed_table_count: " << JsonIntValue(root, "obstructed_table_count") << "\n";
	md << "- " << MarkdownLink("table_quality.json", opt.out_path, path) << "\n";
	for(int i = 0; i < tables.GetCount(); i++) {
		ValueMap table = tables[i];
		md << "- table[" << i << "]: frame=" << JsonIntValue(table, "frame_index")
		   << " table=" << JsonIntValue(table, "table_id")
		   << " usable=" << JsonTextValue(table, "usable")
		   << " reason=`" << JsonTextValue(table, "reason") << "`"
		   << " pot=`" << OneLine(JsonTextValue(table, "pot_text")) << "`\n";
	}
	md << "\n";
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
	String pipeline_summary_path = AppendFileName(opt.tracker_dir, "pipeline_summary.json");
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
	if(FileExists(pipeline_summary_path)) {
		Value pipeline_value = ParseJSON(LoadFile(pipeline_summary_path));
		if(!IsError(pipeline_value)) {
			ValueMap pipeline = pipeline_value;
			md << "- table_mode: `" << JsonTextValue(pipeline, "table_mode") << "`\n";
			md << "- hero_cards_expected: `" << JsonTextValue(pipeline, "hero_cards_expected") << "`\n";
			md << "- observer_nohero: `" << JsonTextValue(pipeline, "observer_nohero") << "`\n";
		}
	}
	md << "- events: " << events.GetCount() << "\n";
	md << "- " << MarkdownLink("events.json", opt.out_path, events_path) << "\n";
	md << "- " << MarkdownLink("tracking_summary.json", opt.out_path, summary_path) << "\n\n";
	if(FileExists(pipeline_summary_path))
		md << "- " << MarkdownLink("pipeline_summary.json", opt.out_path, pipeline_summary_path) << "\n\n";
	AppendOcrSummary(md, opt);
	AppendTableQualitySummary(md, opt);
	AppendCorrelatedSummary(md, opt);

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
