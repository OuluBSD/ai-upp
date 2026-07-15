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

static void AppendNullableString(String& json, const String& value)
{
	if(value.IsEmpty())
		json << "null";
	else
		json << "\"" << JsonString(value) << "\"";
}

static bool HasDigit(const String& text)
{
	for(int i = 0; i < text.GetCount(); i++)
		if(IsDigit(text[i]))
			return true;
	return false;
}

static bool IsActionText(const String& text)
{
	String lower = ToLower(text);
	return lower.Find("fold") >= 0 || lower.Find("call") >= 0 ||
	       lower.Find("check") >= 0 || lower.Find("raise") >= 0 ||
	       lower.Find("bet") >= 0 || lower.Find("all") >= 0;
}

static Vector<String> CleanOcrLines(String text)
{
	text.Replace("\r", "\n");
	Vector<String> lines = Split(text, '\n');
	Vector<String> out;
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.GetCount() >= 2)
			out << line;
	}
	return out;
}

static String ExtractPotAmountText(const String& text)
{
	String lower = ToLower(text);
	int pos = lower.Find("pot");
	if(pos < 0)
		return String();
	String tail = text.Mid(pos);
	int colon = tail.Find(":");
	if(colon >= 0)
		tail = tail.Mid(colon + 1);
	tail = TrimBoth(tail);
	Vector<String> parts = Split(tail, ' ');
	String out;
	for(String part : parts) {
		part = TrimBoth(part);
		if(part.IsEmpty())
			continue;
		if(!out.IsEmpty())
			out << " ";
		out << part;
		if(HasDigit(part) && out.GetCount() > 0)
			break;
	}
	return HasDigit(out) ? out : String();
}

static String NormalizeActionText(const String& text)
{
	String lower = ToLower(text);
	if(lower.Find("fold") >= 0)
		return "fold";
	if(lower.Find("check") >= 0)
		return "check";
	if(lower.Find("call") >= 0)
		return "call";
	if(lower.Find("raise") >= 0)
		return "raise";
	if(lower.Find("bet") >= 0)
		return "bet";
	if(lower.Find("all") >= 0)
		return "all_in";
	return String();
}

static String ExtractStackText(const Vector<String>& lines)
{
	String decimal_stack;
	String fallback_stack;
	for(int i = lines.GetCount() - 1; i >= 0; i--) {
		String lower = ToLower(lines[i]);
		if(lower.Find("bb") >= 0 && HasDigit(lines[i])) {
			if(fallback_stack.IsEmpty())
				fallback_stack = lines[i];
			if((lines[i].Find(".") >= 0 || lines[i].Find(",") >= 0) && decimal_stack.IsEmpty())
				decimal_stack = lines[i];
		}
	}
	if(!decimal_stack.IsEmpty())
		return decimal_stack;
	if(!fallback_stack.IsEmpty())
		return fallback_stack;
	for(int i = lines.GetCount() - 1; i >= 0; i--) {
		if(HasDigit(lines[i]) && !IsActionText(lines[i]))
			return lines[i];
	}
	return String();
}

static bool IsNoiseNameLine(const String& line)
{
	String lower = ToLower(line);
	if(lower.Find("responsible") >= 0 || lower.Find("gaming") >= 0 ||
	   lower.Find("time") >= 0 || lower.Find(" sec") >= 0)
		return true;
	int alnum = 0;
	for(int i = 0; i < line.GetCount(); i++)
		if(IsAlNum(line[i]) || line[i] == '_' || line[i] == '-')
			alnum++;
	return alnum < 3;
}

static String CleanNameCandidate(String line)
{
	int at = line.Find("@");
	if(at > 0)
		line = line.Left(at);
	line = TrimBoth(line);
	while(!line.IsEmpty() && !IsAlNum(line[0]) && line[0] != '_' && line[0] != '(')
		line = line.Mid(1);
	return TrimBoth(line);
}

static String ExtractNameText(const Vector<String>& lines, const String& stack, const String& action)
{
	int stack_index = -1;
	for(int i = 0; i < lines.GetCount(); i++) {
		if(lines[i] == stack) {
			stack_index = i;
			break;
		}
	}
	int start = stack_index >= 0 ? stack_index - 1 : lines.GetCount() - 1;
	for(int i = start; i >= 0; i--) {
		String line = CleanNameCandidate(lines[i]);
		if(line.IsEmpty() || line == stack)
			continue;
		String normalized_action = NormalizeActionText(line);
		if(!action.IsEmpty() && normalized_action == action)
			continue;
		String lower = ToLower(line);
		if(lower.Find("pot") >= 0 || lower.Find("bb") >= 0)
			continue;
		if(HasDigit(line) && line.GetCount() <= 4)
			continue;
		if(IsNoiseNameLine(line))
			continue;
		return line;
	}
	return String();
}

