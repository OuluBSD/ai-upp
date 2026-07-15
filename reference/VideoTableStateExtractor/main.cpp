#include "VideoTableStateExtractor.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoTableStateExtractor\n\n"
	       << "Usage: VideoTableStateExtractor --tracker-dir <dir> [options]\n\n"
	       << "Options:\n"
	       << "  --tracker-dir <dir>           VideoWindowTracker output directory\n"
	       << "  --tracking-summary-json <f>   Override tracking_summary.json path\n"
	       << "  --ocr-json <file>             Override ocr_probe.json path\n"
	       << "  --table-quality-json <file>   Override table_quality.json path\n"
	       << "  --out <file>                  Output JSON path (default <dir>/table_state.json)\n"
	       << "  --table-mode <mode>           unknown, hero, observer/no-hero\n"
	       << "  --help, -h                    Show help\n";
}

static TableStateOptions ParseOptions(const Vector<String>& args)
{
	TableStateOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--tracker-dir" && i + 1 < args.GetCount())
			opt.tracker_dir = args[++i];
		else if(args[i] == "--tracking-summary-json" && i + 1 < args.GetCount())
			opt.tracking_summary_json = args[++i];
		else if(args[i] == "--ocr-json" && i + 1 < args.GetCount())
			opt.ocr_json = args[++i];
		else if(args[i] == "--table-quality-json" && i + 1 < args.GetCount())
			opt.table_quality_json = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--table-mode" && i + 1 < args.GetCount())
			opt.table_mode = VsmNormalizeTableMode(args[++i]);
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(!opt.tracker_dir.IsEmpty()) {
		if(opt.tracking_summary_json.IsEmpty())
			opt.tracking_summary_json = AppendFileName(opt.tracker_dir, "tracking_summary.json");
		if(opt.ocr_json.IsEmpty())
			opt.ocr_json = AppendFileName(opt.tracker_dir, "ocr_probe.json");
		if(opt.table_quality_json.IsEmpty())
			opt.table_quality_json = AppendFileName(opt.tracker_dir, "table_quality.json");
		if(opt.out_path.IsEmpty())
			opt.out_path = AppendFileName(opt.tracker_dir, "table_state.json");
	}
	opt.table_mode = VsmNormalizeTableMode(opt.table_mode);
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

static void AppendRect(String& out, const Rect& r)
{
	out << "{\"x\": " << r.left << ", \"y\": " << r.top
	    << ", \"w\": " << r.Width() << ", \"h\": " << r.Height() << "}";
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

static ValueMap MapValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueMap());
	return IsValueMap(value) ? ValueMap(value) : ValueMap();
}

static Rect RectValue(ValueMap map, const char *key)
{
	ValueMap rect = MapValue(map, key);
	if(rect.IsEmpty())
		return Rect(0, 0, 0, 0);
	return RectC(IntValue(rect, "x"), IntValue(rect, "y"),
	             IntValue(rect, "w"), IntValue(rect, "h"));
}

static ValueArray ArrayValue(ValueMap map, const char *key)
{
	Value value = map.Get(key, ValueArray());
	return IsValueArray(value) ? ValueArray(value) : ValueArray();
}

static String SemanticPath(ValueMap table, const char *semantic_name)
{
	ValueMap semantic = MapValue(table, "semantic");
	ValueMap item = MapValue(semantic, semantic_name);
	return TextValue(item, "path");
}

static Rect SemanticRect(ValueMap table, const char *semantic_name)
{
	ValueMap semantic = MapValue(table, "semantic");
	ValueMap item = MapValue(semantic, semantic_name);
	return RectValue(item, "rect");
}

static int FirstFrameIndex(ValueMap tracking)
{
	ValueArray frames = ArrayValue(tracking, "frames");
	if(frames.IsEmpty())
		return 0;
	ValueMap frame = frames[0];
	return IntValue(frame, "index");
}

static ValueArray FirstFrameTables(ValueMap tracking)
{
	ValueArray frames = ArrayValue(tracking, "frames");
	if(frames.IsEmpty())
		return ValueArray();
	ValueMap frame = frames[0];
	return ArrayValue(frame, "tables");
}

static bool IsCardLikePixel(const RGBA& p)
{
	int maxc = max(max((int)p.r, (int)p.g), (int)p.b);
	int minc = min(min((int)p.r, (int)p.g), (int)p.b);
	bool bright_neutral = p.r > 150 && p.g > 150 && p.b > 150;
	bool saturated = maxc - minc > 45;
	bool felt_green = p.g * 100 > p.r * 115 && p.g * 100 > p.b * 105;
	return !felt_green && (bright_neutral || saturated);
}

