#include "VideoTableVisualAuditReport.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoTableVisualAuditReport\n\n"
	       << "Usage: VideoTableVisualAuditReport --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>   VideoWindowTracker output directory\n"
	       << "  --out <file>          HTML output path (default <dir>/visual_audit.html)\n"
	       << "  --max-frames <n>      Maximum frames to render (default 3)\n"
	       << "  --help, -h            Show help\n";
}

static VisualAuditOptions ParseOptions(const Vector<String>& args)
{
	VisualAuditOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--max-frames" && i + 1 < args.GetCount())
			opt.max_frames = max(1, StrInt(args[++i]));
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_path.IsEmpty() && !opt.tracker_dir.IsEmpty())
		opt.out_path = AppendFileName(opt.tracker_dir, "visual_audit.html");
	return opt;
}

static String HtmlEscape(const String& text)
{
	String out;
	for(int i = 0; i < text.GetCount(); i++) {
		byte c = text[i];
		if(c == '&')
			out << "&amp;";
		else if(c == '<')
			out << "&lt;";
		else if(c == '>')
			out << "&gt;";
		else if(c == '"')
			out << "&quot;";
		else
			out.Cat(c);
	}
	return out;
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

static String RelPath(const String& from_file, const String& target)
{
	String from_dir = GetFileDirectory(NormalizePath(from_file));
	String normalized_target = NormalizePath(target);
	if(normalized_target.StartsWith(from_dir))
		return normalized_target.Mid(from_dir.GetCount());
	return normalized_target;
}

static String HtmlPath(const String& report_path, const String& path)
{
	return HtmlEscape(RelPath(report_path, path));
}

static String TextValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	if(IsVoid(value) || IsNull(value))
		return String();
	return IsString(value) ? String(value) : AsString(value);
}

static int IntValue(ValueMap map, const char *key, int fallback = 0)
{
	Value value = map.Get(key, Value());
	if(IsNumber(value))
		return (int)value;
	int parsed = StrInt(AsString(value));
	return IsNull(parsed) ? fallback : parsed;
}

static ValueMap MapValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueMap());
	return IsValueMap(value) ? ValueMap(value) : ValueMap();
}

static ValueArray ArrayValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueArray());
	return IsValueArray(value) ? ValueArray(value) : ValueArray();
}

static ValueMap LoadJsonMap(const String& path)
{
	String text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty())
		return ValueMap();
	Value value = ParseJSON(text);
	if(IsError(value) || !IsValueMap(value))
		return ValueMap();
	return ValueMap(value);
}

static String FindTableCrop(ValueMap tracking, int frame_index, int table_id)
{
	for(Value frame_value : ArrayValue(tracking, "frames")) {
		ValueMap frame = frame_value;
		if(IntValue(frame, "index") != frame_index)
			continue;
		for(Value table_value : ArrayValue(frame, "tables")) {
			ValueMap table = table_value;
			if(IntValue(table, "id") == table_id)
				return TextValue(table, "crop");
		}
	}
	return String();
}

static String FindFrameOverlay(ValueMap tracking, int frame_index)
{
	for(Value frame_value : ArrayValue(tracking, "frames")) {
		ValueMap frame = frame_value;
		if(IntValue(frame, "index") == frame_index)
			return TextValue(frame, "overlay");
	}
	return String();
}

static void AppendImage(String& html, const String& report_path, const String& label,
                        const String& path, const String& css_class = String())
{
	html << "<figure";
	if(!css_class.IsEmpty())
		html << " class=\"" << HtmlEscape(css_class) << "\"";
	html << ">";
	if(path.IsEmpty() || !FileExists(path))
		html << "<div class=\"missing\">missing: " << HtmlEscape(label) << "</div>";
	else
		html << "<a href=\"" << HtmlPath(report_path, path) << "\"><img src=\""
		     << HtmlPath(report_path, path) << "\" alt=\"" << HtmlEscape(label) << "\"></a>";
	html << "<figcaption>" << HtmlEscape(label) << "</figcaption></figure>\n";
}

static String SlotSummary(ValueArray slots)
{
	String out;
	for(int i = 0; i < slots.GetCount(); i++) {
		ValueMap slot = slots[i];
		if(!out.IsEmpty())
			out << " ";
		out << IntValue(slot, "index") << ":" << TextValue(slot, "present")
		    << "/" << TextValue(slot, "largest_component_ratio");
	}
	return out;
}