static void ParseSeatText(SeatRegionState& seat, const String& text)
{
	seat.raw_text = text;
	Vector<String> lines = CleanOcrLines(text);
	for(String line : lines) {
		seat.action = NormalizeActionText(line);
		if(!seat.action.IsEmpty())
			break;
	}
	seat.stack_text = ExtractStackText(lines);
	seat.name = ExtractNameText(lines, seat.stack_text, seat.action);
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

static String TableCropPath(ValueMap table)
{
	return TextValue(table, "crop");
}

static Rect TopHalf(const Rect& r)
{
	return Rect(r.left, r.top, r.right, (r.top + r.bottom) / 2);
}

static Rect BottomHalf(const Rect& r)
{
	return Rect(r.left, (r.top + r.bottom) / 2, r.right, r.bottom);
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

static int PixelIndex(int x, int y, int width)
{
	return y * width + x;
}

static void AnalyzeComponents(const Vector<byte>& mask, int width, int height,
                              int& component_count, int& largest_pixels,
                              Rect& largest_bounds)
{
	component_count = 0;
	largest_pixels = 0;
	largest_bounds = Rect(0, 0, 0, 0);
	Vector<byte> seen;
	seen.SetCount(mask.GetCount(), 0);
	Vector<int> stack;
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int start = PixelIndex(x, y, width);
			if(!mask[start] || seen[start])
				continue;
			component_count++;
			int pixels = 0;
			Rect bounds(0, 0, 0, 0);
			stack.Clear();
			stack << start;
			seen[start] = 1;
			while(!stack.IsEmpty()) {
				int at = stack.Top();
				stack.Drop();
				int px = at % width;
				int py = at / width;
				Rect pixel = RectC(px, py, 1, 1);
				bounds = pixels == 0 ? pixel : bounds | pixel;
				pixels++;
				for(int dy = -1; dy <= 1; dy++) {
					for(int dx = -1; dx <= 1; dx++) {
						if(dx == 0 && dy == 0)
							continue;
						int nx = px + dx;
						int ny = py + dy;
						if(nx < 0 || ny < 0 || nx >= width || ny >= height)
							continue;
						int ni = PixelIndex(nx, ny, width);
						if(mask[ni] && !seen[ni]) {
							seen[ni] = 1;
							stack << ni;
						}
					}
				}
			}
			if(pixels > largest_pixels) {
				largest_pixels = pixels;
				largest_bounds = bounds;
			}
		}
	}
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
		int y0 = img.GetHeight() / 8;
		int y1 = img.GetHeight() * 7 / 8;
		int slot_width = x1 - x0;
		int sample_height = y1 - y0;
		BoardSlotState& slot_state = slots_out.Add();
		slot_state.index = slot;
		slot_state.rect = Rect(x0, 0, x1, img.GetHeight());
		int hit = 0;
		int total = 0;
		Rect hit_bounds(0, 0, 0, 0);
		Vector<byte> mask;
		mask.SetCount(slot_width * sample_height, 0);
		for(int y = y0; y < y1; y++) {
			const RGBA *row = img[y];
			for(int x = x0 + 2; x < x1 - 2; x++) {
				total++;
				if(IsCardLikePixel(row[x])) {
					hit++;
					Point p(x - x0, y);
					Rect pixel = RectC(p.x, p.y, 1, 1);
					hit_bounds = hit == 1 ? pixel : hit_bounds | pixel;
					mask[PixelIndex(p.x, y - y0, slot_width)] = 1;
				}
			}
		}
		hits << hit;
		slot_state.cardlike_pixels = hit;
		slot_state.sampled_pixels = total;
		slot_state.cardlike_ratio = total > 0 ? (double)hit / total : 0;
		slot_state.cardlike_bounds = hit_bounds;
		AnalyzeComponents(mask, slot_width, sample_height,
		                  slot_state.component_count,
		                  slot_state.largest_component_pixels,
		                  slot_state.largest_component_bounds);
		slot_state.largest_component_bounds.Offset(0, y0);
		slot_state.largest_component_ratio = total > 0 ?
		                                     (double)slot_state.largest_component_pixels / total : 0;
		bool enough_pixels = total > 0 && hit * 100 >= total * 12;
		bool enough_component = slot_state.largest_component_pixels * 100 >= total * 12;
		bool enough_shape = slot_state.largest_component_bounds.Height() >= img.GetHeight() / 2 &&
		                    slot_state.largest_component_bounds.Width() >= slot_width / 3;
		slot_state.present = enough_pixels && enough_component && enough_shape;
		slot_state.confidence = slot_state.present ? min(0.90, 0.60 + slot_state.cardlike_ratio) :
		                        (enough_pixels ? 0.45 : 0.25);
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
	       semantic == "left_seats" || semantic == "right_seats" ||
	       semantic == "left_top_seat" || semantic == "left_bottom_seat" ||
	       semantic == "right_top_seat" || semantic == "right_bottom_seat";
}