static Image CropImage(const Image& img, const Rect& r)
{
	ImageBuffer ib(r.Width(), r.Height());
	for(int y = 0; y < r.Height(); y++)
		memcpy(ib[y], img[r.top + y] + r.left, r.Width() * sizeof(RGBA));
	return ib;
}

static int EstimateBoardCardCount(const String& path, const String& slot_dir,
                                  Vector<BoardSlotState>& slots_out,
                                  double& confidence, String& reason)
{
	confidence = 0;
	if(path.IsEmpty() || !FileExists(path)) {
		reason = "missing_board_crop";
		return 0;
	}
	Image img = StreamRaster::LoadFileAny(path);
	if(img.IsEmpty()) {
		reason = "failed_to_load_board_crop";
		return 0;
	}
	int slots = 5;
	int count = 0;
	Vector<int> hits;
	for(int slot = 0; slot < slots; slot++) {
		int x0 = slot * img.GetWidth() / slots;
		int x1 = (slot + 1) * img.GetWidth() / slots;
		BoardSlotState& slot_state = slots_out.Add();
		slot_state.index = slot;
		slot_state.rect = Rect(x0, 0, x1, img.GetHeight());
		int hit = 0;
		int total = 0;
		for(int y = img.GetHeight() / 8; y < img.GetHeight() * 7 / 8; y++) {
			const RGBA *row = img[y];
			for(int x = x0 + 2; x < x1 - 2; x++) {
				total++;
				if(IsCardLikePixel(row[x]))
					hit++;
			}
		}
		hits << hit;
		slot_state.cardlike_pixels = hit;
		slot_state.present = total > 0 && hit * 100 >= total * 12;
		slot_state.confidence = slot_state.present ? 0.70 : 0.30;
		if(!slot_dir.IsEmpty()) {
			RealizeDirectory(slot_dir);
			slot_state.crop_path = AppendFileName(slot_dir, Format("slot_%02d.jpg", slot));
			JPGEncoder().Quality(95).SaveFile(slot_state.crop_path, CropImage(img, slot_state.rect));
		}
		if(slot_state.present)
			count++;
	}
	confidence = count > 0 ? min(0.95, 0.50 + 0.10 * count) : 0.25;
	reason = "slot_cardlike_pixels";
	for(int hit : hits)
		reason << " " << hit;
	return count;
}

static int FindState(Vector<ExtractedTableState>& states, int frame_index, int table_id)
{
	for(int i = 0; i < states.GetCount(); i++)
		if(states[i].frame_index == frame_index && states[i].table_id == table_id)
			return i;
	return -1;
}

static ExtractedTableState& GetState(Vector<ExtractedTableState>& states, int frame_index,
                                     int table_id, const String& mode)
{
	int index = FindState(states, frame_index, table_id);
	if(index >= 0)
		return states[index];
	ExtractedTableState& state = states.Add();
	state.frame_index = frame_index;
	state.table_id = table_id;
	state.table_mode = mode;
	return state;
}

static bool IsSeatSemantic(const String& semantic)
{
	return semantic == "top_seat" || semantic == "bottom_seat" ||
	       semantic == "left_seats" || semantic == "right_seats";
}

static void AddSeatRegion(ExtractedTableState& state, ValueMap table,
                          const char *semantic, int index, const String& role)
{
	SeatRegionState& seat = state.seats.Add();
	seat.index = index;
	seat.semantic = semantic;
	seat.rect = SemanticRect(table, semantic);
	seat.crop_path = SemanticPath(table, semantic);
	seat.role = role;
	seat.confidence = seat.rect.IsEmpty() || seat.crop_path.IsEmpty() ? 0.20 : 0.60;
}

static void LoadQuality(const TableStateOptions& opt, Vector<ExtractedTableState>& states)
{
	ValueMap root = LoadJsonMap(opt.table_quality_json);
	String mode = VsmNormalizeTableMode(TextValue(root, "table_mode"));
	if(mode == "unknown")
		mode = opt.table_mode;
	ValueArray tables = ArrayValue(root, "tables");
	for(int i = 0; i < tables.GetCount(); i++) {
		ValueMap table = tables[i];
		int frame_index = IntValue(table, "frame_index");
		int table_id = IntValue(table, "table_id");
		ExtractedTableState& state = GetState(states, frame_index, table_id, mode);
		state.usable = BoolValue(table, "usable");
		state.quality_reason = TextValue(table, "reason");
		state.title_text = TextValue(table, "title_text");
		state.pot_text = TextValue(table, "pot_text");
		state.reasons << "quality:" + state.quality_reason;
	}
}

