#include "VideoTableQuality.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoTableQuality\n\n"
	       << "Usage: VideoTableQuality --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>  Tracker output directory\n"
	       << "  --ocr-json <file>    Override ocr_probe.json path\n"
	       << "  --out <file>         Output JSON path (default <dir>/table_quality.json)\n"
	       << "  --table-mode <mode>  Table perspective: unknown, hero, observer (default unknown)\n"
	       << "  --help, -h           Show help\n";
}

static bool IsValidTableMode(const String& mode)
{
	return mode == "unknown" || mode == "hero" || mode == "observer";
}

static TableQualityOptions ParseOptions(const Vector<String>& args)
{
	TableQualityOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--ocr-json" && i + 1 < args.GetCount())
			opt.ocr_json = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--table-mode" && i + 1 < args.GetCount())
			opt.table_mode = ToLower(args[++i]);
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(!IsValidTableMode(opt.table_mode))
		opt.table_mode = "unknown";
	if(!opt.tracker_dir.IsEmpty()) {
		if(opt.ocr_json.IsEmpty())
			opt.ocr_json = AppendFileName(opt.tracker_dir, "ocr_probe.json");
		if(opt.out_path.IsEmpty())
			opt.out_path = AppendFileName(opt.tracker_dir, "table_quality.json");
	}
	return opt;
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

static String TextValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsString(value) ? String(value) : AsString(value);
}

static int IntValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, Value());
	return IsNumber(value) ? (int)value : StrInt(AsString(value));
}

static bool ContainsAny(const String& text, std::initializer_list<const char *> needles)
{
	String lower = ToLower(text);
	for(const char *needle : needles)
		if(lower.Find(ToLower(String(needle))) >= 0)
			return true;
	return false;
}

static int FindEntry(Vector<TableQualityEntry>& entries, int frame_index, int table_id)
{
	for(int i = 0; i < entries.GetCount(); i++)
		if(entries[i].frame_index == frame_index && entries[i].table_id == table_id)
			return i;
	return -1;
}

static TableQualityEntry& GetEntry(Vector<TableQualityEntry>& entries, int frame_index, int table_id)
{
	int index = FindEntry(entries, frame_index, table_id);
	if(index >= 0)
		return entries[index];
	TableQualityEntry& entry = entries.Add();
	entry.frame_index = frame_index;
	entry.table_id = table_id;
	return entry;
}

static void Finalize(TableQualityEntry& entry)
{
	entry.has_title_anchor = ContainsAny(entry.title_text, {"hold'em", "no limit", "limit hold"});
	entry.has_pot_anchor = ContainsAny(entry.pot_text, {"pot:"});
	entry.has_obstruction_text = ContainsAny(entry.title_text + " " + entry.pot_text,
	                                         {"pokerstars lobby", "logged out", "login",
	                                          "responsible gaming", "quick seat", "real money",
	                                          "play money"});
	entry.usable = entry.has_title_anchor && entry.has_pot_anchor && !entry.has_obstruction_text;
	if(entry.usable)
		entry.reason = "title_and_pot_anchors";
	else if(entry.has_obstruction_text)
		entry.reason = "obstruction_text";
	else if(!entry.has_pot_anchor)
		entry.reason = "missing_pot_anchor";
	else if(!entry.has_title_anchor)
		entry.reason = "missing_title_anchor";
	else
		entry.reason = "unknown";
}

static Vector<TableQualityEntry> LoadQualityEntries(const TableQualityOptions& opt)
{
	Vector<TableQualityEntry> entries;
	String text = LoadFile(opt.ocr_json);
	if(text.IsVoid() || text.IsEmpty())
		return entries;
	Value value = ParseJSON(text);
	if(IsError(value))
		return entries;
	ValueMap root = value;
	ValueArray results = root.Get("results", ValueArray());
	for(int i = 0; i < results.GetCount(); i++) {
		ValueMap result = results[i];
		int frame_index = IntValue(result, "frame_index");
		int table_id = IntValue(result, "table_id");
		String semantic = TextValue(result, "semantic");
		String ocr_text = TextValue(result, "text");
		TableQualityEntry& entry = GetEntry(entries, frame_index, table_id);
		if(semantic == "title")
			entry.title_text = ocr_text;
		else if(semantic == "pot_label")
			entry.pot_text = ocr_text;
	}
	for(TableQualityEntry& entry : entries)
		Finalize(entry);
	Sort(entries, [](const TableQualityEntry& a, const TableQualityEntry& b) {
		if(a.frame_index != b.frame_index)
			return a.frame_index < b.frame_index;
		return a.table_id < b.table_id;
	});
	return entries;
}

static bool SaveQuality(const TableQualityOptions& opt, const Vector<TableQualityEntry>& entries)
{
	int usable = 0;
	int obstructed = 0;
	int max_usable_tables = 0;
	VectorMap<int, int> usable_by_frame;
	for(const TableQualityEntry& entry : entries) {
		if(entry.usable) {
			usable++;
			int index = usable_by_frame.Find(entry.frame_index);
			if(index < 0)
				usable_by_frame.Add(entry.frame_index, 1);
			else
				usable_by_frame[index]++;
		}
		else
			obstructed++;
	}
	for(int count : usable_by_frame.GetValues())
		max_usable_tables = max(max_usable_tables, count);

	String json;
	json << "{\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"ocr_json\": \"" << JsonString(opt.ocr_json) << "\",\n";
	json << "  \"table_mode\": \"" << JsonString(opt.table_mode) << "\",\n";
	json << "  \"hero_cards_expected\": " << (opt.table_mode == "hero" ? "true" : "false") << ",\n";
	json << "  \"observer_nohero\": " << (opt.table_mode == "observer" ? "true" : "false") << ",\n";
	json << "  \"table_quality_count\": " << entries.GetCount() << ",\n";
	json << "  \"usable_table_count\": " << usable << ",\n";
	json << "  \"obstructed_table_count\": " << obstructed << ",\n";
	json << "  \"max_usable_tables\": " << max_usable_tables << ",\n";
	json << "  \"tables\": [\n";
	for(int i = 0; i < entries.GetCount(); i++) {
		const TableQualityEntry& entry = entries[i];
		if(i)
			json << ",\n";
		json << "    {\"frame_index\": " << entry.frame_index
		     << ", \"table_id\": " << entry.table_id
		     << ", \"usable\": " << (entry.usable ? "true" : "false")
		     << ", \"reason\": \"" << JsonString(entry.reason)
		     << "\", \"has_title_anchor\": " << (entry.has_title_anchor ? "true" : "false")
		     << ", \"has_pot_anchor\": " << (entry.has_pot_anchor ? "true" : "false")
		     << ", \"has_obstruction_text\": " << (entry.has_obstruction_text ? "true" : "false")
		     << ", \"title_text\": \"" << JsonString(entry.title_text)
		     << "\", \"pot_text\": \"" << JsonString(entry.pot_text) << "\"}";
	}
	json << "\n  ]\n";
	json << "}\n";
	if(!SaveFile(opt.out_path, json))
		return false;
	Cout() << "table_quality_json=" << opt.out_path << "\n";
	Cout() << "table_mode=" << opt.table_mode
	       << " hero_cards_expected=" << (opt.table_mode == "hero" ? "true" : "false") << "\n";
	Cout() << "usable_table_count=" << usable << " max_usable_tables=" << max_usable_tables
	       << " obstructed_table_count=" << obstructed << "\n";
	return true;
}

static bool RunQuality(const TableQualityOptions& opt)
{
	return SaveQuality(opt, LoadQualityEntries(opt));
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	TableQualityOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunQuality(opt))
		SetExitCode(1);
}