static void AppendTableState(String& html, const VisualAuditOptions& opt, ValueMap tracking,
                             ValueMap table)
{
	int frame_index = IntValue(table, "frame_index");
	int table_id = IntValue(table, "table_id");
	String table_crop = FindTableCrop(tracking, frame_index, table_id);
	html << "<section class=\"table-state\">\n";
	html << "<h3>Frame " << frame_index << " Table " << table_id << "</h3>\n";
	html << "<p>usable=<code>" << HtmlEscape(TextValue(table, "usable"))
	     << "</code> confidence=<code>" << HtmlEscape(TextValue(table, "confidence"))
	     << "</code> board_cards=<code>" << IntValue(table, "board_card_count")
	     << "</code> slots=<code>" << HtmlEscape(SlotSummary(ArrayValue(table, "board_slots")))
	     << "</code> pot=<code>" << HtmlEscape(OneLine(TextValue(table, "pot_text")))
	     << "</code> pot_amount=<code>" << HtmlEscape(OneLine(TextValue(table, "pot_amount_text")))
	     << "</code></p>\n";
	html << "<div class=\"image-row\">\n";
	AppendImage(html, opt.out_path, "table crop", table_crop, "table-crop");
	AppendImage(html, opt.out_path, "board crop", TextValue(table, "board_crop_path"), "board-crop");
	html << "</div>\n";
	html << "<h4>Board Slots</h4><div class=\"image-row small\">\n";
	for(Value slot_value : ArrayValue(table, "board_slots")) {
		ValueMap slot = slot_value;
		String label = Format("slot %d present=%s ratio=%s", IntValue(slot, "index"),
		                      TextValue(slot, "present"), TextValue(slot, "largest_component_ratio"));
		AppendImage(html, opt.out_path, label, TextValue(slot, "crop_path"), "slot-crop");
	}
	html << "</div>\n";
	html << "<h4>Seats</h4><div class=\"image-row small\">\n";
	for(Value seat_value : ArrayValue(table, "seats")) {
		ValueMap seat = seat_value;
		String label = Format("seat %d %s name=%s stack=%s action=%s",
		                      IntValue(seat, "index"), TextValue(seat, "role"),
		                      OneLine(TextValue(seat, "name")),
		                      OneLine(TextValue(seat, "stack_text")),
		                      OneLine(TextValue(seat, "action")));
		AppendImage(html, opt.out_path, label, TextValue(seat, "crop_path"), "seat-crop");
	}
	html << "</div>\n";
	html << "</section>\n";
}

static bool WriteReport(const VisualAuditOptions& opt)
{
	String tracking_path = AppendFileName(opt.tracker_dir, "tracking_summary.json");
	String state_path = AppendFileName(opt.tracker_dir, "table_state.json");
	ValueMap tracking = LoadJsonMap(tracking_path);
	ValueMap state = LoadJsonMap(state_path);
	if(tracking.IsEmpty() || state.IsEmpty()) {
		Cerr() << "ERROR: missing or invalid tracking_summary.json/table_state.json in " << opt.tracker_dir << "\n";
		return false;
	}
	String html;
	html << "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">"
	     << "<title>Video Table Visual Audit</title>\n"
	     << "<style>body{font-family:Arial,sans-serif;margin:20px;background:#111;color:#eee;}"
	     << "a{color:#9cf}.image-row{display:flex;gap:12px;flex-wrap:wrap;align-items:flex-start;}"
	     << "figure{margin:0 0 12px 0;background:#222;padding:8px;border:1px solid #444;}"
	     << "figcaption{font-size:12px;color:#ccc;margin-top:4px;}img{max-width:420px;max-height:280px;image-rendering:auto;}"
	     << ".small img{max-width:160px;max-height:120px}.table-state{border-top:2px solid #555;padding-top:14px;margin-top:18px;}"
	     << ".missing{width:160px;height:80px;border:1px dashed #777;color:#f99;padding:8px;}code{color:#ffd27f;}"
	     << "</style></head><body>\n";
	html << "<h1>Video Table Visual Audit</h1>\n";
	html << "<p>tracker_dir=<code>" << HtmlEscape(opt.tracker_dir) << "</code> "
	     << "table_mode=<code>" << HtmlEscape(TextValue(state, "table_mode")) << "</code> "
	     << "state_count=<code>" << IntValue(state, "state_count") << "</code></p>\n";
	html << "<p><a href=\"" << HtmlPath(opt.out_path, tracking_path) << "\">tracking_summary.json</a> "
	     << "<a href=\"" << HtmlPath(opt.out_path, state_path) << "\">table_state.json</a></p>\n";

	int frames_rendered = 0;
	for(Value frame_value : ArrayValue(tracking, "frames")) {
		if(frames_rendered >= opt.max_frames)
			break;
		ValueMap frame = frame_value;
		html << "<section class=\"frame\"><h2>Frame " << IntValue(frame, "index") << "</h2>\n";
		AppendImage(html, opt.out_path, "frame overlay", TextValue(frame, "overlay"), "frame-overlay");
		html << "</section>\n";
		frames_rendered++;
	}

	int tables_rendered = 0;
	for(Value table_value : ArrayValue(state, "tables")) {
		ValueMap table = table_value;
		if(IntValue(table, "frame_index") >= opt.max_frames)
			continue;
		AppendTableState(html, opt, tracking, table);
		tables_rendered++;
	}
	html << "</body></html>\n";
	if(!SaveFile(opt.out_path, html)) {
		Cerr() << "ERROR: failed to write " << opt.out_path << "\n";
		return false;
	}
	Cout() << "visual_audit_report=" << opt.out_path << "\n";
	Cout() << "frames_rendered=" << frames_rendered << " tables_rendered=" << tables_rendered << "\n";
	return true;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	VisualAuditOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!WriteReport(opt))
		SetExitCode(1);
}