static void LoadOcr(const TableStateOptions& opt, Vector<ExtractedTableState>& states)
{
	ValueMap root = LoadJsonMap(opt.ocr_json);
	ValueArray results = ArrayValue(root, "results");
	for(int i = 0; i < results.GetCount(); i++) {
		ValueMap result = results[i];
		int frame_index = IntValue(result, "frame_index");
		int table_id = IntValue(result, "table_id");
		String semantic = TextValue(result, "semantic");
		String text = TextValue(result, "text");
		ExtractedTableState& state = GetState(states, frame_index, table_id, opt.table_mode);
		if(semantic == "title" && state.title_text.IsEmpty())
			state.title_text = text;
		else if(semantic == "pot_label" && state.pot_text.IsEmpty())
			state.pot_text = text;
		else if(IsSeatSemantic(semantic)) {
			OcrTextKey& seat = state.seat_texts.Add();
			seat.frame_index = frame_index;
			seat.table_id = table_id;
			seat.semantic = semantic;
			seat.text = text;
			seat.path = TextValue(result, "path");
		}
	}
}

static void LoadTracking(const TableStateOptions& opt, Vector<ExtractedTableState>& states)
{
	ValueMap tracking = LoadJsonMap(opt.tracking_summary_json);
	String mode = VsmNormalizeTableMode(TextValue(tracking, "table_mode"));
	if(mode == "unknown")
		mode = opt.table_mode;
	int frame_index = FirstFrameIndex(tracking);
	ValueArray tables = FirstFrameTables(tracking);
	for(int i = 0; i < tables.GetCount(); i++) {
		ValueMap table = tables[i];
		int table_id = IntValue(table, "id");
		ExtractedTableState& state = GetState(states, frame_index, table_id, mode);
		state.board_crop_path = SemanticPath(table, "board_cards");
		String slot_dir = opt.tracker_dir.IsEmpty() ? String() :
		                  AppendFileName(AppendFileName(opt.tracker_dir, "table_state_slots"),
		                                 Format("frame_%06d_table_%d", frame_index, table_id));
		state.board_card_count = EstimateBoardCardCount(state.board_crop_path,
		                                                 slot_dir,
		                                                 state.board_slots,
		                                                 state.board_confidence,
		                                                 state.board_reason);
		AddSeatRegion(state, table, "top_seat", 0, "top");
		AddSeatRegion(state, table, "right_seats", 1, "right_group");
		AddSeatRegion(state, table, "bottom_seat", 2, "bottom");
		AddSeatRegion(state, table, "left_seats", 3, "left_group");
		state.reasons << "board:" + state.board_reason;
	}
}

static void Finalize(Vector<ExtractedTableState>& states)
{
	for(ExtractedTableState& state : states) {
		int score = 0;
		if(state.usable)
			score += 35;
		if(!state.title_text.IsEmpty())
			score += 15;
		if(!state.pot_text.IsEmpty())
			score += 15;
		if(!state.board_crop_path.IsEmpty())
			score += 15;
		if(!state.seat_texts.IsEmpty())
			score += 15;
		score += (int)(state.board_confidence * 5);
		state.confidence = min(0.99, score / 100.0);
		if(VsmObserverNoHero(state.table_mode))
			state.reasons << "observer_nohero:hero_cards_not_expected";
	}
	Sort(states, [](const ExtractedTableState& a, const ExtractedTableState& b) {
		if(a.frame_index != b.frame_index)
			return a.frame_index < b.frame_index;
		return a.table_id < b.table_id;
	});
}

static void AppendStringArray(String& json, const Vector<String>& values)
{
	json << "[";
	for(int i = 0; i < values.GetCount(); i++) {
		if(i)
			json << ", ";
		json << "\"" << JsonString(values[i]) << "\"";
	}
	json << "]";
}

