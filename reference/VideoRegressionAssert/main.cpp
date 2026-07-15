#include "VideoRegressionAssert.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoRegressionAssert\n\n"
	       << "Usage: VideoRegressionAssert --tracker-dir <dir> [expectations]\n\n"
	       << "Inputs:\n"
	       << "  --tracker-dir <dir>        Tracker output directory\n"
	       << "  --pipeline-summary <file>  Override pipeline_summary.json path\n"
	       << "  --events-json <file>       Override events.json path\n"
	       << "  --ocr-json <file>          Override ocr_probe.json path\n\n"
	       << "  --table-quality-json <file> Override table_quality.json path\n\n"
	       << "  --table-mode <mode>         Require table_mode in pipeline/quality JSON\n\n"
	       << "Expectations:\n"
	       << "  --expect-frames <n>        Require exact frames_tracked\n"
	       << "  --min-frames <n>           Require at least n frames_tracked\n"
	       << "  --expect-tables <n>        Require exact max_frame_tables\n"
	       << "  --min-tables <n>           Require at least n max_frame_tables\n"
	       << "  --min-events <n>           Require at least n events\n"
	       << "  --min-ocr-crops <n>        Require at least n OCR crops\n"
	       << "  --min-usable-tables <n>    Require at least n usable table contents\n"
	       << "  --require-event <type>     Require an event type; can repeat\n"
	       << "  --ocr-ok, --require-ocr    Require OCR probe ok=true\n"
	       << "  --require-ocr-text <text>  Require OCR result text substring; can repeat\n"
	       << "  --help, -h                 Show help\n";
}

static VideoAssertOptions ParseOptions(const Vector<String>& args)
{
	VideoAssertOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--pipeline-summary" && i + 1 < args.GetCount())
			opt.pipeline_summary = args[++i];
		else if(args[i] == "--events-json" && i + 1 < args.GetCount())
			opt.events_json = args[++i];
		else if(args[i] == "--ocr-json" && i + 1 < args.GetCount())
			opt.ocr_json = args[++i];
		else if(args[i] == "--table-quality-json" && i + 1 < args.GetCount())
			opt.table_quality_json = args[++i];
		else if(args[i] == "--table-mode" && i + 1 < args.GetCount())
			opt.table_mode = ToLower(args[++i]);
		else if(args[i] == "--expect-frames" && i + 1 < args.GetCount())
			opt.expect_frames = StrInt(args[++i]);
		else if(args[i] == "--min-frames" && i + 1 < args.GetCount())
			opt.min_frames = StrInt(args[++i]);
		else if(args[i] == "--expect-tables" && i + 1 < args.GetCount())
			opt.expect_tables = StrInt(args[++i]);
		else if(args[i] == "--min-tables" && i + 1 < args.GetCount())
			opt.min_tables = StrInt(args[++i]);
		else if(args[i] == "--min-events" && i + 1 < args.GetCount())
			opt.min_events = StrInt(args[++i]);
		else if(args[i] == "--min-ocr-crops" && i + 1 < args.GetCount())
			opt.min_ocr_crops = StrInt(args[++i]);
		else if(args[i] == "--min-usable-tables" && i + 1 < args.GetCount())
			opt.min_usable_tables = StrInt(args[++i]);
		else if(args[i] == "--require-event" && i + 1 < args.GetCount())
			opt.required_events << args[++i];
		else if(args[i] == "--ocr-ok" || args[i] == "--require-ocr")
			opt.require_ocr_ok = true;
		else if(args[i] == "--require-ocr-text" && i + 1 < args.GetCount())
			opt.required_ocr_texts << args[++i];
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
		else {
			Cerr() << "ERROR: unknown or incomplete argument: " << args[i] << "\n";
			opt.parse_error = true;
		}
	}
	if(!opt.tracker_dir.IsEmpty()) {
		if(opt.pipeline_summary.IsEmpty())
			opt.pipeline_summary = AppendFileName(opt.tracker_dir, "pipeline_summary.json");
		if(opt.events_json.IsEmpty())
			opt.events_json = AppendFileName(opt.tracker_dir, "events.json");
		if(opt.ocr_json.IsEmpty())
			opt.ocr_json = AppendFileName(opt.tracker_dir, "ocr_probe.json");
		if(opt.table_quality_json.IsEmpty())
			opt.table_quality_json = AppendFileName(opt.tracker_dir, "table_quality.json");
	}
	return opt;
}

static bool LoadJsonMap(const String& path, String& text, ValueMap& map, const char *label)
{
	text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty()) {
		Cout() << "FAIL " << label << " missing_or_empty path=" << path << "\n";
		return false;
	}
	Value value = ParseJSON(text);
	if(IsError(value)) {
		Cout() << "FAIL " << label << " parse_error path=" << path << "\n";
		return false;
	}
	map = value;
	Cout() << "PASS " << label << " loaded path=" << path << "\n";
	return true;
}

static ValueMap MapValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueMap());
	if(IsValueMap(value))
		return ValueMap(value);
	return ValueMap();
}

static int IntValue(ValueMap map, const char *key, int fallback = -1)
{
	Value value = map.Get(key, Value());
	if(IsNumber(value))
		return (int)value;
	return StrInt(AsString(value));
}

static bool BoolValue(ValueMap map, const char *key, bool fallback = false)
{
	Value value = map.Get(key, Value());
	if(IsNumber(value))
		return (int)value != 0;
	String text = ToLower(AsString(value));
	if(text == "true" || text == "1")
		return true;
	if(text == "false" || text == "0")
		return false;
	return fallback;
}

