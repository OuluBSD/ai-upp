#include "VideoEventCorrelator.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoEventCorrelator\n\n"
	       << "Usage: VideoEventCorrelator --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>  VideoWindowTracker output directory\n"
	       << "  --events-json <file> Override events.json path\n"
	       << "  --out <file>         Output JSON path (default <dir>/correlated_events.json)\n"
	       << "  --frame-gap <n>      Merge events when frame gap is <= n (default 1)\n"
	       << "  --help, -h           Show help\n";
}

static EventCorrelatorOptions ParseOptions(const Vector<String>& args)
{
	EventCorrelatorOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--events-json" && i + 1 < args.GetCount())
			opt.events_json = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--frame-gap" && i + 1 < args.GetCount())
			opt.frame_gap = max(0, StrInt(args[++i]));
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(!opt.tracker_dir.IsEmpty()) {
		if(opt.events_json.IsEmpty())
			opt.events_json = AppendFileName(opt.tracker_dir, "events.json");
		if(opt.out_path.IsEmpty())
			opt.out_path = AppendFileName(opt.tracker_dir, "correlated_events.json");
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

static int IntValue(ValueMap map, const char *key, int fallback = 0)
{
	Value value = map.Get(key, Value());
	return IsNumber(value) ? (int)value : Nvl(StrInt(AsString(value)), fallback);
}

static double DoubleValue(ValueMap map, const char *key, double fallback = 0)
{
	Value value = map.Get(key, Value());
	return IsNumber(value) ? (double)value : ScanDouble(AsString(value));
}

static void AddUnique(Vector<String>& values, const String& value)
{
	if(value.IsEmpty())
		return;
	for(const String& existing : values)
		if(existing == value)
			return;
	values << value;
}

static String EventKind(const String& type)
{
	if(type == "seat_activity")
		return "seat_transition";
	if(type == "board_changed" || type == "pot_changed" || type == "chips_changed" ||
	   type == "button_changed")
		return "table_transition";
	return type;
}

static bool IsPrimaryEvent(const String& type)
{
	return type == "board_changed" || type == "pot_changed" || type == "chips_changed";
}

static bool SameTransition(const CorrelatedVideoEvent& group, const RawVideoEvent& event, int frame_gap)
{
	if(group.table_id != event.table_id || event.frame_index > group.end_frame + frame_gap)
		return false;
	if(group.kind == "table_transition")
		return true;
	return group.kind == EventKind(event.type);
}

static Vector<RawVideoEvent> LoadEvents(const EventCorrelatorOptions& opt, int& frame_count,
                                        String& input_dir)
{
	Vector<RawVideoEvent> out;
	String text = LoadFile(opt.events_json);
	if(text.IsVoid() || text.IsEmpty())
		return out;
	Value root_value = ParseJSON(text);
	if(IsError(root_value))
		return out;
	ValueMap root = root_value;
	frame_count = IntValue(root, "frame_count", 0);
	input_dir = TextValue(root, "input_dir");
	ValueArray events = root.Get("events", ValueArray());
	for(int i = 0; i < events.GetCount(); i++) {
		ValueMap src = events[i];
		RawVideoEvent& event = out.Add();
		event.frame_index = IntValue(src, "frame_index");
		event.table_id = IntValue(src, "table_id");
		event.type = TextValue(src, "type");
		event.reason = TextValue(src, "reason");
		event.confidence = DoubleValue(src, "confidence");
		ValueMap evidence = src.Get("evidence", ValueMap());
		event.semantic = TextValue(evidence, "semantic");
		event.change_blocks = IntValue(evidence, "change_blocks");
	}
	Sort(out, [](const RawVideoEvent& a, const RawVideoEvent& b) {
		if(a.table_id != b.table_id)
			return a.table_id < b.table_id;
		if(a.frame_index != b.frame_index)
			return a.frame_index < b.frame_index;
		return a.type < b.type;
	});
	return out;
}

static Vector<CorrelatedVideoEvent> Correlate(const Vector<RawVideoEvent>& events, int frame_gap)
{
	Vector<CorrelatedVideoEvent> groups;
	for(const RawVideoEvent& event : events) {
		int target = -1;
		for(int i = groups.GetCount() - 1; i >= 0; i--) {
			if(SameTransition(groups[i], event, frame_gap)) {
				target = i;
				break;
			}
		}
		if(target < 0) {
			CorrelatedVideoEvent& group = groups.Add();
			group.id = groups.GetCount() - 1;
			group.start_frame = event.frame_index;
			group.end_frame = event.frame_index;
			group.table_id = event.table_id;
			group.kind = EventKind(event.type);
			target = groups.GetCount() - 1;
		}
		CorrelatedVideoEvent& group = groups[target];
		if(IsPrimaryEvent(event.type))
			group.kind = "table_transition";
		group.start_frame = min(group.start_frame, event.frame_index);
		group.end_frame = max(group.end_frame, event.frame_index);
		group.raw_event_count++;
		group.change_blocks += event.change_blocks;
		group.confidence = max(group.confidence, event.confidence);
		AddUnique(group.types, event.type);
		AddUnique(group.semantics, event.semantic);
	}
	return groups;
}

static void AppendStringArray(String& json, const char *key, const Vector<String>& values)
{
	json << "\"" << key << "\": [";
	for(int i = 0; i < values.GetCount(); i++) {
		if(i)
			json << ", ";
		json << "\"" << JsonString(values[i]) << "\"";
	}
	json << "]";
}

static bool SaveCorrelated(const EventCorrelatorOptions& opt,
                           const Vector<CorrelatedVideoEvent>& groups,
                           int raw_count, int frame_count, const String& input_dir)
{
	String json;
	json << "{\n";
	json << "  \"input_dir\": \"" << JsonString(input_dir) << "\",\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"frame_count\": " << frame_count << ",\n";
	json << "  \"raw_event_count\": " << raw_count << ",\n";
	json << "  \"correlated_event_count\": " << groups.GetCount() << ",\n";
	json << "  \"events\": [\n";
	for(int i = 0; i < groups.GetCount(); i++) {
		const CorrelatedVideoEvent& group = groups[i];
		if(i)
			json << ",\n";
		json << "    {\"id\": " << group.id
		     << ", \"kind\": \"" << JsonString(group.kind)
		     << "\", \"table_id\": " << group.table_id
		     << ", \"start_frame\": " << group.start_frame
		     << ", \"end_frame\": " << group.end_frame
		     << ", \"raw_event_count\": " << group.raw_event_count
		     << ", \"change_blocks\": " << group.change_blocks
		     << ", \"confidence\": " << group.confidence << ", ";
		AppendStringArray(json, "types", group.types);
		json << ", ";
		AppendStringArray(json, "semantics", group.semantics);
		json << "}";
	}
	json << "\n  ]\n";
	json << "}\n";
	if(!SaveFile(opt.out_path, json))
		return false;
	Cout() << "correlated_events_json=" << opt.out_path << "\n";
	Cout() << "raw_events=" << raw_count << " correlated_events=" << groups.GetCount() << "\n";
	return true;
}

static bool RunCorrelator(const EventCorrelatorOptions& opt)
{
	int frame_count = 0;
	String input_dir;
	Vector<RawVideoEvent> raw = LoadEvents(opt, frame_count, input_dir);
	Vector<CorrelatedVideoEvent> groups = Correlate(raw, opt.frame_gap);
	return SaveCorrelated(opt, groups, raw.GetCount(), frame_count, input_dir);
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	EventCorrelatorOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunCorrelator(opt))
		SetExitCode(1);
}