static bool SaveState(const TableStateOptions& opt, const Vector<ExtractedTableState>& states)
{
	String json;
	json << "{\n";
	json << "  \"tool\": \"VideoTableStateExtractor\",\n";
	json << "  \"schema\": \"observer_table_state_v1\",\n";
	json << "  \"tracker_dir\": \"" << JsonString(opt.tracker_dir) << "\",\n";
	json << "  \"table_mode\": \"" << JsonString(opt.table_mode) << "\",\n";
	json << "  \"hero_cards_expected\": " << (VsmHeroCardsExpected(opt.table_mode) ? "true" : "false") << ",\n";
	json << "  \"observer_nohero\": " << (VsmObserverNoHero(opt.table_mode) ? "true" : "false") << ",\n";
	json << "  \"explicit_unknowns\": true,\n";
	json << "  \"state_count\": " << states.GetCount() << ",\n";
	json << "  \"tables\": [\n";
	for(int i = 0; i < states.GetCount(); i++) {
		const ExtractedTableState& state = states[i];
		if(i)
			json << ",\n";
		json << "    {\"frame_index\": " << state.frame_index
		     << ", \"table_id\": " << state.table_id
		     << ", \"table_mode\": \"" << JsonString(state.table_mode)
		     << "\", \"usable\": " << (state.usable ? "true" : "false")
		     << ", \"confidence\": " << Format("%.2f", state.confidence)
		     << ", \"quality_reason\": \"" << JsonString(state.quality_reason)
		     << "\", \"title_text\": \"" << JsonString(state.title_text)
		     << "\", \"pot_text\": \"" << JsonString(state.pot_text)
		     << "\", \"board_crop_path\": \"" << JsonString(state.board_crop_path)
		     << "\", \"board_card_count\": " << state.board_card_count
		     << ", \"board_confidence\": " << Format("%.2f", state.board_confidence)
		     << ", \"board_reason\": \"" << JsonString(state.board_reason)
		     << "\", \"board_slots\": [";
		for(int j = 0; j < state.board_slots.GetCount(); j++) {
			const BoardSlotState& slot = state.board_slots[j];
			if(j)
				json << ", ";
			json << "{\"index\": " << slot.index
			     << ", \"rect\": ";
			AppendRect(json, slot.rect);
			json << ", \"present\": " << (slot.present ? "true" : "false")
			     << ", \"confidence\": " << Format("%.2f", slot.confidence)
			     << ", \"cardlike_pixels\": " << slot.cardlike_pixels
			     << ", \"rank\": null, \"suit\": null"
			     << ", \"crop_path\": \"" << JsonString(slot.crop_path) << "\"}";
		}
		json << "], \"seats\": [";
		for(int j = 0; j < state.seats.GetCount(); j++) {
			const SeatRegionState& seat = state.seats[j];
			if(j)
				json << ", ";
			json << "{\"index\": " << seat.index
			     << ", \"semantic\": \"" << JsonString(seat.semantic)
			     << "\", \"role\": \"" << JsonString(seat.role)
			     << "\", \"rect\": ";
			AppendRect(json, seat.rect);
			json << ", \"crop_path\": \"" << JsonString(seat.crop_path)
			     << "\", \"confidence\": " << Format("%.2f", seat.confidence)
			     << ", \"name\": null, \"stack\": null, \"action\": null"
			     << ", \"visible_hole_cards\": []}";
		}
		json << "], \"seat_texts\": [";
		for(int j = 0; j < state.seat_texts.GetCount(); j++) {
			const OcrTextKey& seat = state.seat_texts[j];
			if(j)
				json << ", ";
			json << "{\"semantic\": \"" << JsonString(seat.semantic)
			     << "\", \"text\": \"" << JsonString(seat.text)
			     << "\", \"path\": \"" << JsonString(seat.path) << "\"}";
		}
		json << "], \"reasons\": ";
		AppendStringArray(json, state.reasons);
		json << "}";
	}
	json << "\n  ]\n";
	json << "}\n";
	return SaveFile(opt.out_path, json);
}

static bool RunExtractor(const TableStateOptions& opt)
{
	Vector<ExtractedTableState> states;
	LoadQuality(opt, states);
	LoadOcr(opt, states);
	LoadTracking(opt, states);
	Finalize(states);
	if(!SaveState(opt, states))
		return false;
	Cout() << "table_state_json=" << opt.out_path << "\n";
	Cout() << "table_state_count=" << states.GetCount()
	       << " table_mode=" << opt.table_mode << "\n";
	for(const ExtractedTableState& state : states) {
		Cout() << "table_state frame=" << state.frame_index
		       << " table=" << state.table_id
		       << " usable=" << (state.usable ? "true" : "false")
		       << " board_cards=" << state.board_card_count
		       << " board_slots=" << state.board_slots.GetCount()
		       << " seats=" << state.seats.GetCount()
		       << " observer_nohero=" << (VsmObserverNoHero(state.table_mode) ? "true" : "false")
		       << " confidence=" << Format("%.2f", state.confidence) << "\n";
	}
	return true;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	TableStateOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.tracker_dir.IsEmpty()) {
		PrintHelp();
		if(opt.tracker_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	if(!RunExtractor(opt))
		SetExitCode(1);
}