static String TextValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	if(IsVoid(value) || IsNull(value))
		return String();
	return IsString(value) ? String(value) : AsString(value);
}

static bool CheckIntExact(const char *name, int actual, int expected)
{
	if(expected < 0)
		return true;
	if(actual == expected) {
		Cout() << "PASS " << name << " actual=" << actual << " expected=" << expected << "\n";
		return true;
	}
	Cout() << "FAIL " << name << " actual=" << actual << " expected=" << expected << "\n";
	return false;
}

static bool CheckIntMin(const char *name, int actual, int expected)
{
	if(expected < 0)
		return true;
	if(actual >= expected) {
		Cout() << "PASS " << name << " actual=" << actual << " min=" << expected << "\n";
		return true;
	}
	Cout() << "FAIL " << name << " actual=" << actual << " min=" << expected << "\n";
	return false;
}

static bool CheckTextExact(const char *name, const String& actual, const String& expected)
{
	if(expected.IsEmpty())
		return true;
	if(actual == expected) {
		Cout() << "PASS " << name << " actual=" << actual << " expected=" << expected << "\n";
		return true;
	}
	Cout() << "FAIL " << name << " actual=" << actual << " expected=" << expected << "\n";
	return false;
}

static bool HasEventType(ValueMap events_root, const String& type)
{
	ValueArray events = events_root.Get("events", ValueArray());
	for(int i = 0; i < events.GetCount(); i++) {
		ValueMap event = events[i];
		if(TextValue(event, "type") == type)
			return true;
	}
	return false;
}

static bool OcrContains(ValueMap ocr_root, const String& needle)
{
	ValueArray results = ocr_root.Get("results", ValueArray());
	for(int i = 0; i < results.GetCount(); i++) {
		ValueMap result = results[i];
		if(TextValue(result, "text").Find(needle) >= 0)
			return true;
	}
	return false;
}

static bool RunAssertions(const VideoAssertOptions& opt)
{
	VideoAssertContext ctx;
	ctx.pipeline_ok = LoadJsonMap(opt.pipeline_summary, ctx.pipeline_text, ctx.pipeline, "pipeline_summary");
	ctx.events_ok = LoadJsonMap(opt.events_json, ctx.events_text, ctx.events_root, "events_json");
	ctx.ocr_ok = LoadJsonMap(opt.ocr_json, ctx.ocr_text, ctx.ocr_root, "ocr_json");
	ValueMap table_quality_root;
	ctx.table_quality_ok = LoadJsonMap(opt.table_quality_json, ctx.table_quality_text,
	                                   table_quality_root, "table_quality_json");

	bool ok = ctx.pipeline_ok && ctx.events_ok && ctx.ocr_ok && ctx.table_quality_ok;
	ValueMap observed = MapValue(ctx.pipeline, "observed");
	int frames_tracked = IntValue(observed, "frames_tracked");
	int max_frame_tables = IntValue(observed, "max_frame_tables");
	int event_count = IntValue(ctx.events_root, "event_count");
	int ocr_crop_count = IntValue(ctx.ocr_root, "crop_count");
	int max_usable_tables = IntValue(table_quality_root, "max_usable_tables");
	bool ocr_ok = BoolValue(ctx.ocr_root, "ok");
	String pipeline_mode = TextValue(ctx.pipeline, "table_mode");
	String quality_mode = TextValue(table_quality_root, "table_mode");

	ok = CheckIntExact("frames_tracked", frames_tracked, opt.expect_frames) && ok;
	ok = CheckIntMin("frames_tracked", frames_tracked, opt.min_frames) && ok;
	ok = CheckIntExact("max_frame_tables", max_frame_tables, opt.expect_tables) && ok;
	ok = CheckIntMin("max_frame_tables", max_frame_tables, opt.min_tables) && ok;
	ok = CheckIntMin("event_count", event_count, opt.min_events) && ok;
	ok = CheckIntMin("ocr_crop_count", ocr_crop_count, opt.min_ocr_crops) && ok;
	ok = CheckIntMin("max_usable_tables", max_usable_tables, opt.min_usable_tables) && ok;
	ok = CheckTextExact("pipeline_table_mode", pipeline_mode, opt.table_mode) && ok;
	ok = CheckTextExact("quality_table_mode", quality_mode, opt.table_mode) && ok;

	for(const String& type : opt.required_events) {
		if(HasEventType(ctx.events_root, type))
			Cout() << "PASS required_event type=" << type << "\n";
		else {
			Cout() << "FAIL required_event type=" << type << "\n";
			ok = false;
		}
	}

	if(opt.require_ocr_ok) {
		if(ocr_ok)
			Cout() << "PASS ocr_ok true\n";
		else {
			Cout() << "FAIL ocr_ok false error=\"" << TextValue(ctx.ocr_root, "error") << "\"\n";
			ok = false;
		}
	}

	for(const String& needle : opt.required_ocr_texts) {
		if(OcrContains(ctx.ocr_root, needle))
			Cout() << "PASS required_ocr_text text=\"" << needle << "\"\n";
		else {
			Cout() << "FAIL required_ocr_text text=\"" << needle << "\"\n";
			ok = false;
		}
	}

	Cout() << (ok ? "ASSERTIONS_PASS" : "ASSERTIONS_FAIL") << "\n";
	return ok;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	VideoAssertOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty() || opt.parse_error) {
		PrintHelp();
		if((opt.tracker_dir.IsEmpty() || opt.parse_error) && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunAssertions(opt))
		SetExitCode(1);
}