static void AddSeatRegion(ExtractedTableState& state, const char *semantic,
                          int index, const String& role,
                          const Rect& rect, const String& crop_path)
{
	SeatRegionState& seat = state.seats.Add();
	seat.index = index;
	seat.semantic = semantic;
	seat.rect = rect;
	seat.crop_path = crop_path;
	seat.role = role;
	seat.confidence = seat.rect.IsEmpty() || seat.crop_path.IsEmpty() ? 0.20 : 0.60;
}

static String SaveSeatCrop(const Image& table_crop, const Rect& rect, const String& out_dir,
                           int index)
{
	if(table_crop.IsEmpty() || rect.IsEmpty() || out_dir.IsEmpty())
		return String();
	Rect bounds = RectC(0, 0, table_crop.GetWidth(), table_crop.GetHeight());
	Rect clipped = rect & bounds;
	if(clipped.IsEmpty())
		return String();
	RealizeDirectory(out_dir);
	String path = AppendFileName(out_dir, Format("seat_%02d.jpg", index));
	JPGEncoder().Quality(95).SaveFile(path, CropImage(table_crop, clipped));
	return path;
}

static void AddObserverSeats(ExtractedTableState& state, ValueMap table,
                             const String& seat_dir)
{
	String table_crop_path = TableCropPath(table);
	Image table_crop;
	if(!table_crop_path.IsEmpty() && FileExists(table_crop_path))
		table_crop = StreamRaster::LoadFileAny(table_crop_path);

	Rect top = SemanticRect(table, "top_seat");
	Rect left_top = SemanticRect(table, "left_top_seat");
	Rect left_bottom = SemanticRect(table, "left_bottom_seat");
	Rect right_top = SemanticRect(table, "right_top_seat");
	Rect right_bottom = SemanticRect(table, "right_bottom_seat");
	Rect right = SemanticRect(table, "right_seats");
	Rect bottom = SemanticRect(table, "bottom_seat");
	Rect left = SemanticRect(table, "left_seats");
	if(left_top.IsEmpty())
		left_top = TopHalf(left);
	if(left_bottom.IsEmpty())
		left_bottom = BottomHalf(left);
	if(right_top.IsEmpty())
		right_top = TopHalf(right);
	if(right_bottom.IsEmpty())
		right_bottom = BottomHalf(right);

	AddSeatRegion(state, "top_seat", 0, "top",
	              top, SaveSeatCrop(table_crop, top, seat_dir, 0));
	AddSeatRegion(state, "right_top_seat", 1, "right_top",
	              right_top, SaveSeatCrop(table_crop, right_top, seat_dir, 1));
	AddSeatRegion(state, "right_bottom_seat", 2, "right_bottom",
	              right_bottom, SaveSeatCrop(table_crop, right_bottom, seat_dir, 2));
	AddSeatRegion(state, "bottom_seat", 3, "bottom",
	              bottom, SaveSeatCrop(table_crop, bottom, seat_dir, 3));
	AddSeatRegion(state, "left_bottom_seat", 4, "left_bottom",
	              left_bottom, SaveSeatCrop(table_crop, left_bottom, seat_dir, 4));
	AddSeatRegion(state, "left_top_seat", 5, "left_top",
	              left_top, SaveSeatCrop(table_crop, left_top, seat_dir, 5));
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
		state.pot_amount_text = ExtractPotAmountText(state.pot_text);
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
		else if(semantic == "pot_label" && state.pot_text.IsEmpty()) {
			state.pot_text = text;
			state.pot_amount_text = ExtractPotAmountText(state.pot_text);
		}
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

static void ApplyStructuredOcr(ExtractedTableState& state)
{
	if(state.pot_amount_text.IsEmpty())
		state.pot_amount_text = ExtractPotAmountText(state.pot_text);
	for(const OcrTextKey& text : state.seat_texts) {
		for(SeatRegionState& seat : state.seats) {
			if(seat.semantic == text.semantic) {
				ParseSeatText(seat, text.text);
				break;
			}
		}
	}
}

static void LoadTracking(const TableStateOptions& opt, Vector<ExtractedTableState>& states)
{
	ValueMap tracking = LoadJsonMap(opt.tracking_summary_json);
	String mode = VsmNormalizeTableMode(TextValue(tracking, "table_mode"));
	if(mode == "unknown")
		mode = opt.table_mode;
	ValueArray frames = ArrayValue(tracking, "frames");
	for(int fi = 0; fi < frames.GetCount(); fi++) {
		ValueMap frame = frames[fi];
		int frame_index = IntValue(frame, "index", fi);
		ValueArray tables = ArrayValue(frame, "tables");
		for(int i = 0; i < tables.GetCount(); i++) {
			ValueMap table = tables[i];
			int table_id = IntValue(table, "id");
			ExtractedTableState& state = GetState(states, frame_index, table_id, mode);
			state.board_crop_path = SemanticPath(table, "board_cards");
			String slot_dir = opt.tracker_dir.IsEmpty() ? String() :
			                  AppendFileName(AppendFileName(opt.tracker_dir, "table_state_slots"),
			                                 Format("frame_%06d_table_%d", frame_index, table_id));
			String seat_dir = opt.tracker_dir.IsEmpty() ? String() :
			                  AppendFileName(AppendFileName(opt.tracker_dir, "table_state_seats"),
			                                 Format("frame_%06d_table_%d", frame_index, table_id));
			state.board_card_count = EstimateBoardCardCount(state.board_crop_path,
			                                                 slot_dir,
			                                                 state.board_slots,
			                                                 state.board_confidence,
			                                                 state.board_reason);
			AddObserverSeats(state, table, seat_dir);
			state.reasons << "board:" + state.board_reason;
		}
	}
}

static void Finalize(Vector<ExtractedTableState>& states)
{
	for(ExtractedTableState& state : states) {
		ApplyStructuredOcr(state);
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
		     << "\", \"pot_amount_text\": ";
		AppendNullableString(json, state.pot_amount_text);
		json << ", \"board_crop_path\": \"" << JsonString(state.board_crop_path)
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
			     << ", \"sampled_pixels\": " << slot.sampled_pixels
			     << ", \"cardlike_ratio\": " << Format("%.4f", slot.cardlike_ratio)
			     << ", \"cardlike_bounds\": ";
			AppendRect(json, slot.cardlike_bounds);
			json << ", \"component_count\": " << slot.component_count
			     << ", \"largest_component_pixels\": " << slot.largest_component_pixels
			     << ", \"largest_component_ratio\": " << Format("%.4f", slot.largest_component_ratio)
			     << ", \"largest_component_bounds\": ";
			AppendRect(json, slot.largest_component_bounds);
			json
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
			     << ", \"raw_text\": ";
			AppendNullableString(json, seat.raw_text);
			json << ", \"name\": ";
			AppendNullableString(json, seat.name);
			json << ", \"stack_text\": ";
			AppendNullableString(json, seat.stack_text);
			json << ", \"action\": ";
			AppendNullableString(json, seat.action);
			json << ", \"stack\": null"
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
		String slot_line;
		for(const BoardSlotState& slot : state.board_slots) {
			if(!slot_line.IsEmpty())
				slot_line << ",";
			slot_line << slot.index << ":" << (slot.present ? "1" : "0")
			          << "/" << Format("%.2f", slot.cardlike_ratio);
		}
		Cout() << "table_state frame=" << state.frame_index
		       << " table=" << state.table_id
		       << " usable=" << (state.usable ? "true" : "false")
		       << " board_cards=" << state.board_card_count
		       << " board_slots=" << state.board_slots.GetCount()
		       << " slot_ratio=" << slot_line
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
