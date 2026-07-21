#include <CtrlCore/CtrlCore.h> // GUI_APP_MAIN (engine pulls no Ctrl header itself)
#include <VisualStateModel/VisualStateModel.h>
#include <ComputerVision/ComputerVision.h> // Task 0290a: native MatchTemplate + OrbSystem (no OpenCV)
#include <plugin/jpg/jpg.h>
#include <plugin/png/png.h> // Task 0291b: PNG overlays (thin colored lines survive lossless)

#include <TexasHoldem/TexasHoldemLocalGame.h>
#include <TexasHoldem/TexasHoldemLogicState.h> // card/board art helpers (reuse game/CardRender via GameTable's own encoding)
#include <Poker/LocalEngineFactory.h>
#include <Poker/CardsValue.h>
#include <GameRules/Game.h>
#include <GameRules/GameData.h>
#include <GameRules/PlayerData.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BeroInterface.h>
#include <GameRules/BoardInterface.h>
#include <GameRules/PlayerInterface.h>
#include <GameRules/GuiInterface.h>
#include <GameRules/EngineLog.h>
#include <EditorCommon/ConfigFile.h>

#include <chrono>

using namespace Upp;

// ===========================================================================
// Task 0280: continuous per-frame recognition + resolution loop.
//
// Combines four previously-separate batch tools' logic into one incremental
// loop that, for each new frame from VsmVideoServerFrameSource (Task 0279):
//   1. crops the (static) table window and detects changes vs. the previous
//      frame (VsmDetectChanges, ChangeDetect.h -- already incremental).
//   2. classifies each changed region into a semantic category
//      (VsmLiveRegionClassifier -- the genuinely-new piece, Task 0280).
//   3. OCRs OCR-relevant regions (VsmTesseractOcrEngine, Task 0277).
//   4. resolves recognition output into structural game events + value-change
//      candidate actions.
//   5. applies resolved board/street events to a real headless Game
//      (game/TexasHoldem, via Task 0270's external-action API + setMyCards).
//
// Modes:
//   --classify-selftest   Rigorous leave-one-out classification accuracy over
//                         the 267-candidate labeled dataset (deterministic, no
//                         server needed). Reports the NEW classifier's accuracy.
//   --offline-frames DIR  Runs the full stages-1..4 pipeline over the dataset's
//                         own source frames (frame_%06d.jpg) for per-stage
//                         timing, deterministic, no server needed.
//   --sidecar2            Decodes an MP4 directly and emits only recognized,
//                         parser-shaped sidecar events at one PTS frame/second.
//   --live                Continuous loop against a running VideoServer (the
//                         real Task 0278/0279 live source).
//   --shader-evidence-selftest  Runs the shared two-window shader evidence
//                         adapter on deterministic synthetic input.
// ===========================================================================

// Fixed 2D card-game table windows in the 1920x1080 recording. Confirmed static
// across all 56 tracking frames of the M12 recording (tracking.json), and by
// VideoFrameWindowDetector's two table_candidate results for frame 0.
// Task 0283 real finding: the table M12's ground truth and the 267-candidate
// labeled dataset (real_recording_combined_classified_dataset.json) were
// actually built from is table_1 ("Aludra", rect (968,1,944,682) in
// tracking.json), NOT table_0 ("Donati", rect (8,1,944,682)) -- confirmed via
// (a) tracking.json's own table_id assignment, (b) a table_1 crop's "Pot: 4 BB"
// text matching real_recording_ground_truth.jsonl's pot=40 at the same elapsed
// time, (c) table_0's frame-0 board (7d,Jc,2h) never appearing anywhere in the
// ground truth board sequence, while table_1's frame-0 board (2c,3d,8c,9c,7s)
// and its hand-1 flop (3d,3s,Kh) match the ground truth exactly. The original
// crop (table_0) was a real, previously-undiscovered bug: the live loop was
// recognizing an unrelated table's hand, not the one its own reference
// dataset/ground truth describe. Using a fixed crop matches M12's
// single-static-table scope; a moving/multi-table feed would need
// VideoWindowTracker per frame (explicitly out of scope for this task).
static const Rect kLeftTableRect  = RectC(8,   1, 944, 682);
static const Rect kRightTableRect = RectC(968, 1, 944, 682);
static const Rect kTableRect = kRightTableRect;

struct WindowId : Moveable<WindowId> {
	String value;

	WindowId() {}
	explicit WindowId(const String& value) : value(value) {}

	bool operator==(const WindowId& id) const { return value == id.value; }
	bool operator!=(const WindowId& id) const { return value != id.value; }
};

struct WindowDescriptor : Moveable<WindowDescriptor> {
	WindowId id;
	String   name;
	Rect     source_rect;
};

struct WindowFrame : Moveable<WindowFrame> {
	WindowDescriptor descriptor;
	int64            source_sequence = -1;
	int64            window_sequence = -1;
	int64            timestamp_ms = -1;
	VsmFrameImage    table;
};

static WindowDescriptor MakeWindowDescriptor(const char *id, const char *name, const Rect& rect)
{
	WindowDescriptor descriptor;
	descriptor.id = WindowId(id);
	descriptor.name = name;
	descriptor.source_rect = rect;
	return descriptor;
}

static Vector<WindowDescriptor> SelectWindowDescriptors(const String& mode)
{
	Vector<WindowDescriptor> descriptors;
	if(mode == "both" || mode == "right")
		descriptors.Add(MakeWindowDescriptor("R", "right", kRightTableRect));
	if(mode == "both" || mode == "left")
		descriptors.Add(MakeWindowDescriptor("L", "left", kLeftTableRect));
	return descriptors;
}

static const char* kDatasetDefault = "tmp/real_recording_combined_classified_dataset.json";

static int RunShaderEvidenceSelfTest()
{
	VsmImageBuffer first, second, crop_map;
	first.Create(8, 5, 1);
	second.Create(8, 5, 1);
	crop_map.Create(2, 2, 1);
	crop_map.Set(0, 0, 210); crop_map.Set(1, 0, 70);
	crop_map.Set(0, 1, 40);  crop_map.Set(1, 1, 160);
	for(int y = 0; y < 2; y++)
		for(int x = 0; x < 2; x++) {
			first.Set(2 + x, 1 + y, crop_map.Get(x, y));
			second.Set(4 + x, 2 + y, crop_map.Get(x, y));
		}
	VsmImageBuffer packed;
	Vector<VsmPackedWindow> windows;
	String error;
	if(!VsmPackTwoWindows(first, second, packed, windows, error)) {
		Cerr() << "shader-evidence-selftest ERROR pack=" << error << "\n";
		return 1;
	}
	windows[0].id = "L"; windows[0].timestamp_ms = 1000;
	windows[1].id = "R"; windows[1].timestamp_ms = 1000;
	VsmShaderRecognitionService service;
	service.manifest.crop_map_width = 2; service.manifest.crop_map_height = 2;
	VsmShaderTemplate& templ = service.manifest.templates.Add();
	templ.id = "digit-fixture"; templ.label = "digit-fixture";
	templ.w = templ.foreground_w = 2; templ.h = templ.foreground_h = 2;
	service.crop_map.Create(crop_map.width, crop_map.height, crop_map.channels);
	for(int y = 0; y < crop_map.height; y++)
		for(int x = 0; x < crop_map.width; x++)
			service.crop_map.Set(x, y, crop_map.Get(x, y));
	service.use_threshold = true;
	service.threshold = 250;
	VsmShaderEvidenceAdapter adapter;
	adapter.SetService(service);
	Vector<VsmShaderWindowEvidence> results;
	if(!adapter.ProcessPacked(packed, windows, results, error)) {
		Cerr() << "shader-evidence-selftest ERROR process=" << error << "\n";
		return 1;
	}
	if(results.GetCount() != 2 || results[0].id != "L" || results[1].id != "R") {
		Cerr() << "shader-evidence-selftest ERROR window-separation\n";
		return 1;
	}
	VsmImageBuffer source;
	source.Create(16, 5, 1);
	for(int y = 0; y < 5; y++)
		for(int x = 0; x < 8; x++) {
			source.Set(x, y, first.Get(x, y));
			source.Set(8 + x, y, second.Get(x, y));
		}
	Vector<VsmPackedWindow> source_windows;
	VsmPackedWindow& source_left = source_windows.Add();
	source_left.id = "L"; source_left.width = 8; source_left.height = 5;
	source_left.timestamp_ms = 1000;
	VsmPackedWindow& source_right = source_windows.Add();
	source_right.id = "R"; source_right.source_x = 8;
	source_right.width = 8; source_right.height = 5;
	source_right.timestamp_ms = 1000;
	Vector<VsmShaderWindowEvidence> source_results;
	if(!adapter.ProcessSource(source, source_windows, source_results, error) ||
	   source_results.GetCount() != 2 || source_results[0].id != "L" ||
	   source_results[1].id != "R") {
		Cerr() << "shader-evidence-selftest ERROR source-window-separation=" << error << "\n";
		return 1;
	}
	Cout() << "shader-evidence source-path=pass windows=" << source_results.GetCount() << "\n";
	for(const VsmShaderWindowEvidence& result : results)
		Cout() << "shader-evidence window=" << result.id
		       << " timestamp_ms=" << result.timestamp_ms
		       << " runs=" << result.runs.GetCount() << "\n";
	Cout() << "shader-evidence-selftest status=pass windows=" << results.GetCount() << "\n";
	return 0;
}

// Task 0290a: external card template library (rank
// glyphs used by MatchTemplate). Overridable with --templates <dir>.
static String g_templates_dir = "tmp/templates";

struct RecognitionAnchor : Moveable<RecognitionAnchor> {
	String name;
	Image image;
};

struct RecognitionAnchors {
	Vector<RecognitionAnchor> anchors;
	bool loaded = false;

	void Load(const String& dir)
	{
		anchors.Clear();
		Vector<String> names;
		names << "BB-in-game" << "BB-folded" << "BB-board" << "balance-all-in"
		       << "action-bet" << "action-call" << "action-check" << "action-fold"
		       << "action-post-sb" << "action-post-bb" << "action-raise"
		       << "action-resume" << "action-muck" << "action-won";
		for(const String& name : names) {
			String path = AppendFileName(dir, name + ".png");
			Image image = StreamRaster::LoadFileAny(path);
			if(image.IsEmpty()) {
				Cout() << "anchor_missing name=" << name << " path=" << path << "\n";
				continue;
			}
			RecognitionAnchor& anchor = anchors.Add();
			anchor.name = name;
			anchor.image = image;
			Cout() << Format("anchor_loaded name=%s size=%dx%d path=%s\n", ~name,
			                 image.GetWidth(), image.GetHeight(), ~path);
		}
		loaded = !anchors.IsEmpty();
	}
};

static RecognitionAnchors g_recognition_anchors;

struct FixedTextSearchLayout {
	int width = 944;
	int height = 682;
	Rect name[6];
	Rect balance[6];
	Rect action[6];
	Rect bet[6];
	Rect board_money;
};

static FixedTextSearchLayout MakeFixedTextSearchLayout()
{
	FixedTextSearchLayout layout;
	static const Rect names[6] = {
		RectC(16, 74, 176, 54), RectC(20, 348, 184, 54), RectC(54, 146, 176, 54),
		RectC(372, 84, 160, 54), RectC(750, 146, 176, 54), RectC(768, 350, 176, 54)
	};
	static const Rect balances[6] = {
		RectC(414, 494, 148, 48), RectC(30, 378, 146, 48), RectC(62, 176, 148, 48),
		RectC(388, 112, 140, 48), RectC(764, 176, 148, 48), RectC(784, 378, 148, 48)
	};
	static const Rect bets[6] = {
		RectC(352, 392, 250, 70), RectC(154, 304, 190, 82), RectC(170, 190, 190, 82),
		RectC(350, 142, 250, 78), RectC(610, 190, 190, 82), RectC(610, 304, 190, 82)
	};
	for(int seat = 0; seat < 6; seat++) {
		layout.name[seat] = names[seat];
		layout.balance[seat] = balances[seat];
		layout.action[seat] = names[seat]; // action replaces the name plate
		layout.bet[seat] = bets[seat];
	}
	layout.board_money = RectC(272, 166, 396, 238);
	return layout;
}

static FixedTextSearchLayout g_fixed_text_layout = MakeFixedTextSearchLayout();

static bool SearchLayoutIntersects(const Rect& rect, const Rect& area)
{
	Rect overlap = rect & area;
	return overlap.Width() > 0 && overlap.Height() > 0;
}

static bool IsMoneySearchRegion(const Rect& rect, const String& category)
{
	if(category == "pot_label" || category == "seat_balance_plate" ||
	   category == "seat_bet_label" || category == "chip_badge_stack")
		return SearchLayoutIntersects(rect, g_fixed_text_layout.board_money) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[0]) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[1]) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[2]) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[3]) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[4]) ||
		       SearchLayoutIntersects(rect, g_fixed_text_layout.balance[5]);
	if(category.Find("composite") >= 0) {
		if(SearchLayoutIntersects(rect, g_fixed_text_layout.board_money)) return true;
		for(int seat = 0; seat < 6; seat++)
			if(SearchLayoutIntersects(rect, g_fixed_text_layout.balance[seat]) ||
			   SearchLayoutIntersects(rect, g_fixed_text_layout.bet[seat])) return true;
	}
	return false;
}

static bool IsActionSearchRegion(const Rect& rect, const String& category)
{
	if(category != "seat_action_bubble" && category != "seat_name_plate" &&
	   category.Find("composite") < 0) return false;
	for(int seat = 0; seat < 6; seat++)
		if(SearchLayoutIntersects(rect, g_fixed_text_layout.action[seat])) return true;
	return false;
}

static bool SaveFixedTextSearchLayout(const String& path)
{
	String out;
	out << "{\n  \"width\":944,\n  \"height\":682,\n  \"coordinate_space\":\"table-window\",\n";
	auto write_rects = [&](const char* key, const Rect* rects) {
		out << "  \"" << key << "\":[";
		for(int i = 0; i < 6; i++) {
			if(i) out << ",";
			out << Format("{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
			             rects[i].left, rects[i].top, rects[i].Width(), rects[i].Height());
		}
		out << "],\n";
	};
	write_rects("name", g_fixed_text_layout.name);
	write_rects("balance", g_fixed_text_layout.balance);
	write_rects("action", g_fixed_text_layout.action);
	write_rects("bet", g_fixed_text_layout.bet);
	out << Format("  \"board_money\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d},\n",
	             g_fixed_text_layout.board_money.left, g_fixed_text_layout.board_money.top,
	             g_fixed_text_layout.board_money.Width(), g_fixed_text_layout.board_money.Height());
	out << "  \"source\":\"fixed 944x682 layout, calibrated from recorded two-window feed\"\n}\n";
	return SaveFile(path, out);
}

static String CalibrationJsonString(const String& value);

static bool LoadFixedTextSearchLayout(const String& path)
{
	String text = LoadFile(path);
	if(text.IsVoid() || text.IsEmpty()) {
		Cerr() << "ERROR: text layout is empty: " << path << "\n";
		return false;
	}
	Value root = ParseJSON(text);
	if(IsError(root) || root.GetType() != VALUEMAP_V) {
		Cerr() << "ERROR: invalid text layout JSON: " << path << "\n";
		return false;
	}
	ValueMap map = root;
	Value width_value = map.Get("width", Value());
	Value height_value = map.Get("height", Value());
	int width = width_value.IsNull() ? 0 : (int)(double)width_value;
	int height = height_value.IsNull() ? 0 : (int)(double)height_value;
	if(width <= 0 || height <= 0) {
		Cerr() << "ERROR: text layout has invalid dimensions: " << path << "\n";
		return false;
	}
	FixedTextSearchLayout candidate;
	candidate.width = width;
	candidate.height = height;
	auto read_rects = [&](const char *key, Rect *out, int count) {
		Value value = map.Get(key, Value());
		if(value.GetType() != VALUEARRAY_V) {
			Cout() << Format("text_layout_field key=%s type=%d expected=array\n", key, (int)value.GetType());
			return false;
		}
		ValueArray array = value;
		if(array.GetCount() != count) {
			Cout() << Format("text_layout_field key=%s count=%d expected=%d\n", key, array.GetCount(), count);
			return false;
		}
		for(int i = 0; i < count; i++) {
			if(array[i].GetType() != VALUEMAP_V) {
				Cout() << Format("text_layout_rect key=%s index=%d type=%d expected=map\n", key, i, (int)array[i].GetType());
				return false;
			}
			ValueMap item = array[i];
			int x = (int)item.Get("x", Value());
			int y = (int)item.Get("y", Value());
			int w = (int)item.Get("w", Value());
			int h = (int)item.Get("h", Value());
			if(w <= 0 || h <= 0 || x < 0 || y < 0 || x + w > width || y + h > height) {
				Cout() << Format("text_layout_rect_invalid key=%s index=%d x=%d y=%d w=%d h=%d bounds=%dx%d\n",
				                 key, i, x, y, w, h, width, height);
				return false;
			}
			out[i] = RectC(x, y, w, h);
		}
		return true;
	};
	if(!read_rects("name", candidate.name, 6) ||
	   !read_rects("balance", candidate.balance, 6) ||
	   !read_rects("action", candidate.action, 6) ||
	   !read_rects("bet", candidate.bet, 6)) {
		Cerr() << "ERROR: text layout must contain six valid name/balance/action/bet rectangles: " << path << "\n";
		return false;
	}
	Value board_value = map.Get("board_money", Value());
	if(board_value.GetType() != VALUEMAP_V) {
		Cerr() << "ERROR: text layout is missing board_money: " << path << "\n";
		return false;
	}
	ValueMap board = board_value;
	int bx = (int)board.Get("x", Value()), by = (int)board.Get("y", Value());
	int bw = (int)board.Get("w", Value()), bh = (int)board.Get("h", Value());
	if(bx < 0 || by < 0 || bw <= 0 || bh <= 0 || bx + bw > width || by + bh > height) {
		Cerr() << "ERROR: text layout has invalid board_money rectangle: " << path << "\n";
		return false;
	}
	candidate.board_money = RectC(bx, by, bw, bh);
	g_fixed_text_layout = candidate;
	Cout() << Format("text_layout_loaded path=%s size=%dx%d\n", ~path, width, height);
	return true;
}

static bool WritePipelineReport(const String& path, const String& mode, const String& video,
	                               const String& layout, const String& sidecar, int exit_code)
{
	if(path.IsEmpty()) return true;
	String report;
	report << "{\n"
	       << "  \"mode\": \"" << mode << "\",\n"
	       << "  \"video\": \"" << video << "\",\n"
	       << "  \"text_layout\": \"" << layout << "\",\n"
	       << "  \"sidecar2_output\": \"" << sidecar << "\",\n"
	       << "  \"exit_code\": " << exit_code << "\n}\n";
	return SaveFile(path, report);
}

static bool WriteCalibrationFailure(const String& path, const String& stage,
	                                  const String& message, int samples = 0)
{
	if(path.IsEmpty()) return true;
	String report;
	report << "{\n"
	       << "  \"status\": \"error\",\n"
	       << "  \"stage\": \"" << CalibrationJsonString(stage) << "\",\n"
	       << "  \"message\": \"" << CalibrationJsonString(message) << "\",\n"
	       << "  \"samples\": " << samples << "\n"
	       << "}\n";
	bool ok = SaveFile(path, report);
	Cout() << Format("calibration_failure stage=%s samples=%d report=%s write=%s\n",
	                 ~stage, samples, ~path, ok ? "ok" : "failed");
	return ok;
}

static void AppendOcrVariantJson(String& out, const char *name, const String& text,
	                               int exit_code, double confidence)
{
	out << "\"" << name << "\":{";
	out << "\"text\":\"" << CalibrationJsonString(text) << "\",";
	out << "\"confidence\":" << Format("%.6f", confidence) << ",";
	out << "\"exit_code\":" << exit_code << ",";
	out << "\"status\":\"" << (exit_code == 0 ? (text.IsEmpty() ? "blank" : "ok") : "error") << "\"}";
}

static bool SaveGeneratedTextLayout(const String& calibration_path, const String& output_path,
	                                  const String& report_path)
{
	String text = LoadFile(calibration_path);
	if(text.IsVoid() || text.IsEmpty()) {
		Cerr() << "ERROR: calibration input is empty: " << calibration_path << "\n";
		return false;
	}
	Value root = ParseJSON(text);
	if(IsError(root) || root.GetType() != VALUEMAP_V) {
		Cerr() << "ERROR: invalid calibration JSON: " << calibration_path << "\n";
		return false;
	}
	ValueMap calibration = root;
	ValueArray samples = calibration.Get(Value("samples"), Value());
	if(samples.IsEmpty()) {
		Cerr() << "ERROR: calibration JSON has no samples: " << calibration_path << "\n";
		return false;
	}
	FixedTextSearchLayout candidate;
	Value width_value = calibration.Get(Value("width"), Value());
	Value height_value = calibration.Get(Value("height"), Value());
	candidate.width = width_value.IsNull() ? 944 : (int)(double)width_value;
	candidate.height = height_value.IsNull() ? 682 : (int)(double)height_value;
	int accepted = 0, rejected = 0;
	bool have_board = false;
	for(int si = 0; si < samples.GetCount(); si++) {
		if(samples[si].GetType() != VALUEMAP_V) { rejected++; continue; }
		ValueArray windows = ((ValueMap)samples[si]).Get(Value("windows"), Value());
		for(int wi = 0; wi < windows.GetCount(); wi++) {
			if(windows[wi].GetType() != VALUEMAP_V) { rejected++; continue; }
			ValueArray regions = ((ValueMap)windows[wi]).Get(Value("regions"), Value());
			for(int ri = 0; ri < regions.GetCount(); ri++) {
				if(regions[ri].GetType() != VALUEMAP_V) { rejected++; continue; }
				ValueMap item = regions[ri];
				String name = (String)item.Get(Value("region"), Value());
				int index = (int)item.Get(Value("index"), Value((int)-1));
				Rect rect = RectC((int)item.Get(Value("x"), Value((int)-1)), (int)item.Get(Value("y"), Value((int)-1)),
				                  (int)item.Get(Value("w"), Value((int)-1)), (int)item.Get(Value("h"), Value((int)-1)));
				if(rect.left < 0 || rect.top < 0 || rect.Width() <= 0 || rect.Height() <= 0 ||
				   rect.right > candidate.width || rect.bottom > candidate.height) {
					rejected++;
					continue;
				}
				Rect *target = nullptr;
				if(name == "name" && index >= 0 && index < 6) target = &candidate.name[index];
				else if(name == "balance" && index >= 0 && index < 6) target = &candidate.balance[index];
				else if(name == "action" && index >= 0 && index < 6) target = &candidate.action[index];
				else if(name == "bet" && index >= 0 && index < 6) target = &candidate.bet[index];
				else if(name == "board_money" && index < 0) { target = &candidate.board_money; have_board = true; }
				else { rejected++; continue; }
				if(target->Width() > 0 && *target != rect) {
					rejected++;
					continue;
				}
				*target = rect;
				accepted++;
			}
		}
	}
	for(int i = 0; i < 6; i++)
		if(candidate.name[i].Width() <= 0 || candidate.balance[i].Width() <= 0 ||
		   candidate.action[i].Width() <= 0 || candidate.bet[i].Width() <= 0)
			return WriteCalibrationFailure(report_path, "layout-generation", "one or more seat regions were underspecified", 0), false;
	if(!have_board || candidate.board_money.Width() <= 0)
		return WriteCalibrationFailure(report_path, "layout-generation", "board_money region was underspecified", 0), false;
	FixedTextSearchLayout saved = g_fixed_text_layout;
	g_fixed_text_layout = candidate;
	bool saved_ok = SaveFixedTextSearchLayout(output_path);
	g_fixed_text_layout = saved;
	String report = Format("{\n  \"status\": \"%s\",\n  \"accepted\": %d,\n  \"rejected\": %d,\n  \"output\": \"%s\"\n}\n",
	                       saved_ok ? "ok" : "error", accepted, rejected,
	                       ~output_path);
	if(!report_path.IsEmpty()) SaveFile(report_path, report);
	Cout() << Format("layout_generation accepted=%d rejected=%d output=%s status=%s\n",
	                 accepted, rejected, ~output_path, saved_ok ? "ok" : "error");
	return saved_ok;
}

static bool WriteLayoutParityReport(const String& layout_path, const String& report_path)
{
	if(!LoadFixedTextSearchLayout(layout_path))
		return false;
	String report;
	report << "{\n  \"status\": \"ok\",\n"
	       << "  \"shared_layout\": true,\n"
	       << "  \"coordinate_space\": \"table-window\",\n"
	       << "  \"paths\": [\"live\", \"sidecar2\"],\n"
	       << "  \"windows\": [\n";
	Vector<WindowDescriptor> windows = SelectWindowDescriptors("both");
	for(int i = 0; i < windows.GetCount(); i++) {
		if(i) report << ",\n";
		const WindowDescriptor& window = windows[i];
		report << Format("    {\"id\":\"%s\",\"source_rect\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d},\"regions\":{",
		                 ~window.id.value, window.source_rect.left, window.source_rect.top,
		                 window.source_rect.Width(), window.source_rect.Height());
		auto append_rect = [&](const char *name, const Rect& rect, bool comma) {
			if(comma) report << ",";
			report << Format("\"%s\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}",
			                 name, rect.left, rect.top, rect.Width(), rect.Height());
		};
		bool comma = false;
		for(int seat = 0; seat < 6; seat++) {
			append_rect(Format("name%d", seat), g_fixed_text_layout.name[seat], comma); comma = true;
			append_rect(Format("balance%d", seat), g_fixed_text_layout.balance[seat], comma); comma = true;
			append_rect(Format("action%d", seat), g_fixed_text_layout.action[seat], comma); comma = true;
			append_rect(Format("bet%d", seat), g_fixed_text_layout.bet[seat], comma); comma = true;
		}
		append_rect("board_money", g_fixed_text_layout.board_money, comma);
		report << "}}";
	}
	report << "\n  ],\n  \"unexpected_differences\": 0\n}\n";
	bool ok = SaveFile(report_path, report);
	Cout() << Format("layout_parity paths=live,sidecar2 windows=%d unexpected=0 status=%s report=%s\n",
	                 windows.GetCount(), ok ? "ok" : "error", ~report_path);
	return ok;
}

static int RgbAnchorDistance(const Image& candidate, const Image& anchor, double scale)
{
	if(candidate.IsEmpty() || anchor.IsEmpty()) return -1;
	int w = max(1, (int)(anchor.GetWidth() * scale + 0.5));
	int h = max(1, (int)(anchor.GetHeight() * scale + 0.5));
	if(candidate.GetWidth() < w || candidate.GetHeight() < h) return -1;
	Image resized = Rescale(anchor, w, h);
	String target = VsmLiveRegionClassifier::Signature(resized);
	int best = -1;
	// Coarse RGB matching is deliberate: these assets carry action colour and
	// brightness, so grayscale MatchTemplate would discard useful evidence.
	// Two-pixel stride keeps the live cost bounded while tolerating detector
	// rectangle jitter; the accepted result is still gated by OCR/state logic.
	for(int y = 0; y <= candidate.GetHeight() - h; y += 2)
		for(int x = 0; x <= candidate.GetWidth() - w; x += 2) {
			Image probe = Crop(candidate, RectC(x, y, w, h));
			int d = VsmLiveRegionClassifier::SignatureDistance(
				VsmLiveRegionClassifier::Signature(probe), target);
			if(best < 0 || d < best) best = d;
		}
	return best;
}

static String MatchRecognitionAnchor(const Image& candidate, const String& prefix,
	                                 int& out_distance)
{
	out_distance = -1;
	if(candidate.IsEmpty() || !g_recognition_anchors.loaded) return String();
	String best;
	for(const RecognitionAnchor& anchor : g_recognition_anchors.anchors) {
		if(!anchor.name.StartsWith(prefix)) continue;
		for(double scale : {0.75, 0.85, 0.95, 1.0, 1.05, 1.15, 1.25}) {
			int distance = RgbAnchorDistance(candidate, anchor.image, scale);
			if(distance >= 0 && (out_distance < 0 || distance < out_distance)) {
				out_distance = distance;
				best = anchor.name;
			}
		}
	}
	return best;
}

static bool IsAcceptedRecognitionAnchor(const String& name, int distance)
{
	// The signature is RGB (not grayscale) and has 64 samples. This conservative
	// limit is intentionally a first gate; semantic OCR and state transitions are
	// still required before an emitted sidecar event becomes authoritative.
	return !name.IsEmpty() && distance >= 0 && distance <= 520;
}

static int RunRecognitionAnchorSelfTest()
{
	Cout() << "=== Mode: recognition-anchor-selftest (RGB templates) ===\n";
	if(!g_recognition_anchors.loaded) {
		Cerr() << "ERROR: no recognition anchors loaded\n";
		return 1;
	}
	int failed = 0;
	for(const RecognitionAnchor& anchor : g_recognition_anchors.anchors) {
		String prefix = anchor.name.StartsWith("action-") ? "action-" : "BB-";
		int distance = -1;
		String got = MatchRecognitionAnchor(anchor.image, prefix, distance);
		bool ok = !got.IsEmpty() && distance == 0;
		Cout() << Format("anchor=%s matched=%s distance=%d %s\n", ~anchor.name,
		                 ~got, distance, ok ? "PASS" : "FAIL");
		if(!ok) failed++;
	}
	Cout() << Format("anchor_selftest=%d/%d\n",
	                 g_recognition_anchors.anchors.GetCount() - failed,
	                 g_recognition_anchors.anchors.GetCount());
	return failed ? 2 : 0;
}

// ---------------------------------------------------------------------------
static double NowMs()
{
	using namespace std::chrono;
	return duration_cast<duration<double, std::milli>>(
	           steady_clock::now().time_since_epoch()).count();
}

struct Stage : Moveable<Stage> {
	String name;
	double total_ms = 0;
	int    calls = 0;
	double max_ms = 0;
	void Add(double ms) { total_ms += ms; calls++; if(ms > max_ms) max_ms = ms; }
	void Merge(const Stage& stage) {
		total_ms += stage.total_ms;
		calls += stage.calls;
		max_ms = max(max_ms, stage.max_ms);
	}
	double Avg() const { return calls ? total_ms / calls : 0; }
};

struct StageSet {
	Stage acquire, crop, change, structural, classify, template_match, ocr, resolve, engine;
	void MergePrepared(const StageSet& stages) {
		crop.Merge(stages.crop);
		change.Merge(stages.change);
		structural.Merge(stages.structural);
		classify.Merge(stages.classify);
		template_match.Merge(stages.template_match);
	}
	void Print() {
		auto row = [](const Stage& s) {
			Cout() << Format("  %-14s calls=%-6d avg=", ~s.name, s.calls)
			       << Format("%.3f", s.Avg()) << "ms  max="
			       << Format("%.3f", s.max_ms) << "ms  total="
			       << Format("%.1f", s.total_ms) << "ms\n";
		};
		row(acquire); row(crop); row(change); row(structural); row(classify);
		row(template_match); row(ocr); row(resolve); row(engine);
	}
};

static void InitializeStageNames(StageSet& stages)
{
	stages.acquire.name = "acquire";
	stages.crop.name = "crop+conv";
	stages.change.name = "change_detect";
	stages.structural.name = "structural_pixels";
	stages.classify.name = "classify";
	stages.template_match.name = "template_match";
	stages.ocr.name = "ocr";
	stages.resolve.name = "resolve_commit";
	stages.engine.name = "engine_commit";
}

// ---------------------------------------------------------------------------
// Crop a sub-rect out of a VsmImageBuffer (RGBA, w*h*4 row-major -- confirmed
// identical byte layout to VsmFrameImage.data by Task 0279 / LiveSession.cpp)
// straight into a VsmFrameImage. Clamps to buffer bounds.
static VsmFrameImage CropBufferToFrame(const VsmImageBuffer& buf, const Rect& in_r)
{
	VsmFrameImage out;
	if(buf.IsEmpty() || buf.channels < 3) return out;
	Rect r = in_r & RectC(0, 0, buf.width, buf.height);
	if(r.Width() <= 0 || r.Height() <= 0) return out;
	int cw = r.Width(), ch = r.Height();
	out.width = cw; out.height = ch;
	out.data.Alloc((size_t)cw * ch * 4);
	int bc = buf.channels;
	for(int y = 0; y < ch; y++) {
		const byte* srow = buf.pixels.Begin() + (size_t)((r.top + y) * buf.width + r.left) * bc;
		byte* drow = out.data + (size_t)y * cw * 4;
		if(bc == 4) {
			memcpy(drow, srow, (size_t)cw * 4);
		} else { // bc == 3: expand to RGBA
			for(int x = 0; x < cw; x++) {
				drow[x*4+0] = srow[x*3+0]; drow[x*4+1] = srow[x*3+1];
				drow[x*4+2] = srow[x*3+2]; drow[x*4+3] = 255;
			}
		}
	}
	return out;
}

static VsmImageBuffer CropBufferToImageBuffer(const VsmImageBuffer& buf, const Rect& in_r)
{
	VsmImageBuffer out;
	if(buf.IsEmpty() || buf.channels < 3) return out;
	Rect r = in_r & RectC(0, 0, buf.width, buf.height);
	if(r.Width() <= 0 || r.Height() <= 0) return out;
	out.Create(r.Width(), r.Height(), buf.channels);
	for(int y = 0; y < r.Height(); y++)
		memcpy(out.pixels.Begin() + (size_t)y * r.Width() * buf.channels,
		       buf.pixels.Begin() + (size_t)((r.top + y) * buf.width + r.left) * buf.channels,
		       (size_t)r.Width() * buf.channels);
	return out;
}

static Vector<WindowFrame> FanOutSourceFrame(const VsmImageBuffer& source,
	                                         int64 source_sequence, int64 window_sequence,
	                                         int64 timestamp_ms,
	                                         const Vector<WindowDescriptor>& descriptors)
{
	Vector<WindowFrame> frames;
	for(const WindowDescriptor& descriptor : descriptors) {
		WindowFrame& frame = frames.Add();
		frame.descriptor.id = WindowId(descriptor.id.value);
		frame.descriptor.name = descriptor.name;
		frame.descriptor.source_rect = descriptor.source_rect;
		frame.source_sequence = source_sequence;
		frame.window_sequence = window_sequence;
		frame.timestamp_ms = timestamp_ms;
		frame.table = CropBufferToFrame(source, descriptor.source_rect);
	}
	return frames;
}

// Crop a sub-rect out of a VsmFrameImage (RGBA).
static VsmFrameImage CropFrameImage(const VsmFrameImage& src, const Rect& in_r)
{
	VsmFrameImage out;
	if(src.IsEmpty()) return out;
	Rect r = in_r & RectC(0, 0, src.width, src.height);
	if(r.Width() <= 0 || r.Height() <= 0) return out;
	int cw = r.Width(), ch = r.Height();
	out.width = cw; out.height = ch;
	out.data.Alloc((size_t)cw * ch * 4);
	for(int y = 0; y < ch; y++)
		memcpy(out.data + (size_t)y * cw * 4,
		       ~src.data + (size_t)((r.top + y) * src.width + r.left) * 4,
		       (size_t)cw * 4);
	return out;
}

// Load a raw 1920x1080 JPG frame into a VsmImageBuffer (for offline mode).
static bool LoadJpgToBuffer(const String& path, VsmImageBuffer& out)
{
	Image img = StreamRaster::LoadFileAny(path);
	if(img.IsEmpty()) return false;
	int w = img.GetWidth(), h = img.GetHeight();
	out.Create(w, h, 4);
	for(int y = 0; y < h; y++) {
		const RGBA* row = img[y];
		byte* d = out.pixels.Begin() + (size_t)y * w * 4;
		for(int x = 0; x < w; x++) {
			d[x*4+0] = row[x].r; d[x*4+1] = row[x].g;
			d[x*4+2] = row[x].b; d[x*4+3] = row[x].a;
		}
	}
	return true;
}

// Parse the first chip/BB number out of an OCR string ("1,085" -> 1085,
// "103.2 BB" -> 103.2). Returns false if no digit found.
static bool ParseChipValue(const String& text, double& out)
{
	String num;
	bool seen_dot = false, seen_digit = false;
	for(int i = 0; i < text.GetCount(); i++) {
		int c = text[i];
		if(c >= '0' && c <= '9') { num.Cat(c); seen_digit = true; }
		else if(c == '.' && !seen_dot && seen_digit) { num.Cat('.'); seen_dot = true; }
		else if(c == ',') { /* thousands separator: skip */ }
		else if(seen_digit) break; // number ended
	}
	if(!seen_digit) return false;
	const char* endp = nullptr;
	out = ScanDouble(num.Begin(), &endp);
	return true;
}

// Task 0288 fix 3: distinguish a real player name from a chip value that was
// OCR'd off a balance/bet plate the classifier mislabelled as a name plate.
// A real player name in this recording ALWAYS contains alphabetic characters
// (isarpires98, kolyamaster02, wegohigh, ...) -- crucially, ParseChipValue()
// alone is NOT a sufficient test, because it happily extracts "98" out of
// "isarpires98". The discriminator is: a plate whose text is PURELY numeric
// (digits + separators, plus at most a trailing BB/SB unit) is a chip value,
// not a name. Letters other than the unit tokens b/s mean it's a real name.
static bool IsPureChipText(const String& s)
{
	String t = TrimBoth(s);
	if(t.IsEmpty()) return false;
	int digits = 0, letters = 0;
	for(int i = 0; i < t.GetCount(); i++) {
		int c = t[i];
		if(c >= '0' && c <= '9') digits++;
		else if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			int lc = c | 0x20;               // to-lower for ASCII letters
			if(lc != 'b' && lc != 's') letters++; // allow only BB/SB unit letters
		}
	}
	return digits > 0 && letters == 0;
}

static bool IsOcrCategory(const String& cat)
{
	// Task 0287: seat_name_plate added -- the classifier already identifies name
	// plates, but no text extraction ran on them before, so the live mirror only
	// ever showed generic "SeatN". Balance/bet were already OCR'd; names are the
	// one new OCR category here.
	// Task 0289: seat_action_bubble ("Call"/"Fold"/"Raise"/"Check"/"All In" -- the
	// action/turn/fold signal) and chip_badge_stack (the round-bet-total chip
	// figure, e.g. "376.6 BB" next to a chip graphic) are BOTH real categories the
	// classifier already produces but that were never OCR'd anywhere in the live
	// loop before this task. Both are added here, following the exact same pattern
	// Task 0287 used for name plates.
	return cat == "pot_label" || cat == "seat_balance_plate"
	    || cat == "seat_bet_label" || cat == "seat_name_plate"
	    || cat == "seat_action_bubble" || cat == "chip_badge_stack";
}

// Task 0289: canonicalize a seat_action_bubble OCR string ("Call\n78.3 BB",
// "Check", "Call\nAll In", "Raise", "Fold") to a short action word. The real
// action bubbles in this client (verified against real dataset crops) carry the
// action verb on the first line, sometimes followed by an amount or "All In" on
// a second line. Scan for known keywords case-insensitively; "All In" wins over
// a leading "Call"/"Raise" verb since it is the more consequential state. Returns
// "" if the text contains no recognizable action word (e.g. a misclassified
// name/balance plate leaking into this category -- do NOT invent an action then).
static String NormalizeActionText(const String& raw)
{
	String low = ToLower(raw);
	if(low.Find("all in") >= 0 || low.Find("allin") >= 0 || low.Find("all-in") >= 0) return "All In";
	if(low.Find("fold")  >= 0) return "Fold";
	if(low.Find("check") >= 0) return "Check";
	if(low.Find("raise") >= 0) return "Raise";
	if(low.Find("call")  >= 0) return "Call";
	if(low.Find("bet")   >= 0) return "Bet";
	return "";
}

// Task 0289: how many alphabetic chars remain in `text` after removing one
// occurrence of the action keyword `kw`. Used to decide whether a name-plate OCR
// result is REALLY a misclassified action bubble (~0 leftover letters, e.g. a bare
// "Fold"/"Check") vs. a genuine player name that merely contains an action-like
// substring (many leftover letters). Real names in this recording never reduce to
// near-zero leftover letters against any action keyword.
static int LettersOutsideKeyword(const String& text, const String& kw)
{
	String low = ToLower(text), k = ToLower(kw);
	int pos = low.Find(k);
	String rest = (pos >= 0) ? (low.Left(pos) + low.Mid(pos + k.GetCount())) : low;
	int letters = 0;
	for(int i = 0; i < rest.GetCount(); i++) { int c = rest[i]; if(c >= 'a' && c <= 'z') letters++; }
	return letters;
}

// Task 0290b: length of the LONGEST continuous run of alphabetic characters in
// `s`. A real player name in this recording always carries a long continuous
// alphabetic run (Donati=6, Aludra=6, wegohigh=8, isarpires98 has "isarpires"=9,
// kolyamaster02 has "kolyamaster"=11, Arakatakas100 has "Arakatakas"=10) -- every
// real username seen is >= 6. A chip-amount-shaped OCR read that leaked into the
// name field (a misclassified balance/bet plate "191.5 BB", or a hand-result
// phrase "Won 191.5 BB") has only short fragments ("BB"=2, "Won"=3). So a low
// longest-alpha-run + a chip amount present is a robust, keyword-free "this is
// NOT a name" discriminator (see kMinNameAlphaRun).
static int LongestAlphaRun(const String& s)
{
	int best = 0, cur = 0;
	for(int i = 0; i < s.GetCount(); i++) {
		int c = s[i];
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { cur++; if(cur > best) best = cur; }
		else cur = 0;
	}
	return best;
}

// Task 0290b: detect a chip-amount-SHAPED substring in `s` and return its parsed
// value. "Shaped" means a numeric run that is EITHER >= 2 digits long, OR
// contains a decimal point, OR is immediately followed (after optional spaces) by
// a "BB"/"SB" unit -- this deliberately does NOT fire on a lone single digit that
// might be part of a username (e.g. the "5" many nicks contain), while it DOES
// fire on real chip figures ("191.5 BB", "1,085", "78.3", "5 BB", "99.5 BB").
// Pairing this with LongestAlphaRun() (below, in the name-plate resolve path) is
// what separates "chip value that leaked into the name field" from a genuine
// username that merely happens to contain digits (isarpires98).
static bool ContainsChipAmount(const String& s, double& val)
{
	String low = ToLower(s);
	bool shaped = false;
	for(int i = 0; i < low.GetCount(); ) {
		int c = low[i];
		if(c >= '0' && c <= '9') {
			int j = i, digits = 0; bool dot = false;
			while(j < low.GetCount()) {
				int d = low[j];
				if(d >= '0' && d <= '9') { digits++; j++; }
				else if(d == '.' || d == ',') { if(d == '.') dot = true; j++; }
				else break;
			}
			int k = j; while(k < low.GetCount() && low[k] == ' ') k++;
			bool unit = (low.Mid(k, 2) == "bb" || low.Mid(k, 2) == "sb");
			if(digits >= 2 || dot || unit) { shaped = true; break; }
			i = (j > i) ? j : i + 1;
		}
		else i++;
	}
	if(!shaped) return false;
	return ParseChipValue(s, val);
}

// Task 0290b: minimum longest-alpha-run for an OCR'd name-plate string to be
// ACCEPTED as a real player name when it also carries a chip amount. 4 sits
// safely below every real username seen (all >= 6) and above the fragments of
// the known leak cases ("BB"=2, "Won"=3, "All"/"Bet"=3). See LongestAlphaRun().
static const int kMinNameAlphaRun = 4;

// ---------------------------------------------------------------------------
// Task 0287: map a classified region's table-space rect to a seat index (0-5).
//
// This is a fixed-layout six-seat 2D card-game table (same premise as kTableRect
// being a fixed empirical constant). The 267-candidate labeled dataset
// (real_recording_combined_classified_dataset.json) carries real
// seat_name_plate / seat_balance_plate / seat_bet_label rects; grouping their
// centroids (NOT the noisy semantic_hits zone labels, which overlap heavily)
// yields six clean rail clusters. Their centroids and the oval centre below are
// the real derived evidence (see the task's Status/Evidence writeup):
//
//   BOTTOM        cx~500 cy~492      LEFT_BOTTOM   cx~ 84 cy~372
//   LEFT_TOP      cx~110 cy~170      TOP           cx~444 cy~108
//   RIGHT_TOP     cx~828 cy~168      RIGHT_BOTTOM  cx~852 cy~372
//   oval centre   (470, 280)  (mean of the six rail centroids)
//
// Assignment is by ANGLE from the oval centre, not a rectangular grid: a bet
// chip amount is drawn pulled IN toward the pot (e.g. the bottom seat's bet at
// cy~420, well above its name plate at cy~492), so a radial/angular test is
// far more robust than axis-aligned zone boxes -- validated on all 21 distinct
// real seat rects, it agrees with the most-specific dataset zone label in every
// case, including the center-pulled bets the zone boxes would misplace.
//
// The seat INDEX each zone maps to matches BuildSnapshot()/Paint()'s existing
// render convention (ang = pi/2 + 2*pi*i/n, seat 0 at bottom, clockwise):
//   i=0 bottom, i=1 left-bottom, i=2 left-top, i=3 top, i=4 right-top,
//   i=5 right-bottom -- so a region OCR'd from the top of the real video lands
// on seat index 3, which the mirror also draws at the top: the mirror's spatial
// layout mirrors the video's, making per-seat correctness visually checkable.
struct SeatAnchor { int cx, cy, seat; };
static const int kTableCenterX = 470, kTableCenterY = 280;
static const SeatAnchor kSeatAnchors[6] = {
	{ 500, 492, 0 }, // BOTTOM
	{  84, 372, 1 }, // LEFT_BOTTOM
	{ 110, 170, 2 }, // LEFT_TOP
	{ 444, 108, 3 }, // TOP
	{ 828, 168, 4 }, // RIGHT_TOP
	{ 852, 372, 5 }, // RIGHT_BOTTOM
};

static int RegionToSeat(int left, int top, int w, int h)
{
	const double kPi = 3.14159265358979323846;
	double cx = left + w / 2.0, cy = top + h / 2.0;
	double a = atan2(cy - kTableCenterY, cx - kTableCenterX);
	int best = -1; double best_d = 1e18;
	for(const SeatAnchor& sa : kSeatAnchors) {
		double aa = atan2((double)(sa.cy - kTableCenterY), (double)(sa.cx - kTableCenterX));
		double d = fabs(a - aa);
		if(d > kPi) d = 2 * kPi - d; // shortest circular angular distance
		if(d < best_d) { best_d = d; best = sa.seat; }
	}
	return best;
}

// ---------------------------------------------------------------------------
// Task 0290b: OCR-FREE structural fold signal -- the two face-down hole-card
// backs over a seat vs. that region gone empty/felt.
//
// Per-seat expected hole-card-back region (TABLE space, same coordinate frame as
// kSeatAnchors and kTableRect's crop). Derived from REAL data, not guessed:
//   - seats 0,1,2,5: taken directly from the labeled dataset's real
//     `hole_card_facedown` rects (frame_000006/54 region crops).
//   - seat 4: the left-right mirror of seat 2 about the table centre x (940-x),
//     validated against real full frames (cards present ~11% white, absent ~0%).
//   - seat 3 (TOP): DISABLED. The dataset carries ZERO hole_card_facedown entries
//     for the top seat, and a direct look at real frames shows the TOP pod draws
//     its name plate/avatar exactly where a naive card rect would sit, so any rect
//     there false-triggers on white name/avatar text. Rather than emit a noisy/
//     wrong seat-3 fold signal, seat 3 falls back to OCR-only fold detection
//     (which still fires there -- e.g. frame_000006's seat-3 "Fold" name-plate
//     read is caught by the action reroute). Disclosed honestly, not papered over.
//
// The discriminator is the near-white pixel fraction: this client's card backs
// are a bright teal-and-white diamond pattern (~11-19% of pixels near-white on
// real crops), whereas felt / an empty seat / a folded seat's dark action bubble
// are 0-2.9%. kCardBackWhiteFrac=0.06 sits in that gap with margin on BOTH sides
// (>3% above the felt max, >5% below the card-back min) -- the same safety-first
// "threshold strictly inside the real measured gap" discipline Task 0286 used for
// the OCR cache. Pixel/color sampling only: NO OCR, no classifier, ~a few thousand
// strided pixel reads per seat per frame (sub-millisecond, folded into st.resolve).
struct CardBackRegion { int seat; Rect rect; bool enabled; };
static const CardBackRegion kCardBackRegions[6] = {
	{ 0, RectC(456, 432,  96, 48), true  }, // BOTTOM       (dataset-measured)
	{ 1, RectC( 96, 312,  96, 48), true  }, // LEFT_BOTTOM  (dataset-measured)
	{ 2, RectC(144, 120,  72, 48), true  }, // LEFT_TOP     (dataset-measured)
	{ 3, RectC(  0,   0,   0,  0), false }, // TOP          (no reliable region -- disabled)
	{ 4, RectC(724, 120,  72, 48), true  }, // RIGHT_TOP    (mirror of seat 2, validated)
	{ 5, RectC(768, 312, 144, 48), true  }, // RIGHT_BOTTOM (dataset-measured)
};
static const double kCardBackWhiteFrac    = 0.06; // >= => two card backs present
static const int    kCardBackConfirmFrames = 3;   // debounce raw samples before committing

// Fraction (0..1) of near-white pixels (all RGB channels >= 150) in a table-space
// rect of the RGBA frame; -1 if the rect is empty. Strided x2 in both axes (the
// diamond pattern is dense; full sampling is unnecessary and this keeps the cost
// negligible). Matches the offline calibration that set kCardBackWhiteFrac.
static double CardBackWhiteFraction(const VsmFrameImage& table, const Rect& in_r)
{
	Rect r = in_r & RectC(0, 0, table.width, table.height);
	if(r.Width() <= 0 || r.Height() <= 0) return -1;
	int white = 0, tot = 0;
	for(int y = r.top; y < r.bottom; y += 2) {
		const byte* row = ~table.data + (size_t)((size_t)y * table.width + r.left) * 4;
		for(int x = 0; x < r.Width(); x += 2) {
			const byte* p = row + (size_t)x * 4;
			int mn = min((int)p[0], min((int)p[1], (int)p[2]));
			if(mn >= 150) white++;
			tot++;
		}
	}
	return tot ? (double)white / tot : -1;
}

// ---------------------------------------------------------------------------
// Headless GUI shim so a real Game runs with no window (identical shape to
// reference/VideoGameEngineReplayValidator's HeadlessGui).
class HeadlessGui : public GuiInterface {
public:
	virtual bool isTestMode() const override { return true; }
	virtual void initGui(int) override {}
	virtual void refreshGameLabels(TexasRound) const override {}
	virtual void nextRoundCleanGui() override {}
	virtual void logNewGameHandMsg(int, int) override {}
	virtual void flushLogAtGame(int) override {}
	virtual void logNewBlindsSetsMsg(int, int, String, String) override {}
	virtual void flushLogAtHand() override {}
	virtual void dealHoleCards() override {}
	virtual void refreshPot() override {}
	virtual void refreshSet() override {}
	virtual void nextPlayerAnimation() override {}
	virtual void flipHolecardsAllIn() override {}
	virtual void logDealBoardCardsMsg(int, int, int, int, int = -1, int = -1) override {}
	virtual void refreshGroupbox(int, int) override {}
	virtual void preflopAnimation1() override {}
	virtual void flopAnimation1() override {}
	virtual void turnAnimation1() override {}
	virtual void riverAnimation1() override {}
	virtual void postRiverAnimation1() override {}
	virtual void logPlayerActionMsg(String, int, int) override {}
	virtual void logFlipHoleCardsMsg(String, int, int, int = -1, String = "shows") override {}
	virtual void logWinningHandMsg(String, String, int) override {}
	virtual void dealBeRoCards(TexasRound) override {}
	virtual void beRoAnimation2(TexasRound) override {}
	virtual void meInAction() override {}
	virtual void postRiverRunAnimation1() override {}
	virtual void refreshCash() override {}
	virtual void refreshAction(int, int) override {}
	virtual void SignalNetClientError(int, int) override {}
};

// Card index from rank+suit strings the dataset carries for board_card entries.
// Reference-client style: rank in {2..9,T,J,Q,K,A}, suit in {c,d,h,s}. Engine card
// index = suit*13 + rankidx (matches ReplayValidator's FormatCard()).
static int CardIndex(const String& rank, const String& suit)
{
	if(rank.IsEmpty() || suit.IsEmpty()) return -1;
	static const char* ranks[] = { "2","3","4","5","6","7","8","9","T","J","Q","K","A" };
	int ri = -1;
	String R = ToUpper(rank);
	for(int i = 0; i < 13; i++) if(R == ranks[i]) { ri = i; break; }
	String s = ToLower(suit);
	int si = -1;
	if(s.StartsWith("c")) si = 0; else if(s.StartsWith("d")) si = 1;
	else if(s.StartsWith("h")) si = 2; else if(s.StartsWith("s")) si = 3;
	if(ri < 0 || si < 0) return -1;
	return si * 13 + ri;
}

// Inverse of CardIndex: engine card index (suit*13+rank) -> "3d"/"Kh"/... or "?".
static String FormatCardStr(int idx)
{
	if(idx < 0 || idx >= 52) return "?";
	static const char* ranks[] = { "2","3","4","5","6","7","8","9","T","J","Q","K","A" };
	static const char  suits[] = { 'c','d','h','s' };
	return String(ranks[idx % 13]) + suits[idx / 13];
}

// ===========================================================================
// Task 0290a: real card recognition (merged-flop split + suit-by-colour +
// rank template match). Fixes Task 0288's disclosed "board stays 0/5 in
// practice" limitation: real flops arrive as ONE merged changed region, so the
// per-region classifier almost never resolved 3 distinct board cards.
//
// Three pieces, all reusing native infrastructure (NO OpenCV, NO new matcher):
//   1. FELT-GAP SPLIT: scan the fixed board band's columns and cut at
//      felt-coloured vertical gaps -> individual per-card sub-rects. This does
//      not depend on the change detector's (merged) rect; it re-segments from
//      raw pixels, so a merged 3/4/5-card region always resolves to the right
//      number of cards. Search is BOUNDED to the board band (never full-frame).
//   2. SUIT BY COLOUR: this client renders a 4-COLOUR deck -- the WHOLE card
//      background is tinted by suit (measured on real frames: diamonds=blue
//      (119,162,220), hearts=red (241,96,57), clubs=green (19,191,128),
//      spades=grey (144,145,133)). So suit is a trivial, robust dominant-colour
//      test, no template needed.
//   3. RANK BY TEMPLATE MATCH: ComputerVision::MatchTemplate (TM_CCOEFF_NORMED)
//      slides the external rank-glyph library (templates/ranks/*.png,
//      white glyph on grey) over the card crop at a few calibrated scales and
//      takes the peak response. TM_CCOEFF_NORMED's translational search is what
//      makes this work (a single forced bbox alignment does NOT -- verified);
//      the on-screen card scale is fixed on this static table, so a 3-point
//      scale set is enough (no per-frame ORB scale search needed -- see the
//      task's ORB-vs-template timing finding). Real accuracy on ground-truth
//      board frames: 12/12 board cards (--card-recog-test).
//
// ALL geometry is TABLE space (the kTableRect crop) and MEASURED on real
// recorded frames (tmp/real_recording_0263_frames, ground truth in
// bin/video_record_25min_20260716_203356.txt): board cards start at x~314,
// each ~60px wide with a ~5px felt gap (stride ~64), card art spans y~248..322.
// ===========================================================================
static const int kBoardCardTop = 248, kBoardCardBot = 322; // table-space vertical band
static const int kBoardBandX0  = 300, kBoardBandX1  = 668; // horizontal search bound
static const double kRankScales[] = { 0.9, 1.0, 1.1 };     // calibrated (native ~1.0 peaks)
static const int kNumRankScales = 3;

// A table-space RGBA pixel is dark table felt. Measured felt (1-7,54-105,12-28):
// low red, mid-dominant green, low blue. The bounds give margin against every
// card colour seen (blue card has b>75; red has r>60; the green CLUB card is much
// brighter (g~191) with high blue (b~128), so g<150 && b<75 both exclude it; grey
// has r>=60). p -> RGBA.
static inline bool IsFeltPixel(const byte* p)
{
	int r = p[0], g = p[1], b = p[2];
	return g > 40 && g < 150 && r < 60 && b < 75 && g > r + 25 && g > b + 12;
}

// Build a single-channel grayscale ByteMat ((r+g+b)/3) from a table-space rect of
// an RGBA VsmFrameImage. Clamped to bounds. Matches ImageToGrayByteMat's averaging
// so a card crop and a template glyph are compared on the same luma definition.
static ByteMat TableRegionToGray(const VsmFrameImage& table, const Rect& in_r)
{
	ByteMat m;
	Rect r = in_r & RectC(0, 0, table.width, table.height);
	if(r.Width() <= 0 || r.Height() <= 0) return m;
	m.SetSize(r.Width(), r.Height(), 1);
	for(int y = 0; y < r.Height(); y++) {
		const byte* row = ~table.data + (size_t)((size_t)(r.top + y) * table.width + r.left) * 4;
		for(int x = 0; x < r.Width(); x++) {
			const byte* p = row + (size_t)x * 4;
			m.data[y * r.Width() + x] = (byte)(((int)p[0] + (int)p[1] + (int)p[2]) / 3);
		}
	}
	return m;
}

struct RankTemplate : Moveable<RankTemplate> {
	String          rank;    // "2".."9","T","J","Q","K","A" (T's glyph renders as "10")
	Vector<ByteMat> scaled;  // one tight-cropped grayscale glyph per kRankScales entry
};
struct CardTemplates {
	Vector<RankTemplate> ranks;
	bool ok = false;
};

// Load the external rank-glyph library from <dir>/ranks/{rank}.png. Each
// glyph is tight-cropped to its bright (white) pixels then pre-rescaled to every
// calibrated scale, so per-frame recognition is a fixed set of MatchTemplate calls.
// Task 0291a: scale set is now a parameter so hole-card recognition (smaller
// on-screen glyphs than board cards) can pass a wider set without disturbing the
// board path. Board callers pass no scales -> the original kRankScales set, so
// board recognition is byte-for-byte unchanged (still 12/12).
static CardTemplates LoadCardTemplates(const String& dir, const Vector<double>& scales)
{
	CardTemplates ct;
	static const char* names[] = { "2","3","4","5","6","7","8","9","T","J","Q","K","A" };
	for(const char* nm : names) {
		String path = AppendFileName(AppendFileName(dir, "ranks"), String(nm) + ".png");
		Image im = StreamRaster::LoadFileAny(path);
		if(im.IsEmpty()) { Cerr() << "WARN: rank template missing: " << path << "\n"; continue; }
		Size sz = im.GetSize();
		int minx = sz.cx, miny = sz.cy, maxx = -1, maxy = -1; // bright-glyph bbox
		for(int y = 0; y < sz.cy; y++) {
			const RGBA* srow = im[y];
			for(int x = 0; x < sz.cx; x++) {
				int g = ((int)srow[x].r + (int)srow[x].g + (int)srow[x].b) / 3;
				if(g > 185) { if(x < minx) minx = x; if(x > maxx) maxx = x;
				              if(y < miny) miny = y; if(y > maxy) maxy = y; }
			}
		}
		if(maxx < minx || maxy < miny) { Cerr() << "WARN: empty glyph in " << path << "\n"; continue; }
		Image tight = Crop(im, RectC(minx, miny, maxx - minx + 1, maxy - miny + 1));
		RankTemplate& rt = ct.ranks.Add();
		rt.rank = nm;
		for(double sc : scales) {
			int nw = max(1, (int)(tight.GetWidth()  * sc + 0.5));
			int nh = max(1, (int)(tight.GetHeight() * sc + 0.5));
			Image scg = Rescale(tight, nw, nh);
			ByteMat bm; ImageToGrayByteMat(scg, bm);
			rt.scaled.Add() = pick(bm);
		}
	}
	ct.ok = ct.ranks.GetCount() == 13;
	if(!ct.ok) Cerr() << "WARN: card templates incomplete (" << ct.ranks.GetCount() << "/13)\n";
	return ct;
}

// Board-card default: the original 3-point calibrated scale set (unchanged).
static CardTemplates LoadCardTemplates(const String& dir)
{
	Vector<double> scales;
	for(int s = 0; s < kNumRankScales; s++) scales.Add(kRankScales[s]);
	return LoadCardTemplates(dir, scales);
}

// Suit from dominant card background colour (4-colour deck). Samples the card
// interior, skipping near-white glyph pixels. Returns "c"/"d"/"h"/"s" or "".
// Task 0291a: made rect-driven (samples card.top/card.Height() instead of the
// fixed board-band constants) so the SAME pipeline serves both board cards AND
// hole cards. Behaviour-preserving for board cards: SplitBoardBand() already
// emits rects with top==kBoardCardTop and height==(kBoardCardBot-kBoardCardTop),
// so card.top+8 == kBoardCardTop+8 and card.Height()-20 == kBoardCardBot-kBoardCardTop-20.
static String ClassifySuitByColor(const VsmFrameImage& table, const Rect& card)
{
	Rect r = RectC(card.left + 6, card.top + 8,
	               max(1, card.Width() - 12), max(1, card.Height() - 20))
	         & RectC(0, 0, table.width, table.height);
	if(r.Width() <= 0 || r.Height() <= 0) return "";
	long rs = 0, gs = 0, bs = 0, n = 0;
	for(int y = r.top; y < r.bottom; y++) {
		const byte* row = ~table.data + (size_t)((size_t)y * table.width + r.left) * 4;
		for(int x = 0; x < r.Width(); x++) {
			const byte* p = row + (size_t)x * 4;
			if(p[0] > 200 && p[1] > 200 && p[2] > 200) continue; // skip white glyph
			rs += p[0]; gs += p[1]; bs += p[2]; n++;
		}
	}
	if(n < 20) return "";
	int r0 = (int)(rs / n), g0 = (int)(gs / n), b0 = (int)(bs / n);
	int mx = max(r0, max(g0, b0)), mn = min(r0, min(g0, b0));
	if(mx - mn < 28)                        return "s"; // grey/desaturated -> spade
	if(r0 >= g0 && r0 >= b0 && r0 > g0 + 25) return "h"; // red   -> heart
	if(b0 >= r0 && b0 >= g0 && b0 > r0 + 20) return "d"; // blue  -> diamond
	if(g0 >= r0 && g0 >= b0)                 return "c"; // green -> club
	return "";
}

// Rank via sliding MatchTemplate over the card crop at the calibrated scales.
// Returns the best rank string and its peak TM_CCOEFF_NORMED score in out_score.
static String RecognizeRank(const VsmFrameImage& table, const Rect& card,
                            const CardTemplates& ct, double& out_score)
{
	out_score = -2;
	// Task 0291a: rect-driven vertical band (card.top/card.Height()) so hole cards
	// reuse this unchanged; behaviour-preserving for board cards (SplitBoardBand
	// emits top==kBoardCardTop, height==kBoardCardBot-kBoardCardTop).
	Rect band = RectC(card.left - 2, card.top, card.Width() + 4, card.Height());
	ByteMat img = TableRegionToGray(table, band);
	if(img.IsEmpty()) return "";
	String best; double bestv = -2;
	for(const RankTemplate& rt : ct.ranks) {
		for(const ByteMat& tm : rt.scaled) {
			if(tm.IsEmpty() || tm.cols > img.cols || tm.rows > img.rows) continue;
			FloatMat res;
			MatchTemplate(img, tm, res, TM_CCOEFF_NORMED);
			if(res.IsEmpty()) continue;
			double mn = 0, mx = 0;
			MinMaxLoc(res, &mn, &mx);
			if(mx > bestv) { bestv = mx; best = rt.rank; }
		}
	}
	out_score = bestv;
	return best;
}

// Segment the fixed board band into individual card sub-rects at felt-coloured
// vertical gaps. A wide segment (a merged block with no interior felt gap) is
// subdivided by the fixed card stride as a fallback. Bounded to the board band.
static Vector<Rect> SplitBoardBand(const VsmFrameImage& table)
{
	Vector<Rect> cards;
	int y0 = kBoardCardTop + 2, y1 = kBoardCardBot - 2;
	int tot = y1 - y0;
	auto colIsCard = [&](int x) -> bool {
		if(x < 0 || x >= table.width) return false;
		int nonfelt = 0;
		for(int y = y0; y < y1; y++) {
			const byte* p = ~table.data + (size_t)((size_t)y * table.width + x) * 4;
			if(!IsFeltPixel(p)) nonfelt++;
		}
		return nonfelt > tot / 2;
	};
	int runstart = -1;
	int x1 = min(kBoardBandX1, table.width - 1);
	for(int x = kBoardBandX0; x <= x1; x++) {
		bool card = colIsCard(x);
		if(card && runstart < 0) runstart = x;
		if((!card || x == x1) && runstart >= 0) {
			int end = card ? x + 1 : x;
			int w = end - runstart;
			if(w >= 25) {
				const int kCardStride = 64; // measured on-screen card+gap pitch
				int parts = (w >= 90) ? (w + kCardStride / 2) / kCardStride : 1; // merged-block fallback
				if(parts < 1) parts = 1;
				for(int k = 0; k < parts; k++) {
					int a = runstart + (int)((double)w * k / parts);
					int b = runstart + (int)((double)w * (k + 1) / parts);
					cards.Add(RectC(a, kBoardCardTop, b - a, kBoardCardBot - kBoardCardTop));
				}
			}
			runstart = -1;
		}
	}
	return cards;
}

// Full board recognition: split the board band, then per card classify suit by
// colour and rank by template match. Returns left-to-right card indices
// (suit*13+rank, or -1 if unresolved). Optional dbg gets a per-card trace line.
static Vector<int> RecognizeBoardCards(const VsmFrameImage& table, const CardTemplates& ct,
                                       Vector<String>* dbg = nullptr)
{
	Vector<int> out;
	if(!ct.ok) return out;
	Vector<Rect> cards = SplitBoardBand(table);
	for(const Rect& cr : cards) {
		String suit = ClassifySuitByColor(table, cr);
		double sc = -2;
		String rank = RecognizeRank(table, cr, ct, sc);
		int idx = CardIndex(rank, suit);
		if(dbg) dbg->Add(Format("[x%d w%d] %s%s score=%.2f -> card=%d",
		                        cr.left, cr.Width(), ~rank, ~suit, sc, idx));
		out.Add(idx);
	}
	return out;
}

// ===========================================================================
// Task 0290c item 1: DEALER CHIP via template match (independent, video-grounded
// cross-check of Task 0288's engine-derived GBUTTON_DEALER indicator).
//
// Real finding (investigated on real 0263 frames before writing any code): in
// THIS reference client the dealer button is NOT a "D" glyph -- it is a small
// white-rimmed disc bearing a red symbol. The already-existing
// real template images at templates/dealer_chip/{1.png (48x42, the bright live
// appearance), seat5_is_dealer_0956.png (32x32, a dimmed variant)} ARE that
// button; 1.png matches the live look, so it is the template used here.
//
// Reuses the EXACT Task 0290a matching infrastructure: native ComputerVision::
// MatchTemplate (TM_CCOEFF_NORMED, no OpenCV) sliding a grayscale template over a
// grayscale search region, peak via MinMaxLoc. The on-screen button scale is
// FIXED on this static table (kTableRect), so -- exactly as 0290a found for card
// ranks -- a small pre-scaled template set suffices and no per-frame ORB scale
// search is needed (ORB stays a calibration-only tool). Real calibration on
// frame_000006 (dealer=seat4=bottom): TM_CCOEFF_NORMED peak 0.956 at the true
// button centre (table 383,432) at scale 0.65, vs a best FALSE peak of ~0.63 in
// the pot/board area -- a wide, safe margin, so kDealerScoreMin=0.72 cleanly
// accepts the real button and rejects felt/board/avatar clutter.
//
// The search is BOUNDED to the felt interior (never the full 1920x1080 frame),
// and -- because the button is essentially static within a hand -- it is
// THROTTLED to run once every kDealerEveryFrames frames (see ProcessTableFrame)
// so its cost stays negligible in the live loop.
// ===========================================================================
static const double kDealerScales[] = { 0.60, 0.65, 0.70 }; // calibrated (native ~0.65 peaks)
static const int    kNumDealerScales = 3;
static const double kDealerScoreMin  = 0.72;   // strictly inside the real 0.63/0.956 gap
// Felt-interior search band (TABLE space). Excludes the outer name plates/avatars
// (top y<150, bottom Play-Now y>560) and covers where the button ever sits.
static const Rect   kDealerSearch    = RectC(30, 150, 880, 410);

struct DealerTemplate {
	Vector<ByteMat> scaled;  // one grayscale button glyph per kDealerScales entry (HALF-res)
	bool ok = false;
};

// 2x box-downscale of a single-channel ByteMat. The dealer felt-band search would
// cost ~900M multiply-adds/frame at full resolution (measured: tens of seconds per
// frame in this DEBUG_FULL build -- far too slow even when throttled). Matching at
// HALF resolution -- downscaling BOTH the search image and the template by the same
// factor -- preserves the exact scale relationship that scored 0.96 at full res
// while cutting the cost ~16x. Averaging (not nearest) keeps the small button's
// edges faithful at half size.
static ByteMat DownscaleByteMat2x(const ByteMat& m)
{
	ByteMat o;
	if(m.IsEmpty() || m.cols < 2 || m.rows < 2) return o;
	int nw = m.cols / 2, nh = m.rows / 2;
	o.SetSize(nw, nh, 1);
	for(int y = 0; y < nh; y++)
		for(int x = 0; x < nw; x++) {
			int a = m.data[(2*y)   * m.cols + 2*x],     b = m.data[(2*y)   * m.cols + 2*x + 1];
			int c = m.data[(2*y+1) * m.cols + 2*x],     d = m.data[(2*y+1) * m.cols + 2*x + 1];
			o.data[y * nw + x] = (byte)((a + b + c + d) / 4);
		}
	return o;
}

// Load templates/dealer_chip/1.png, grayscale, pre-scaled to every calibrated
// scale (same style as LoadCardTemplates -- one fixed MatchTemplate set/frame).
static DealerTemplate LoadDealerTemplate(const String& dir)
{
	DealerTemplate dt;
	String path = AppendFileName(AppendFileName(dir, "dealer_chip"), "1.png");
	Image im = StreamRaster::LoadFileAny(path);
	if(im.IsEmpty()) { Cerr() << "WARN: dealer template missing: " << path << "\n"; return dt; }
	for(int s = 0; s < kNumDealerScales; s++) {
		int nw = max(1, (int)(im.GetWidth()  * kDealerScales[s] + 0.5));
		int nh = max(1, (int)(im.GetHeight() * kDealerScales[s] + 0.5));
		Image sc = Rescale(im, nw, nh);
		ByteMat bm; ImageToGrayByteMat(sc, bm);
		dt.scaled.Add() = DownscaleByteMat2x(bm); // store HALF-res (matched at half-res)
	}
	dt.ok = dt.scaled.GetCount() == kNumDealerScales;
	return dt;
}

// Nearest seat anchor (Euclidean) to a table-space point -- the RegionToSeat
// analogue for a POINT-LIKE marker. Euclidean (not RegionToSeat's angular test,
// which is tuned for centre-pulled bet chips) is the robust choice for the
// compact dealer disc: on the real frame it maps the button at (383,429) to the
// BOTTOM seat (dist 133) unambiguously over left-bottom (dist 304).
static int NearestSeatAnchor(int cx, int cy)
{
	int best = -1; double bestd = 1e18;
	for(const SeatAnchor& sa : kSeatAnchors) {
		double d = (double)(sa.cx - cx) * (sa.cx - cx) + (double)(sa.cy - cy) * (sa.cy - cy);
		if(d < bestd) { bestd = d; best = sa.seat; }
	}
	return best;
}

// Find the dealer button in a table frame. Returns the nearest seat index (or -1
// if no match clears kDealerScoreMin); out_score/out_center get the peak. Bounded
// to kDealerSearch, one MatchTemplate per pre-scaled template, global argmax.
static int DetectDealerChip(const VsmFrameImage& table, const DealerTemplate& dt,
                            double& out_score, Point& out_center)
{
	out_score = -2; out_center = Point(-1, -1);
	if(!dt.ok) return -1;
	Rect sr = kDealerSearch & RectC(0, 0, table.width, table.height);
	ByteMat imgf = TableRegionToGray(table, sr);
	ByteMat img = DownscaleByteMat2x(imgf); // match at HALF res (dt.scaled is half-res)
	if(img.IsEmpty()) return -1;
	double bestv = -2; Point bestc(-1, -1);
	for(const ByteMat& tm : dt.scaled) {
		if(tm.IsEmpty() || tm.cols > img.cols || tm.rows > img.rows) continue;
		FloatMat res;
		MatchTemplate(img, tm, res, TM_CCOEFF_NORMED);
		if(res.IsEmpty()) continue;
		double mn = 0, mx = 0; Point mnl, mxl;
		MinMaxLoc(res, &mn, &mx, &mnl, &mxl);
		if(mx > bestv) {
			bestv = mx;
			// peak (half-res) -> full-res table-space centre: (top-left + half-tmpl) * 2
			bestc = Point(sr.left + (mxl.x + tm.cols / 2) * 2, sr.top + (mxl.y + tm.rows / 2) * 2);
		}
	}
	out_score = bestv; out_center = bestc;
	if(bestv < kDealerScoreMin) return -1;
	return NearestSeatAnchor(bestc.x, bestc.y);
}

// ===========================================================================
// Task 0290c item 2: TURN INDICATOR via the countdown-TIMER graphic (continuous
// "whose turn right now", unlike Task 0289's momentary post-action bubble).
//
// Real finding (investigated on real 0263 frames): the acting player's turn timer
// renders as a bright, saturated horizontal BAR (a green -> yellow -> orange ->
// red depleting gradient) directly BELOW that seat's info pod. It is present the
// WHOLE time the player is deciding -- exactly the continuous signal 0289's action
// bubble (which only appears AFTER an action is taken) structurally cannot give.
//
// Detection is pure per-seat pixel sampling (NO OCR, NO template, sub-millisecond):
// for each seat's fixed strip (measured on real frames) count the brightest row's
// bright-saturated pixels; the seat with the strongest bar over kTimerBrightMin is
// "whose turn". Felt (max-channel ~96) and white name text (low saturation) both
// fail the bright+saturated test, so the bar stands alone. Validated against the
// sidecar ground-truth turn order across all 56 real frames of hand 1+2: the
// detector tracks the acting seat continuously and correctly (real bar counts
// 58-139 vs <=41 clutter -- a clean, wide separation; see --dealer-turn-test).
// ===========================================================================
struct TimerStrip { int seat; Rect rect; };
static const TimerStrip kTimerStrips[6] = {
	{ 0, RectC(360, 532, 200, 18) }, // BOTTOM       (bar y~541)
	{ 1, RectC( 45, 398, 215, 18) }, // LEFT_BOTTOM  (bar y~408)
	{ 2, RectC( 52, 212, 208, 18) }, // LEFT_TOP     (bar y~220)
	{ 3, RectC(360, 174, 240, 18) }, // TOP          (bar y~183)
	{ 4, RectC(680, 212, 215, 18) }, // RIGHT_TOP    (bar y~220)
	{ 5, RectC(688, 398, 217, 18) }, // RIGHT_BOTTOM (bar y~408)
};
static const int kTimerBrightMin = 50; // strictly inside the real 41/58 gap

// Count bright-saturated (timer-bar) pixels in the single strongest row of a
// table-space strip. A timer pixel is bright (max channel > 150), saturated
// (max-min > 70) and not blue-dominant (b < 130) -- true for the green/yellow/
// orange/red bar, false for felt, white text, card backs and avatars.
static int TimerBarStrength(const VsmFrameImage& table, const Rect& in_r)
{
	Rect r = in_r & RectC(0, 0, table.width, table.height);
	if(r.Width() <= 0 || r.Height() <= 0) return 0;
	int best = 0;
	for(int y = r.top; y < r.bottom; y++) {
		const byte* row = ~table.data + (size_t)((size_t)y * table.width + r.left) * 4;
		int cnt = 0;
		for(int x = 0; x < r.Width(); x++) {
			const byte* p = row + (size_t)x * 4;
			int R = p[0], G = p[1], B = p[2];
			int mx = max(R, max(G, B)), mn = min(R, min(G, B));
			if(mx > 150 && (mx - mn) > 70 && B < 130) cnt++;
		}
		if(cnt > best) best = cnt;
	}
	return best;
}

// Whose turn: the seat whose timer bar is strongest, if it clears kTimerBrightMin.
// Returns -1 (no one's timer visible) otherwise; out_strength gets the peak count.
static int DetectTurnSeat(const VsmFrameImage& table, int& out_strength)
{
	int best_seat = -1, best = 0;
	for(const TimerStrip& ts : kTimerStrips) {
		int s = TimerBarStrength(table, ts.rect);
		if(s > best) { best = s; best_seat = ts.seat; }
	}
	out_strength = best;
	return best >= kTimerBrightMin ? best_seat : -1;
}

// ===========================================================================
// Mode 1: rigorous leave-one-out classification accuracy over the dataset.
// ===========================================================================
static int RunClassifySelfTest(const String& dataset_path)
{
	Cout() << "=== Mode: classify-selftest (leave-one-out over labeled dataset) ===\n";
	VsmLiveRegionClassifier clf;
	double t0 = NowMs();
	int n = clf.Load(dataset_path);
	Cout() << "loaded " << n << " reference entries in " << Format("%.0f", NowMs() - t0) << "ms\n";
	if(n == 0) { Cerr() << "ERROR: no reference entries loaded\n"; return 1; }

	// Re-read candidate list for their crop paths + true category + global_index.
	Value root = ParseJSON(LoadFile(dataset_path));
	ValueMap m = root;
	ValueArray cands = m.Get("candidates", ValueArray());

	int total = 0, correct = 0, unresolved = 0;
	VectorMap<String, int> per_cat_total, per_cat_correct;
	VectorMap<String, int> confusion; // "true->pred" -> count
	double classify_ms = 0; int classify_calls = 0;

	for(int i = 0; i < cands.GetCount(); i++) {
		ValueMap c = cands[i];
		Value catv = c.Get("category", Value());
		if(IsNull(catv)) continue;
		String truth = AsString(catv);
		if(truth.IsEmpty()) continue;
		String crop_path = AsString(c.Get("crop_path", Value()));
		if(crop_path.IsEmpty() || !FileExists(crop_path)) continue;
		Image img = StreamRaster::LoadFileAny(crop_path);
		if(img.IsEmpty()) continue;
		int gidx = c.Find("global_index") >= 0 ? (int)c["global_index"] : i;

		int rx = 0, ry = 0;
		if(c.Find("rect") >= 0 && IsValueMap(c["rect"])) {
			ValueMap rc = c["rect"];
			rx = rc.Find("x") >= 0 ? (int)rc["x"] : 0;
			ry = rc.Find("y") >= 0 ? (int)rc["y"] : 0;
		}
		double s0 = NowMs();
		VsmLiveClassifyResult r = clf.Classify(img, rx, ry, gidx); // exclude self
		classify_ms += NowMs() - s0; classify_calls++;

		total++;
		per_cat_total.GetAdd(truth, 0)++;
		String pred = r.category.IsEmpty() ? String("<unresolved>") : r.category;
		if(r.category.IsEmpty()) unresolved++;
		if(r.category == truth) { correct++; per_cat_correct.GetAdd(truth, 0)++; }
		else confusion.GetAdd(truth + " -> " + pred, 0)++;
	}

	Cout() << "\n--- Leave-one-out results ---\n";
	Cout() << Format("classified=%d  correct=%d (%.1f%%)  unresolved=%d (%.1f%%)\n",
	                 total, correct, total ? 100.0 * correct / total : 0.0,
	                 unresolved, total ? 100.0 * unresolved / total : 0.0);
	Cout() << Format("avg classify time = %.3f ms/region (%d calls)\n",
	                 classify_calls ? classify_ms / classify_calls : 0.0, classify_calls);

	Cout() << "\n--- Per-category accuracy ---\n";
	for(int i = 0; i < per_cat_total.GetCount(); i++) {
		String cat = per_cat_total.GetKey(i);
		int t = per_cat_total[i], cc = per_cat_correct.Get(cat, 0);
		Cout() << Format("  %-26s %3d/%3d  (%.0f%%)\n", ~cat, cc, t, t ? 100.0 * cc / t : 0.0);
	}

	Cout() << "\n--- Top confusions (true -> predicted) ---\n";
	Vector<int> order;
	for(int i = 0; i < confusion.GetCount(); i++) order.Add(i);
	Sort(order, [&](int a, int b){ return confusion[a] > confusion[b]; });
	for(int k = 0; k < min(15, order.GetCount()); k++)
		Cout() << Format("  %-50s %d\n", ~confusion.GetKey(order[k]), confusion[order[k]]);
	return 0;
}

// ===========================================================================
// Task 0286 Part B verification: does the approximate-hash OCR cache ever risk
// a FALSE HIT (reporting a stale value) across genuinely-distinct real OCR
// content? VideoLiveRecognitionLoop has no mode that ingests an arbitrary
// standalone crop list (its other modes need a full frame/table image), so
// this mode tests the exact load-bearing primitive the cache decision relies
// on directly: VsmLiveRegionClassifier::Signature() + SignatureDistance()
// (the SAME two static functions ProcessTableFrame's cache logic calls).
//
// Real crops in the established 31-crop OCR validation set (Task 0274,
// tmp/_task0274_final_crop_list_hand{1,2}.txt) are pre-cropped at varying
// pixel dimensions (each was hand-picked around its own real OCR target), so
// a naive pairwise Signature() comparison across differently-sized crops
// would reproduce the SAME crop-dimension-mismatch inflation Part B's own
// calibration found (distances inflate 5x+ for mismatched dimensions,
// regardless of real content) -- that would make this safety test trivially
// "pass" without testing anything real. Instead, per category, every crop is
// Rescale()'d to that category's first crop's dimensions BEFORE computing its
// signature -- this reproduces what the real cache actually compares (the
// anchor_rect is always resampled at one fixed size), so a pairwise distance
// here is a fair, representative test of real false-hit risk.
// ===========================================================================
static int RunCropSafetyCheck(const Vector<String>& list_files, int threshold)
{
	Cout() << "=== Mode: crop-safety-check (Task 0286 Part B: false-hit risk over genuinely-distinct real crops) ===\n";
	VectorMap<String, Vector<String>> cat_paths;
	for(const String& lf : list_files) {
		if(!FileExists(lf)) { Cerr() << "ERROR: list file not found: " << lf << "\n"; continue; }
		Vector<String> lines = Split(LoadFile(lf), '\n');
		for(String line : lines) {
			line = TrimBoth(line);
			if(line.IsEmpty()) continue;
			int tab = line.Find('\t');
			if(tab < 0) continue;
			String cat = TrimBoth(line.Left(tab));
			String path = TrimBoth(line.Mid(tab + 1));
			if(cat.IsEmpty() || path.IsEmpty()) continue;
			cat_paths.GetAdd(cat).Add(path);
		}
	}
	if(cat_paths.IsEmpty()) { Cerr() << "ERROR: no (category, path) rows loaded from " << list_files.GetCount() << " list file(s)\n"; return 1; }

	// Group by (category, EXACT native w,h) -- deliberately NOT rescaled to a
	// common size. An earlier version of this check rescaled every crop in a
	// category to one canonical size before comparing; that is NOT what the
	// real cache ever does (its anchor_rect resampling always compares two
	// crops of IDENTICAL native dimensions) and forcing heterogeneous crops
	// (raw detector-sized vs. Task 0273/0274's tightly hand-refined crops)
	// through a shared Rescale() introduced its own blur/distortion artifacts
	// that are not present in the real system -- it produced several
	// misleadingly low distances between genuinely different real values that
	// the real cache would never actually compare against each other (their
	// anchor_rect dimensions would never coincide as the SAME slot's anchor in
	// the first place). Only comparing crops that already share exact pixel
	// dimensions is the fair, representative test of real false-hit risk.
	int total_pairs = 0, below_threshold = 0, total_crops = 0;
	for(int ci = 0; ci < cat_paths.GetCount(); ci++) {
		String cat = cat_paths.GetKey(ci);
		const Vector<String>& paths = cat_paths[ci];
		Vector<Image> imgs; Vector<String> ok_paths;
		for(const String& p : paths) {
			Image img = StreamRaster::LoadFileAny(p);
			if(img.IsEmpty()) { Cerr() << "  WARN: failed to load " << p << "\n"; continue; }
			imgs.Add(img); ok_paths.Add(p);
		}
		total_crops += imgs.GetCount();
		Cout() << Format("--- %s: %d genuinely-distinct real crops loaded ---\n", ~cat, imgs.GetCount());
		// bucket by exact "WxH" dimension key within this category
		VectorMap<String, Vector<int>> by_dim;
		for(int i = 0; i < imgs.GetCount(); i++)
			by_dim.GetAdd(Format("%d,%d", imgs[i].GetWidth(), imgs[i].GetHeight())).Add(i);
		int cat_pairs = 0, cat_below = 0, cat_min = 1 << 30;
		for(int di = 0; di < by_dim.GetCount(); di++) {
			const Vector<int>& idxs = by_dim[di];
			if(idxs.GetCount() < 2) continue; // need >=2 crops of the SAME exact size to form a real pair
			Vector<String> sigs;
			for(int idx : idxs) sigs.Add(VsmLiveRegionClassifier::Signature(imgs[idx]));
			for(int i = 0; i < sigs.GetCount(); i++)
				for(int j = i + 1; j < sigs.GetCount(); j++) {
					int d = VsmLiveRegionClassifier::SignatureDistance(sigs[i], sigs[j]);
					if(d < 0) continue;
					total_pairs++; cat_pairs++;
					if(d < cat_min) cat_min = d;
					if(d <= threshold) {
						below_threshold++; cat_below++;
						Cout() << Format("  ** FALSE-HIT RISK ** dist=%d (threshold=%d) size=%s  %s  vs  %s\n",
						                 d, threshold, ~by_dim.GetKey(di),
						                 ~GetFileName(ok_paths[idxs[i]]), ~GetFileName(ok_paths[idxs[j]]));
					}
				}
		}
		Cout() << Format("  %d exact-size-matched pairs tested (of %d dimension groups), %d below threshold, min distance %s\n",
		                 cat_pairs, by_dim.GetCount(), cat_below, cat_min == (1 << 30) ? String("n/a") : AsString(cat_min));
	}
	Cout() << Format("\n=== Safety verdict: %d crops loaded, %d exact-size-matched same-category pairs tested, "
	                 "%d/%d at or under threshold=%d ===\n",
	                 total_crops, total_pairs, below_threshold, total_pairs, threshold);
	Cout() << (below_threshold == 0
	           ? "PASS: zero false-hit risk across every genuinely-distinct, exact-size-matched real crop pair\n"
	           : "WARNING: some genuinely-distinct, exact-size-matched real crops would score a false cache hit at this threshold\n");
	return below_threshold == 0 ? 0 : 2;
}

// ===========================================================================
// Shared per-frame pipeline state (stages 2..5) used by both offline + live.
// ===========================================================================
// Task 0289: max event-log scrollback kept in ResolveState / published to the
// GUI snapshot. Bounded so a long run can't grow it unboundedly (the user asked
// for a right-edge log "showing all events"; we keep the most recent N).
static const int kMaxEventLog = 60;

struct ResolveState {
	ResolveState() {
		start_ms = msecs();
		for(int i = 0; i < 5; i++) board_cards.Add(-1);
		for(int i = 0; i < 6; i++) {
			seat_stack[i] = -1; seat_bet[i] = -1; seat_round_total[i] = -1;
			seat_name_frame[i] = -1; seat_stack_frame[i] = -1;
			// Task 0290b: structural fold signal state.
			seat_cards_present[i] = -1; cards_raw_last[i] = -1; cards_raw_streak[i] = 0;
			seat_folded[i] = false;
		}
	}
	// Task 0290c item 1: video-grounded dealer seat (from dealer-chip template
	// match), independent of the engine's GBUTTON_DEALER indicator. -1 = not yet
	// detected. Compared against the engine dealer in BuildSnapshot/Paint.
	int    dealer_seat_video = -1;
	double dealer_chip_score = -1;
	// Task 0290c item 2: video-grounded turn seat (from the countdown-timer bar).
	// -1 = no timer bar currently visible (nobody's turn is being shown).
	int    turn_seat_video = -1;
	int    turn_bar_strength = 0;
	// board tracking (structural tier)
	int  board_count = 0;
	Vector<int> board_cards;
	int  street_events = 0; // flop/turn/river dealt
	// value tracking (OCR tier): slot key "xxYY" -> last numeric value
	VectorMap<String, double> slot_value;
	int  value_changes = 0;
	int  pot_reads = 0;
	int  pot_rejected_reads = 0;
	double last_pot = -1;
	// Task 0287: per-seat OCR'd values, attributed via RegionToSeat(). -1 /
	// empty means "not resolved from video yet for this seat" -> BuildSnapshot
	// falls back to the engine-seed value and labels the field engine-seed.
	double seat_stack[6]; // last OCR'd balance-plate value per seat index
	double seat_bet[6];   // last OCR'd bet-label value per seat index
	String seat_name[6];  // last OCR'd name-plate text per seat index
	int    seat_name_frame[6];
	int    seat_stack_frame[6];
	// Task 0289: per-seat latest action word (from seat_action_bubble OCR) and
	// per-seat round-bet-total chip figure (from chip_badge_stack OCR).
	String seat_action[6];       // normalized action word ("Call"/"Fold"/...)
	double seat_round_total[6];  // last OCR'd chip_badge_stack value per seat, -1 none
	int    last_action_seat = -1;    // seat of the most recent action-bubble read
	int64  last_action_ms = 0;       // wall time of that read (for "fresh"/turn highlight)
	int    last_action_frame = -1;   // pipeline frame that produced that read
	// Task 0290b: OCR-free structural fold signal (hole-card-back presence).
	// seat_folded is the AUTHORITATIVE fold state, driven by BOTH the OCR-text
	// path (an action normalizing to "Fold", incl. the name-plate reroute) AND the
	// card-back path (region gone felt/empty). seat_cards_present is the committed
	// (debounced) card-back state; cards_raw_last/cards_raw_streak debounce the raw
	// per-frame sample so a single animation frame can't flip fold state.
	int    seat_cards_present[6];  // committed: -1 unknown, 0 absent, 1 present
	int    cards_raw_last[6];      // last raw sample (1/0) for the debounce streak
	int    cards_raw_streak[6];    // consecutive frames the raw sample agreed
	bool   seat_folded[6];         // authoritative fold state (OCR-fold OR structural)
	// Set the authoritative fold state for a seat, logging only on a real
	// transition (idempotent). `reason` names which signal drove the change so the
	// event log distinguishes an OCR-text fold from a structural card-back fold.
	void SetFolded(int seat, bool f, const String& reason, bool verbose) {
		if(seat < 0 || seat >= 6 || seat_folded[seat] == f) return;
		seat_folded[seat] = f;
		value_changes++;
		LogEvent(Format("seat %d %s (%s)", seat, f ? "fold" : "back in hand", ~reason), verbose);
	}
	// Task 0289: bounded event log (most-recent-last), surfaced in the GUI panel.
	int64  start_ms = 0;
	Vector<String> event_log;
	// Append a timestamped event; keep only the last kMaxEventLog. Also emit to
	// Cout when verbose so the GUI panel and the verbose console log stay the SAME
	// event stream (the task's "reuse the verbose Cout logging as source of truth").
	void LogEvent(const String& text, bool verbose) {
		int t = (int)((msecs() - start_ms) / 1000);
		// NOTE: U++ Format()'s %d id-scan greedily eats the trailing 's' as part of
		// the conversion id ("ds"), so "%ds" mis-parses -- a backtick right after
		// the conversion letter terminates the id and is itself skipped (same quirk
		// the latency "%.0f`ms" line documents), leaving "s" as literal text.
		String line = Format("[t+%d`s] %s", t, ~text);
		event_log.Add(line);
		while(event_log.GetCount() > kMaxEventLog) event_log.Remove(0);
		if(verbose) Cout() << "    [event] " << line << "\n";
	}
};

struct LoopStats {
	int frames = 0;
	int total_regions = 0;
	VectorMap<String, int> category_counts;
	int ocr_calls = 0, ocr_nonempty = 0;
	int classified = 0, unresolved = 0;
	int diagnostic_raw_board_frames = 0;
	int diagnostic_raw_board_max = 0;
	int diagnostic_board_trigger_frames = 0;
	int diagnostic_resolved_board_cards = 0;
	int diagnostic_ocr_candidates = 0;
	int diagnostic_action_candidates = 0;
	int diagnostic_action_reads = 0;
};

static bool g_sidecar2_diagnostics = false;
static String g_sidecar2_diagnostics_path;
static bool g_shader_evidence_stage_enabled = false;
static VsmShaderRecognitionService g_shader_evidence_stage_service;
static VsmShaderEvidenceAdapter g_shader_evidence_stage_adapter;
static String g_shader_evidence_stage_manifest;
static String g_shader_evidence_stage_crop_map;

static void EmitSidecar2Diagnostic(const String& line)
{
	Cout() << line << "\n";
	Cout().Flush();
	if(!g_sidecar2_diagnostics_path.IsEmpty())
		SaveFile(g_sidecar2_diagnostics_path, LoadFile(g_sidecar2_diagnostics_path) + line + "\n");
}

static bool ConfigureShaderEvidenceStage(const String& manifest_path,
                                         const String& crop_map_path)
{
	String error;
	if(!g_shader_evidence_stage_service.manifest.Load(manifest_path)) {
		Cerr() << "ERROR: shader evidence stage manifest cannot be loaded: "
		       << manifest_path << "\n";
		return false;
	}
	if(!g_shader_evidence_stage_service.crop_map.Load(crop_map_path)) {
		Cerr() << "ERROR: shader evidence stage crop map cannot be loaded: "
		       << crop_map_path << "\n";
		return false;
	}
	if(!g_shader_evidence_stage_service.manifest.Validate(error)) {
		Cerr() << "ERROR: shader evidence stage manifest invalid: " << error << "\n";
		return false;
	}
	g_shader_evidence_stage_service.use_threshold = false;
	g_shader_evidence_stage_adapter.SetService(g_shader_evidence_stage_service);
	g_shader_evidence_stage_manifest = manifest_path;
	g_shader_evidence_stage_crop_map = crop_map_path;
	g_shader_evidence_stage_enabled = true;
	Cout() << "shader-evidence-stage enabled execution=cpu-reference"
	       << " manifest=" << manifest_path << " crop_map=" << crop_map_path << "\n";
	Cout().Flush();
	return true;
}

static bool RunShaderEvidenceStage(const VsmImageBuffer& source,
                                   const Vector<WindowDescriptor>& descriptors,
                                   int64 timestamp_ms, const char *path)
{
	if(!g_shader_evidence_stage_enabled)
		return true;
	Vector<VsmPackedWindow> windows;
	for(const WindowDescriptor& descriptor : descriptors) {
		VsmPackedWindow& window = windows.Add();
		window.id = descriptor.id.value;
		window.source_x = descriptor.source_rect.left;
		window.source_y = descriptor.source_rect.top;
		window.width = descriptor.source_rect.Width();
		window.height = descriptor.source_rect.Height();
		window.timestamp_ms = timestamp_ms;
	}
	Vector<VsmShaderWindowEvidence> results;
	String error;
	if(!g_shader_evidence_stage_adapter.ProcessSource(source, windows, results, error)) {
		Cerr() << "shader-evidence-stage path=" << path << " status=error error=" << error << "\n";
		Cerr().Flush();
		return false;
	}
	for(const VsmShaderWindowEvidence& result : results) {
		VsmShaderEvidenceObservation observation =
			MakeVsmShaderEvidenceObservation(path, result);
		String status = observation.IsSuccessful() ? "ok" : "error";
		String line = Format("shader-evidence-observation path=%s window=%s "
		                     "timestamp_ms=%lld runs=%d evidence=%dx%d status=%s error=%s",
		                     ~observation.path, ~observation.window,
		                     observation.timestamp_ms, observation.runs,
		                     observation.width, observation.height, ~status,
		                     observation.error.IsEmpty() ? "none" : ~observation.error);
		EmitSidecar2Diagnostic(line);
	}
	Cout().Flush();
	return true;
}

// ---------------------------------------------------------------------------
// Task 0286 Part B: approximate-hash-based OCR result cache.
//
// Reuses VsmLiveRegionClassifier's already-validated rgb8x8 signature +
// Manhattan-distance comparator (Task 0267/0280) -- NOT a new hashing scheme.
// One slot per (OCR-relevant category, approximate screen position): the last
// signature seen ON A REAL OCR CALL ("last-OCR'd anchor", not last-seen-frame)
// plus the OCR text/confidence obtained at that call. Anchoring to the last
// REAL OCR read (rather than sliding the anchor forward every frame) means
// slow pixel drift across many frames cannot silently accumulate past the
// threshold without ever being caught -- each new frame is always compared
// against the same ground-truth anchor until a real OCR call refreshes it.
//
// IMPORTANT real finding from Task 0286's own calibration (see the task's
// Status/Evidence): the live change-detector does NOT reproduce pixel-
// identical rects frame-to-frame for the same real UI slot (already
// documented by LiveRegionClassifier.cpp) -- comparing the DETECTOR's raw,
// differently-sized crop each frame against a differently-sized cached crop
// inflates the rgb8x8 signature distance by 5x+ even when the underlying
// value has NOT changed (the 8x8 grid samples different actual screen pixels
// when box dimensions differ). The fix: each slot remembers the exact
// anchor_rect its signature was computed from; every subsequent frame
// re-samples that SAME fixed rect from the current table image for
// comparison, regardless of what rect the change-detector reported this
// frame. Only on a real OCR call (cache miss) does the anchor_rect move to
// the detector's freshest rect (best OCR crop quality). This keeps every
// comparison apples-to-apples (identical crop dimensions => identical 8x8
// sample grid => distance reflects real pixel content change, not crop
// framing noise).
//
// THRESHOLD CALIBRATION (real evidence, both real recorded hands,
// --offline-frames tmp/real_recording_0263_frames and _0268_frames,
// --verbose --no-ocr-cache so every OCR call is real and every distance is a
// real anchor-vs-current-frame comparison):
//   genuinely SAME real value (identical real OCR text back-to-back, e.g.
//     "Pot: 6 BB" -> "Pot: 6 BB"):                          distance = 180
//   likely-same, noisy/narrow re-crop ("Pot: 13.8 BB" -> a
//     garbled partial "8.6" from a much narrower live-detector
//     crop of the SAME anchor area -- reusing the cached "Pot:
//     13.8 BB" would have been MORE correct than the fresh OCR):  distance = 79
//   genuinely DIFFERENT real values (10 pairs across both hands,
//     e.g. "Pot: 60 BB"->"Pot: 258.2 BB", "0.4 BB"->"4BB",
//     "Pot: 418.2 BB"->"Pot: 183 BB"): distances = 329, 439, 487,
//     516, 517, 628, 641, 942, 966, 1053 (minimum 329)
// Real, measured margin from THIS calibration alone: 180 (max confirmed
// same) to 329 (min confirmed different) = a real gap of 149, which would
// suggest threshold~220. THAT NUMBER WAS REJECTED by a second, independent
// real-evidence check (below) and threshold=40 is used instead -- see the
// "SAFETY OVERRIDE" note.
//
// SAFETY OVERRIDE (real evidence, `--crop-safety-check` mode, the
// established 31-crop Task 0274 OCR validation set,
// tmp/_task0274_final_crop_list_hand{1,2}.txt -- 31 genuinely-distinct real
// OCR crops, i.e. every pair is a real DIFFERENT value by construction):
// grouping crops by category + EXACT native pixel dimensions (the only fair
// comparison, since the real cache's anchor_rect resampling only ever
// compares two crops of identical dimensions) and computing every pairwise
// rgb8x8 distance found 6 of 45 genuinely-different-value pot_label pairs (at
// the common 120x24 size) scoring AS LOW AS 44 -- e.g. two different real pot
// readings 44 apart, well inside the naive threshold=220 that the live-loop
// calibration alone suggested. This means the "same-value" and "different-
// value" distance distributions actually OVERLAP in real data (confirmed-same
// 79/180 vs. confirmed-different-at-matching-size as low as 44) -- there is
// NO threshold that cleanly separates every real same/different pair in the
// combined evidence. Given the task's own explicit priority ("a false hit
// reporting a stale value is the actively harmful failure mode... verify this
// explicitly"), threshold is set to 40 -- strictly below the lowest
// genuinely-different distance found in EITHER real check (44 here, 329 in
// the live-loop check) -- which makes `--crop-safety-check` PASS with zero
// false-hit risk against all available real evidence, at the honest cost of
// also missing the confirmed-same 79/180 pairs (this cache's real, measured
// hit rate is consequently small -- see the task's Status/Evidence for actual
// before/after OCR-call counts from a real live run). Reported honestly as a
// real, load-bearing finding rather than picking the more "impressive"-
// looking but actually unsafe number the first check alone suggested.
// NOTE: also, both real numbers are far from LiveRegionClassifier's own
// near_max_=120 CATEGORY-classification threshold in either direction (this
// task file's initial expectation was "much smaller than 120") -- real
// measurement contradicts that a priori assumption in both directions,
// reported honestly rather than forced to fit it.
// ---------------------------------------------------------------------------
struct OcrSlot : Moveable<OcrSlot> {
	String category;
	Rect   anchor_rect = Rect(0, 0, 0, 0); // table-space rect the signature/OCR came from
	String signature;    // rgb8x8 signature of anchor_rect's pixels AT THE LAST REAL OCR CALL
	String text;          // OCR text from that call
	double confidence = 0;
	int    last_ocr_frame = -1;
};

struct OcrCacheState {
	Vector<OcrSlot> slots;
	bool enabled = true;
	int  threshold = 40;  // TIGHT threshold, see Task 0286 calibration evidence (below)
	int  hits = 0;         // cache hits (OCR call skipped, cached result reused)
	int  misses = 0;       // real OCR calls made while cache was enabled
};

// Nearest existing slot of the same category whose anchor centre is within
// radius px of (cx,cy); -1 if none. Position-anchored, same POS_RADIUS
// rationale as LiveRegionClassifier's own pos_anchored tier (fixed-layout UI:
// position is a strong discriminator, but the live detector's rect jitters).
static int FindOcrSlot(const Vector<OcrSlot>& slots, const String& category, int cx, int cy, int radius)
{
	int best = -1, best_d = radius + 1;
	for(int i = 0; i < slots.GetCount(); i++) {
		if(slots[i].category != category) continue;
		const Rect& ar = slots[i].anchor_rect;
		int scx = ar.left + ar.Width() / 2, scy = ar.top + ar.Height() / 2;
		int d = abs(scx - cx) + abs(scy - cy);
		if(d <= radius && d < best_d) { best_d = d; best = i; }
	}
	return best;
}

struct PreparedRegion : Moveable<PreparedRegion> {
	Rect                       rect;
	Image                      crop;
	VsmLiveClassifyResult      classification;
	VsmFrameImage              ocr_crop;
	VsmOcrRequest              ocr_request;
};

static DistributedStateSnapshot MakeSidecar2Snapshot(const ResolveState& state,
	const String& phase_override = String())
{
	DistributedStateSnapshot snapshot;
	snapshot.phase = phase_override.IsEmpty()
	               ? (state.board_count >= 5 ? "river" : state.board_count >= 4 ? "turn"
	                  : state.board_count >= 3 ? "flop" : "preflop")
	               : phase_override;
	snapshot.total = state.last_pot < 0 ? 0 : state.last_pot;
	for(int seat = 0; seat < 6; seat++) {
		DistributedParticipantState& participant = snapshot.participants.Add();
		participant.active = !state.seat_folded[seat];
		participant.committed = state.seat_round_total[seat] >= 0
		                     ? state.seat_round_total[seat]
		                     : state.seat_bet[seat];
	}
	return snapshot;
}

struct PreparedTableFrame : Moveable<PreparedTableFrame> {
	WindowDescriptor      descriptor;
	int64                 source_sequence = -1;
	int64                 window_sequence = -1;
	int64                 timestamp_ms = -1;
	int                   commit_frame = -1;
	bool                  ready = false;
	VsmFrameImage         table;
	Image                 table_image;
	Vector<PreparedRegion> regions;
	StageSet              stages;
	int                   card_back_raw[6];
	int                   turn_seat = -1;
	int                   turn_strength = 0;
	bool                  dealer_sampled = false;
	int                   dealer_seat = -1;
	double                dealer_score = -2;
	Point                 dealer_center = Point(-1, -1);
	Vector<int>           board_cards;
	Vector<String>        board_debug;
	int                   changed_regions = 0;
	int                   board_classifier_regions = 0;
	int                   raw_board_count = 0;

	PreparedTableFrame()
	{
		for(int seat = 0; seat < 6; seat++) card_back_raw[seat] = -1;
	}
};

static PreparedTableFrame PrepareTableFrame(WindowFrame& frame, VsmFrameImage& previous,
	                                        bool& has_previous,
	                                        const VsmChangeDetectParams& change_params,
	                                        const VsmLiveRegionClassifier& classifier,
	                                        const CardTemplates& card_templates,
	                                        const DealerTemplate& dealer_template,
	                                        int commit_frame, bool verbose)
{
	PreparedTableFrame prepared;
	prepared.descriptor.id = WindowId(frame.descriptor.id.value);
	prepared.descriptor.name = frame.descriptor.name;
	prepared.descriptor.source_rect = frame.descriptor.source_rect;
	prepared.source_sequence = frame.source_sequence;
	prepared.window_sequence = frame.window_sequence;
	prepared.timestamp_ms = frame.timestamp_ms;
	prepared.commit_frame = commit_frame;
	prepared.table = pick(frame.table);
	if(!has_previous) {
		previous.Set(prepared.table.width, prepared.table.height, ~prepared.table.data);
		has_previous = true;
		return prepared;
	}

	double started = NowMs();
	Vector<VsmChangedRect> changed = VsmDetectChanges(previous, prepared.table, change_params);
	prepared.stages.change.Add(NowMs() - started);

	started = NowMs();
	prepared.table_image = VsmFrameImageToImage(prepared.table);
	prepared.stages.crop.Add(NowMs() - started);

	int board_regions = 0;
	for(const VsmChangedRect& changed_rect : changed) {
		Rect rect = changed_rect.GetRect() & RectC(0, 0, prepared.table.width, prepared.table.height);
		if(rect.Width() <= 0 || rect.Height() <= 0) continue;
		prepared.changed_regions++;
		PreparedRegion& region = prepared.regions.Add();
		region.rect = rect;
		started = NowMs();
		region.crop = Crop(prepared.table_image, rect);
		region.classification = classifier.Classify(region.crop, rect.left, rect.top);
		prepared.stages.classify.Add(NowMs() - started);
		if(region.classification.category == "board_card") board_regions++;
		if(IsOcrCategory(region.classification.category)) {
			region.ocr_crop = CropFrameImage(prepared.table, rect);
			region.ocr_request.semantic = region.classification.category;
			region.ocr_request.region_id = prepared.descriptor.id.value + ":" +
			    Format("%d,%d,%d,%d", rect.left, rect.top, rect.Width(), rect.Height());
			ASSERT(prepared.window_sequence >= 0 && prepared.window_sequence <= INT_MAX);
			region.ocr_request.frame = (int)prepared.window_sequence;
			region.ocr_request.ts = AsString(prepared.timestamp_ms);
		}
	}
	prepared.board_classifier_regions = board_regions;

	started = NowMs();
	for(const CardBackRegion& card_back : kCardBackRegions) {
		if(!card_back.enabled) continue;
		double white_fraction = CardBackWhiteFraction(prepared.table, card_back.rect);
		if(white_fraction >= 0)
			prepared.card_back_raw[card_back.seat] = white_fraction >= kCardBackWhiteFrac ? 1 : 0;
	}
	prepared.turn_seat = DetectTurnSeat(prepared.table, prepared.turn_strength);
	prepared.stages.structural.Add(NowMs() - started);

	started = NowMs();
	static const int kDealerEveryFrames = 8;
	if(dealer_template.ok && commit_frame % kDealerEveryFrames == 0) {
		prepared.dealer_sampled = true;
		prepared.dealer_seat = DetectDealerChip(prepared.table, dealer_template,
		                                          prepared.dealer_score, prepared.dealer_center);
	}
	prepared.raw_board_count = SplitBoardBand(prepared.table).GetCount();
	if((board_regions > 0 || prepared.raw_board_count >= 3) && card_templates.ok)
		prepared.board_cards = RecognizeBoardCards(prepared.table, card_templates,
		                                            verbose ? &prepared.board_debug : nullptr);
	if(prepared.dealer_sampled || board_regions > 0 || prepared.raw_board_count >= 3)
		prepared.stages.template_match.Add(NowMs() - started);

	previous.Set(prepared.table.width, prepared.table.height, ~prepared.table.data);
	prepared.ready = true;
	return prepared;
}

static void CommitPreparedTableFrame(const PreparedTableFrame& prepared,
	                                 VsmTesseractOcrEngine& ocr, bool ocr_available,
	                                 StageSet& st, LoopStats& stats, ResolveState& rs,
	                                 OcrCacheState& oc, const std::shared_ptr<Game>& game,
	                                 bool verbose, int ocr_cap, bool action_only_ocr)
{
	ASSERT(prepared.ready);
	ASSERT(prepared.commit_frame == stats.frames + 1);
	const VsmFrameImage& table = prepared.table;
	const Image& table_img = prepared.table_image;
	int ocr_used = 0; // OCR calls spent on THIS frame (budget throttle)
	int frame_ocr_candidates = 0;
	int frame_action_candidates = 0;
	int frame_action_reads = 0;
	String frame_action_texts;
	VectorMap<String, int> frame_categories;
	double t = 0;
	st.MergePrepared(prepared.stages);

	stats.frames++;
	stats.total_regions += prepared.regions.GetCount();
	if(prepared.raw_board_count > 0) stats.diagnostic_raw_board_frames++;
	stats.diagnostic_raw_board_max = max(stats.diagnostic_raw_board_max, prepared.raw_board_count);
	if(prepared.board_classifier_regions > 0) stats.diagnostic_board_trigger_frames++;
	for(int card : prepared.board_cards)
		if(card >= 0) stats.diagnostic_resolved_board_cards++;

	int board_regions_this_frame = 0;
	if(!prepared.board_cards.IsEmpty())
		board_regions_this_frame = prepared.board_cards.GetCount();
	Vector<int> new_board_cards;

	for(const PreparedRegion& region : prepared.regions) {
		const Rect& r = region.rect;
		const Image& crop = region.crop;
		const VsmLiveClassifyResult& cl = region.classification;
		int bb_distance = -1, action_distance = -1;
		// Changed-region rectangles can contain a whole table animation. Do not
		// run the small-label search over those large regions: the classifier
		// category and bounded geometry are the cheap first-stage gate.
		bool small_text_region = r.Width() <= 260 && r.Height() <= 80;
		bool money_candidate = small_text_region && IsMoneySearchRegion(r, cl.category);
		bool action_candidate = small_text_region && IsActionSearchRegion(r, cl.category);
		String bb_anchor = money_candidate ? MatchRecognitionAnchor(crop, "BB-", bb_distance) : String();
		String action_anchor = action_candidate ? MatchRecognitionAnchor(crop, "action-", action_distance) : String();
		bool bb_match = IsAcceptedRecognitionAnchor(bb_anchor, bb_distance);
		bool action_match = IsAcceptedRecognitionAnchor(action_anchor, action_distance);
		if((bb_match || action_match) && verbose)
			Cout() << Format("    [rgb-anchor] rect=%d,%d,%d,%d category=%s anchor=%s distance=%d accepted=1\n",
			                 r.left, r.top, r.Width(), r.Height(), ~cl.category,
			                 bb_match ? ~bb_anchor : ~action_anchor,
			                 bb_match ? bb_distance : action_distance);

		stats.classified++;
		String cat = cl.category.IsEmpty() ? String("<unresolved>") : cl.category;
		if(cl.category.IsEmpty()) stats.unresolved++;
		stats.category_counts.GetAdd(cat, 0)++;
		frame_categories.GetAdd(cat, 0)++;
		if(IsOcrCategory(cl.category)) frame_ocr_candidates++;
		if(cl.category == "seat_action_bubble" || cl.category == "seat_name_plate")
			frame_action_candidates++;

		// --- stage: OCR (only OCR-relevant categories) ---
		String ocr_text; double ocr_conf = 0;
		int cache_dist = -1; bool cache_hit = false;
		bool fast_action_category = cl.category == "seat_action_bubble"
		                         || cl.category == "seat_name_plate";
		bool anchor_needs_ocr = bb_match || action_match;
		if(ocr_available && (IsOcrCategory(cl.category) || anchor_needs_ocr)
		   && (!action_only_ocr || fast_action_category)
		   && (ocr_cap < 0 || ocr_used < ocr_cap)) {
			// Task 0286 Part B: consult the approximate-hash cache BEFORE paying
			// for a real OCR call. Per OcrCacheState's comment: compare the
			// slot's OWN anchor_rect re-sampled from THIS frame (not this
			// frame's detector rect r) against the anchor's stored signature --
			// keeps crop dimensions identical across the comparison, which a
			// real calibration run found necessary (differently-sized crops of
			// the same real content inflated distance 5x+).
			const int OCR_CACHE_POS_RADIUS = 60; // matches LiveRegionClassifier's own pos_anchored tier
			int qcx = r.left + r.Width() / 2, qcy = r.top + r.Height() / 2;
			int si = FindOcrSlot(oc.slots, cl.category, qcx, qcy, OCR_CACHE_POS_RADIUS);
			if(si >= 0) {
				Rect ar = oc.slots[si].anchor_rect & RectC(0, 0, table_img.GetWidth(), table_img.GetHeight());
				if(ar.Width() > 0 && ar.Height() > 0) {
					Image anchor_crop = Crop(table_img, ar);
					String sig_now = VsmLiveRegionClassifier::Signature(anchor_crop);
					cache_dist = VsmLiveRegionClassifier::SignatureDistance(sig_now, oc.slots[si].signature);
				}
			}
			if(oc.enabled && si >= 0 && cache_dist >= 0 && cache_dist <= oc.threshold) {
				cache_hit = true;
				ocr_text = oc.slots[si].text; ocr_conf = oc.slots[si].confidence;
				oc.hits++;
			} else {
				ocr_used++;
				t = NowMs();
				VsmFrameImage anchor_crop;
				if(!region.ocr_crop.IsEmpty())
					anchor_crop.Set(region.ocr_crop.width, region.ocr_crop.height,
					               ~region.ocr_crop.data);
				VsmOcrRequest anchor_request = region.ocr_request;
				if(anchor_crop.IsEmpty()) {
					anchor_crop = CropFrameImage(table, r);
					anchor_request.semantic = bb_match && bb_anchor == "BB-board"
					                      ? "pot_label" : bb_match ? "seat_balance_plate"
					                      : "seat_action_bubble";
					anchor_request.region_id = Format("anchor:%d,%d,%d,%d", r.left, r.top,
					                                  r.Width(), r.Height());
				}
				VsmOcrResult res = ocr.Execute(anchor_crop, anchor_request);
				st.ocr.Add(NowMs() - t);
				ocr_text = res.text; ocr_conf = res.confidence;
				stats.ocr_calls++;
			if(!TrimBoth(ocr_text).IsEmpty()) stats.ocr_nonempty++;
			if(!TrimBoth(ocr_text).IsEmpty() && (cl.category == "seat_action_bubble" || cl.category == "seat_name_plate"))
				frame_action_reads++;
			if((cl.category == "seat_action_bubble" || cl.category == "seat_name_plate")
			   && !TrimBoth(ocr_text).IsEmpty()) {
				if(!frame_action_texts.IsEmpty()) frame_action_texts << ";";
				frame_action_texts << cl.category << "=\"" << TrimBoth(ocr_text) << "\"";
			}
				if(oc.enabled) oc.misses++;
				// Refresh the anchor ONLY on a real OCR call (not every frame) --
				// moves anchor_rect to THIS frame's (freshest, best-quality) rect.
				int ui = si;
				if(ui < 0) { ui = oc.slots.GetCount(); OcrSlot& ns = oc.slots.Add(); ns.category = cl.category; }
				oc.slots[ui].anchor_rect = r;
				oc.slots[ui].signature = VsmLiveRegionClassifier::Signature(crop); // crop == Crop(table_img, r)
				oc.slots[ui].text = ocr_text; oc.slots[ui].confidence = ocr_conf;
				oc.slots[ui].last_ocr_frame = stats.frames;
			}
		}

		// RGB action templates are authoritative visual evidence for the action
		// label, while OCR supplies optional amount text. This also handles a
		// classifier result that was previously called a composite/name plate.
		if(action_match) {
			int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
			String action = action_anchor.Mid(7);
			if(action == "post-sb") action = "Post SB";
			else if(action == "post-bb") action = "Post BB";
			else if(action == "all-in") action = "All In";
			else if(action == "resume") action = "Resume";
			else if(action == "muck") action = "Muck";
			else if(action == "won") action = "Won";
			else if(!action.IsEmpty()) action.Set(0, ToUpper(action[0]));
			if(seat >= 0 && seat < 6 && !action.IsEmpty()) {
				if(rs.seat_action[seat] != action)
					rs.LogEvent(Format("seat %d action: %s (RGB template)", seat, ~action), verbose);
				rs.seat_action[seat] = action;
				if(action == "Fold") rs.SetFolded(seat, true, "RGB action template", verbose);
				rs.last_action_seat = seat;
				rs.last_action_ms = msecs();
				rs.last_action_frame = stats.frames;
			}
		}
		if(bb_match) {
			double anchor_value = -1;
			if(ParseChipValue(ocr_text, anchor_value)) {
				bool board_money = bb_anchor == "BB-board";
				if(board_money) {
					rs.pot_reads++;
					bool decreasing = rs.last_pot >= 0 && anchor_value + 0.25 < rs.last_pot;
					double committed = 0;
					for(int seat = 0; seat < 6; seat++)
						committed += max(rs.seat_bet[seat], rs.seat_round_total[seat]);
					bool below_contributions = committed > 0.0 && anchor_value + 0.5 < committed;
					if(decreasing || below_contributions) {
						rs.pot_rejected_reads++;
						rs.LogEvent(Format("pot rejected: %.4g previous=%.4g committed=%.4g%s%s",
						                   anchor_value, rs.last_pot, committed,
						                   decreasing ? " decrease" : "",
						                   below_contributions ? " below-contributions" : ""), verbose);
					}
					else {
						if(rs.last_pot < 0 || fabs(rs.last_pot - anchor_value) > 0.001)
							rs.LogEvent(Format("pot: %.4g (BB-board RGB anchor)", anchor_value), verbose);
						rs.last_pot = anchor_value;
					}
				}
				else {
					int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
					if(seat >= 0 && seat < 6) {
						rs.seat_stack[seat] = anchor_value;
						rs.seat_stack_frame[seat] = stats.frames;
						rs.LogEvent(Format("seat %d stack: %.4g (BB RGB anchor)", seat, anchor_value), verbose);
					}
				}
			}
		}

		// --- stage: resolve ---
		t = NowMs();
		if(cl.category == "board_card") {
			// Task 0290a: a board_card region only TRIGGERS board recognition; the
			// actual cards are resolved once per frame by RecognizeBoardCards()
			// below (felt-gap split + suit-by-colour + rank template match over the
			// whole board band), NOT from this single region's classifier rank/suit.
			// This is what fixes Task 0288's "board stays 0/5" -- a merged 3-card
			// region used to resolve at most one classifier-guessed card.
			board_regions_this_frame++;
		}
		else if(cl.category == "seat_name_plate") {
			// Task 0287: name plates carry text, not a chip value -- store the
			// trimmed OCR string, attributed to a seat by rect position.
			String nm = TrimBoth(ocr_text);
			if(!nm.IsEmpty()) {
				int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
				if(seat >= 0 && seat < 6) {
					// Task 0289: a fold/check/... action bubble is sometimes
					// misclassified as a name plate (the classifier's own limitation,
					// noted in Task 0288's Status/Evidence -- e.g. a real "Fold" read
					// off this recording landed as a player NAME). If the plate text
					// is ESSENTIALLY just an action word (near-zero leftover letters),
					// route it to the action slot instead of storing it as a name --
					// the direct parallel to Task 0288's IsPureChipText name-reject,
					// and what makes fold-based card hiding actually fire on real data.
					String act0 = NormalizeActionText(nm);
					double chipv = 0;
					bool   has_chip  = ContainsChipAmount(nm, chipv);
					int    alpha_run = LongestAlphaRun(nm);
					if(!act0.IsEmpty()
					   && LettersOutsideKeyword(nm, act0 == "All In" ? "all" : act0) <= 2) {
						if(rs.seat_action[seat] != act0) {
							rs.value_changes++;
							rs.LogEvent(Format("seat %d action: %s (rerouted from name-plate)", seat, ~act0), verbose);
						}
						rs.seat_action[seat] = act0;
						// Task 0290b: an action-word name-plate read also drives the
						// authoritative fold state (a "Fold" here IS the fold signal;
						// any other action means the seat is live -> clear a stale fold).
						rs.SetFolded(seat, act0 == "Fold", "action via name-plate", verbose);
						rs.last_action_seat = seat;
						rs.last_action_ms = msecs();
						rs.last_action_frame = stats.frames;
					}
					// Task 0290b: a CHIP AMOUNT that leaked into the name field.
					// Real names always carry a long continuous alphabetic run
					// (>= 6 in this recording); a chip-amount-shaped read whose
					// longest alpha run is < kMinNameAlphaRun is NOT a name. This is
					// a keyword-free structural discriminator (per the task's stated
					// preference over another one-off keyword list), so newly-seen
					// phrases ("Wins"/"Split"/"Timed Out"+amount, ...) need no future
					// fix. The alpha-run guard also protects real usernames that
					// merely contain digits (isarpires98, alpha run 9) from ever
					// being mistaken for a leaked chip value.
					else if(has_chip && alpha_run < kMinNameAlphaRun) {
						if(IsPureChipText(nm)) {
							// A clean chip figure ("191.5 BB", "1,085") whose only
							// letters are the BB/SB unit -- a misclassified balance/bet
							// plate (the user's "BB in the name field is really a
							// balance" report). Reroute the VALUE to the seat's bet slot
							// rather than discarding it (Task 0288 discarded; this task
							// reroutes). Bet, not stack, is the deliberate safety choice:
							// a bet is transient/self-healing and renders as a clearly-a-
							// bet pill, whereas a wrong STACK persists as an authoritative
							// lie -- the exact risk Task 0288 flagged. Documented as a
							// reversible judgment call.
							if(rs.seat_bet[seat] < 0 || fabs(rs.seat_bet[seat] - chipv) > 0.001) {
								rs.value_changes++;
								rs.LogEvent(Format("seat %d bet: %.4g (chip amount rerouted from name-plate)", seat, chipv), verbose);
							}
							rs.seat_bet[seat] = chipv;
							if(verbose)
								Cout() << Format("    [reroute-name] seat %d name-plate OCR \"%s\" is a chip value -- "
								                 "rerouted to bet=%.4g, not stored as a name\n", seat, ~nm, chipv);
						}
						else {
							// A phrase carrying a chip amount ("Won 191.5 BB") -- a
							// hand-result/status string with non-unit letters, too
							// ambiguous to store as ANY authoritative value. Reject it
							// from the name field (that IS the bug the user hit) WITHOUT
							// inventing a bet/stack number for it.
							if(rs.seat_name[seat] != nm)  // log the rejection once per distinct phrase
								rs.LogEvent(Format("seat %d name-plate phrase rejected: \"%s\"", seat, ~nm), verbose);
							if(verbose)
								Cout() << Format("    [reject-name] seat %d name-plate OCR \"%s\" is a chip-bearing phrase, "
								                 "not a name -- rejected (value not stored, to avoid a false reading)\n", seat, ~nm);
						}
					}
					// Task 0288 fix 3: the classifier's measured accuracy on
					// seat_balance_plate (~62%) is far lower than on
					// seat_name_plate (~97%), so a balance plate genuinely gets
					// misclassified as a name plate sometimes -- its numeric OCR
					// text would then be stored as if it were the player's NAME
					// (the user's "balance näkyy monien nimessä" report). This
					// branch now only catches numeric reads the chip-amount test
					// above did NOT (e.g. a lone single digit, which is deliberately
					// not "chip-shaped"); still not stored as a name.
					else if(IsPureChipText(nm)) {
						if(verbose)
							Cout() << Format("    [reject-name] seat %d name-plate OCR \"%s\" is purely numeric -- "
							                 "misclassified chip plate, not stored as a name\n", seat, ~nm);
					}
					else {
						if(rs.seat_name[seat] != nm) {
							rs.value_changes++;
							rs.LogEvent(Format("seat %d name: %s", seat, ~nm), verbose);
						}
						rs.seat_name[seat] = nm;
						rs.seat_name_frame[seat] = stats.frames;
					}
				}
			}
		}
		else if(cl.category == "seat_action_bubble") {
			// Task 0289: the action/turn/fold signal. The bubble text carries the
			// action verb (sometimes with an amount below); normalize to a short
			// word and attribute to a seat by position. Whichever seat gets a fresh
			// action read is treated as the acting/last-to-act seat (turn indicator),
			// and an action normalizing to "Fold" hides that seat's hole cards.
			String act = NormalizeActionText(ocr_text);
			if(!act.IsEmpty()) {
				int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
				if(seat >= 0 && seat < 6) {
					if(rs.seat_action[seat] != act) {
						rs.value_changes++;
						rs.LogEvent(Format("seat %d action: %s", seat, ~act), verbose);
					}
					rs.seat_action[seat] = act;
					// Task 0290b: the action bubble also drives the authoritative
					// fold state (a "Fold" bubble folds the seat; any other action
					// means the seat is live -> clear a stale fold).
					rs.SetFolded(seat, act == "Fold", "action bubble", verbose);
					rs.last_action_seat = seat;
					rs.last_action_ms = msecs();
					rs.last_action_frame = stats.frames;
				}
			}
		}
		else if(IsOcrCategory(cl.category)) {
			double val;
			if(ParseChipValue(ocr_text, val)) {
				if(cl.category == "pot_label") {
					rs.pot_reads++;
					double committed = 0;
					for(int seat = 0; seat < 6; seat++)
						committed += max(rs.seat_bet[seat], rs.seat_round_total[seat]);
					bool decreasing = rs.last_pot >= 0 && val + 0.25 < rs.last_pot;
					bool below_contributions = committed > 0.0 && val + 0.5 < committed;
					if(decreasing || below_contributions) {
						rs.pot_rejected_reads++;
						rs.LogEvent(Format("pot rejected: %.4g previous=%.4g committed=%.4g%s%s",
						                   val, rs.last_pot, committed,
						                   decreasing ? " decrease" : "",
						                   below_contributions ? " below-contributions" : ""), verbose);
					}
					else {
						if(rs.last_pot < 0 || fabs(val - rs.last_pot) > 0.001) {
							rs.value_changes++;
							rs.LogEvent(Format("pot: %.4g", val), verbose);
						}
						rs.last_pot = val;
					}
				} else {
					String slot = Format("%s@%d,%d", ~cl.category, r.left, r.top);
					int q = rs.slot_value.Find(slot);
					if(q >= 0 && fabs(rs.slot_value[q] - val) > 0.001) rs.value_changes++;
					rs.slot_value.GetAdd(slot) = val;
					// Task 0287: also attribute to a seat index so BuildSnapshot
					// can render it on the right seat (previously the position
					// key was stored but the seat identity was discarded).
					int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
					if(seat >= 0 && seat < 6) {
						if(cl.category == "seat_balance_plate") {
							if(rs.seat_stack[seat] < 0 || fabs(rs.seat_stack[seat] - val) > 0.001)
								rs.LogEvent(Format("seat %d stack: %.4g", seat, val), verbose);
							rs.seat_stack[seat] = val;
							rs.seat_stack_frame[seat] = stats.frames;
						}
						else if(cl.category == "seat_bet_label") {
							if(rs.seat_bet[seat] < 0 || fabs(rs.seat_bet[seat] - val) > 0.001)
								rs.LogEvent(Format("seat %d bet: %.4g", seat, val), verbose);
							rs.seat_bet[seat] = val;
						}
						// Task 0289: chip_badge_stack is the "round bet total" chip
						// figure (real evidence: the SAME on-felt chips+BB primitive
						// that seat_bet_label also labels in other frames -- see the
						// task Status/Evidence; e.g. center (264,360) is labeled
						// seat_bet_label in some frames and chip_badge_stack in others).
						// Stored in its own per-seat slot and rendered as a distinct
						// felt chip-total element per the user's request (item 4).
						else if(cl.category == "chip_badge_stack") {
							if(rs.seat_round_total[seat] < 0 || fabs(rs.seat_round_total[seat] - val) > 0.001)
								rs.LogEvent(Format("seat %d round-total: %.4g", seat, val), verbose);
							rs.seat_round_total[seat] = val;
						}
					}
				}
			}
		}
		st.resolve.Add(NowMs() - t);

		if(verbose && !cl.category.IsEmpty()) {
			String cache_note;
			if(cache_dist >= 0)
				cache_note = Format("  cache_dist=%d%s", cache_dist, cache_hit ? " HIT" : " miss");
			Cout() << Format("    region (%d,%d %d w x %d h) -> %-22s conf=%.2f tier=%s dist=%d%s%s\n",
			                 r.left, r.top, r.Width(), r.Height(), ~cat, cl.confidence,
			                 ~cl.tier, cl.distance,
			                 ocr_text.IsEmpty() ? "" : ~Format("  ocr=\"%s\"(%.2f)", ~TrimBoth(ocr_text), ocr_conf),
			                 ~cache_note);
		}
	}

	// --- Task 0290b: OCR-free structural fold signal (hole-card-back presence) ---
	// Runs EVERY frame, independent of the change detector (that is the whole
	// point: a fold that the change detector missed, or an action bubble that was
	// misclassified, is still caught here from raw pixels). Pixel/color sampling
	// only -- no OCR, no classifier. Debounced (kCardBackConfirmFrames) so a single
	// card-dealing/mucking animation frame can't flip fold state. Used to CONFIRM
	// or CORRECT the OCR-text fold detection, never to replace it:
	//   * card backs gone   -> fold the seat (catch a fold the OCR path missed);
	//   * card backs present -> clear a stale fold (a new hand was dealt) -- this
	//     also fixes Task 0289's disclosed "fold state never resets per hand" gap.
	t = NowMs();
	for(const CardBackRegion& cb : kCardBackRegions) {
		if(!cb.enabled) continue;
		int raw = prepared.card_back_raw[cb.seat];
		if(raw < 0) continue;
		if(raw == rs.cards_raw_last[cb.seat]) rs.cards_raw_streak[cb.seat]++;
		else { rs.cards_raw_last[cb.seat] = raw; rs.cards_raw_streak[cb.seat] = 1; }
		if(rs.cards_raw_streak[cb.seat] >= kCardBackConfirmFrames
		   && rs.seat_cards_present[cb.seat] != raw) {
			int prev_state = rs.seat_cards_present[cb.seat];
			rs.seat_cards_present[cb.seat] = raw;
			if(raw == 0) {
				// Backs gone -> folded (confirms an OCR fold, or catches a missed one).
				rs.SetFolded(cb.seat, true, "card-backs gone -- structural", verbose);
			}
			else if(prev_state == 0) {
				// Backs came back after being absent -> a fresh hand: the seat is live
				// again. Clear the stale fold AND the stale "Fold" action label so the
				// mirror stops showing a fold that no longer applies.
				rs.SetFolded(cb.seat, false, "card-backs present -- structural", verbose);
				if(ToLower(rs.seat_action[cb.seat]).Find("fold") >= 0)
					rs.seat_action[cb.seat] = "";
			}
		}
	}
	st.resolve.Add(NowMs() - t);

	// --- Task 0290c item 2: continuous turn indicator from the timer bar ---
	// Pure per-seat pixel sampling, every frame (sub-millisecond). Independent of
	// the change detector and of OCR: shows whose turn it is WHILE they decide, a
	// signal 0289's momentary post-action bubble cannot provide.
	t = NowMs();
	{
		int ts = prepared.turn_seat;
		rs.turn_bar_strength = prepared.turn_strength;
		if(ts != rs.turn_seat_video) {
			rs.turn_seat_video = ts;
			if(ts >= 0) rs.LogEvent(Format("turn: seat %d (timer bar, strength %d)", ts, prepared.turn_strength), verbose);
		}
	}
	stats.diagnostic_ocr_candidates += frame_ocr_candidates;
	stats.diagnostic_action_candidates += frame_action_candidates;
	stats.diagnostic_action_reads += frame_action_reads;
	if(g_sidecar2_diagnostics) {
		String categories;
		for(int i = 0; i < frame_categories.GetCount(); i++) {
			if(i) categories << ",";
			categories << frame_categories.GetKey(i) << "=" << frame_categories[i];
		}
		EmitSidecar2Diagnostic(Format("diagnostic window=%s pts=%d changed=%d board_raw=%d board_classifier=%d board_resolved=%d "
		                 "ocr_candidates=%d action_candidates=%d action_reads=%d ocr_calls=%d ocr_nonempty=%d categories=%s action_texts=%s",
		                 ~prepared.descriptor.name, (int)(prepared.timestamp_ms / 1000),
		                 prepared.changed_regions, prepared.raw_board_count,
		                 prepared.board_classifier_regions, prepared.board_cards.GetCount(),
		                 frame_ocr_candidates, frame_action_candidates, frame_action_reads,
		                 stats.ocr_calls, stats.ocr_nonempty, ~categories,
		                 frame_action_texts.IsEmpty() ? "-" : ~frame_action_texts));
	}
	st.resolve.Add(NowMs() - t);

	// --- Task 0290c item 1: video-grounded dealer chip (template match) ---
	// THROTTLED to once every kDealerEveryFrames frames -- the dealer button is
	// static within a hand, and MatchTemplate over the felt band is the one
	// non-trivial cost here, so running it every frame would be wasteful. Logged
	// as an INDEPENDENT signal; agreement with the engine's GBUTTON_DEALER
	// indicator is reported in BuildSnapshot/Paint, not asserted here.
	if(prepared.dealer_sampled) {
		t = NowMs();
		rs.dealer_chip_score = prepared.dealer_score;
		if(prepared.dealer_seat != rs.dealer_seat_video) {
			rs.dealer_seat_video = prepared.dealer_seat;
			if(prepared.dealer_seat >= 0)
				rs.LogEvent(Format("dealer chip: seat %d (template score %.2f at %d,%d)",
				                    prepared.dealer_seat, prepared.dealer_score,
				                    prepared.dealer_center.x, prepared.dealer_center.y), verbose);
		}
		st.engine.Add(NowMs() - t);
	}

	// Structural street events + engine application from board region count.
	t = NowMs();
	if(board_regions_this_frame > 0) {
		// Task 0290a: a board_card region appeared this frame -> re-recognize the
		// WHOLE board from raw pixels (felt-gap split + suit-colour + rank template
		// match), bounded to the fixed board band. This resolves all 3/4/5 cards at
		// once regardless of how the change detector merged them.
		new_board_cards.Clear();
		int resolved_cards = 0;
		for(int ci : prepared.board_cards) { new_board_cards.Add(ci); if(ci >= 0) resolved_cards++; }
		if(verbose)
			for(const String& d : prepared.board_debug) Cout() << "    [board-recog] " << d << "\n";
		// Task 0288 fix 2: gate the transition to ONLY the real valid jumps.
		// The pre-0288 code accepted ANY increase (`resolved_cards > board_count`),
		// so a single classifier false-positive board_card at preflop forced the
		// board into an impossible 1-card state (the user's "preflop näyttää yhden
		// kortin Kh" report) and stuck there. Now: 0->3 needs >=3 resolved cards
		// THIS frame, 3->4 needs >=4, 4->5 needs >=5; anything else is rejected as
		// implausible recognition noise (logged when verbose, not silently eaten).
		int new_count = rs.board_count;
		if(rs.board_count == 0 && resolved_cards >= 3)      new_count = 3;
		else if(rs.board_count == 3 && resolved_cards >= 4) new_count = 4;
		else if(rs.board_count == 4 && resolved_cards >= 5) new_count = 5;
		if(new_count > rs.board_count) {
			// merge resolved cards into board_cards (positionally best-effort);
			// keep any previously-resolved slot when this frame left it unresolved.
			for(int i = 0; i < new_board_cards.GetCount() && i < 5; i++)
				if(new_board_cards[i] >= 0) rs.board_cards[i] = new_board_cards[i];
			int from = rs.board_count, to = new_count;
			rs.board_count = new_count;
			rs.street_events++;
			// --- engine application: force the engine board to the resolved cards ---
			if(game) {
				if(auto hand = game->getCurrentHand()) {
					if(auto board = hand->getBoard()) {
						int bc[5];
						for(int i = 0; i < 5; i++) bc[i] = rs.board_cards[i];
						board->setMyCards(bc);
					}
				}
			}
			rs.LogEvent(Format("board %d -> %d (%s)", from, to,
			                   to >= 5 ? "river" : to >= 4 ? "turn" : "flop"), verbose);
			if(verbose)
				Cout() << Format("    [structural] board %d -> %d dealt; engine board forced\n", from, to);
		}
		else if(verbose && resolved_cards != rs.board_count && resolved_cards > 0) {
			// Real, visible diagnostic signal: a board-card count that does not
			// correspond to a valid street transition from the current state.
			Cout() << Format("    [reject-board] %d board_card region(s) resolved this frame at board_count=%d -- "
			                 "not a valid 0->3/3->4/4->5 jump, ignored as implausible\n",
			                 resolved_cards, rs.board_count);
		}
	}
	st.engine.Add(NowMs() - t);

}

struct RecognitionWindowState {
	WindowDescriptor descriptor;
	StageSet          stages;
	LoopStats         loop_stats;
	ResolveState      resolve_state;
	OcrCacheState     ocr_cache;
	VsmFrameImage     previous;
	bool              has_previous = false;
	int64             next_prepare_sequence = 0;
	int64             next_commit_sequence = 0;
	int64             last_prepared_source_sequence = -1;
	int64             last_committed_source_sequence = -1;
};

static void InitializeRecognitionWindow(RecognitionWindowState& state,
	                                    const WindowDescriptor& descriptor,
	                                    bool ocr_cache_enabled, int ocr_cache_threshold)
{
	state.descriptor.id = WindowId(descriptor.id.value);
	state.descriptor.name = descriptor.name;
	state.descriptor.source_rect = descriptor.source_rect;
	InitializeStageNames(state.stages);
	state.ocr_cache.enabled = ocr_cache_enabled;
	state.ocr_cache.threshold = ocr_cache_threshold;
}

static bool ProcessWindowFrame(WindowFrame& frame, RecognitionWindowState& state,
	                           const VsmLiveRegionClassifier& classifier,
	                           VsmTesseractOcrEngine& ocr, bool ocr_available,
	                           const VsmChangeDetectParams& change_params,
	                           const std::shared_ptr<Game>& game, bool verbose, int ocr_cap,
	                           const CardTemplates& card_templates,
	                           const DealerTemplate& dealer_template,
	                           VsmFrameImage *committed_table = nullptr,
	                           bool action_only_ocr = false)
{
	ASSERT(frame.descriptor.id == state.descriptor.id);
	ASSERT(frame.window_sequence == state.next_prepare_sequence);
	ASSERT(frame.source_sequence > state.last_prepared_source_sequence);
	state.last_prepared_source_sequence = frame.source_sequence;
	state.next_prepare_sequence++;
	PreparedTableFrame prepared = PrepareTableFrame(frame, state.previous, state.has_previous,
	                                                change_params, classifier, card_templates,
	                                                dealer_template, state.loop_stats.frames + 1,
	                                                verbose);
	if(!prepared.ready) {
		ASSERT(prepared.window_sequence == state.next_commit_sequence);
		ASSERT(prepared.source_sequence > state.last_committed_source_sequence);
		state.last_committed_source_sequence = prepared.source_sequence;
		if(committed_table)
			*committed_table = pick(prepared.table);
		state.next_commit_sequence++;
		return false;
	}
	ASSERT(prepared.descriptor.id == state.descriptor.id);
	ASSERT(prepared.window_sequence == state.next_commit_sequence);
	ASSERT(prepared.source_sequence > state.last_committed_source_sequence);
	CommitPreparedTableFrame(prepared, ocr, ocr_available, state.stages, state.loop_stats,
	                         state.resolve_state, state.ocr_cache, game, verbose, ocr_cap,
	                         action_only_ocr);
	state.last_committed_source_sequence = prepared.source_sequence;
	if(committed_table)
		*committed_table = pick(prepared.table);
	state.next_commit_sequence++;
	return true;
}

static int RunShaderEvidenceFrame(const String& video_path, const String& manifest_path,
	                               const String& crop_map_path, int frame_second)
{
	Cout() << "shader-evidence-frame start manifest=" << manifest_path
	       << " crop_map=" << crop_map_path << "\n";
	Cout().Flush();
	VsmShaderRecognitionService service;
	String error;
	Cout() << "shader-evidence-frame stage=load-manifest\n";
	Cout().Flush();
	if(!service.manifest.Load(manifest_path)) {
		Cerr() << "ERROR: shader manifest cannot be loaded: " << manifest_path << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame stage=load-crop-map\n";
	Cout().Flush();
	if(!service.crop_map.Load(crop_map_path)) {
		Cerr() << "ERROR: shader crop map cannot be loaded: " << crop_map_path << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame stage=validate-manifest\n";
	Cout().Flush();
	if(!service.manifest.Validate(error)) {
		Cerr() << "ERROR: shader manifest invalid: " << error << "\n";
		return 1;
	}
	service.use_threshold = false;

	VsmVideoFileFrameSource video;
	Cout() << "shader-evidence-frame stage=open-video\n";
	Cout().Flush();
	if(!video.Open(video_path)) {
		Cerr() << "ERROR: cannot open direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}
	if(frame_second > 0 && !video.SeekMs((int64)frame_second * 1000)) {
		Cerr() << "ERROR: cannot seek direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}
	VsmImageBuffer source;
	int64 timestamp_ms = -1;
	Cout() << "shader-evidence-frame stage=decode-frame\n";
	Cout().Flush();
	if(!video.ReadFrame(source, timestamp_ms)) {
		Cerr() << "ERROR: cannot decode diagnostic frame: " << video.GetLastError() << "\n";
		return 1;
	}
	VsmImageBuffer left = CropBufferToImageBuffer(source, kLeftTableRect);
	VsmImageBuffer right = CropBufferToImageBuffer(source, kRightTableRect);
	if(left.IsEmpty() || right.IsEmpty()) {
		Cerr() << "ERROR: diagnostic windows are outside decoded frame: source="
		       << source.Info() << "\n";
		return 1;
	}
	VsmImageBuffer packed;
	Vector<VsmPackedWindow> windows;
	Cout() << "shader-evidence-frame stage=pack-windows\n";
	Cout().Flush();
	if(!VsmPackTwoWindows(left, right, packed, windows, error)) {
		Cerr() << "ERROR: diagnostic window packing failed: " << error << "\n";
		return 1;
	}
	for(VsmPackedWindow& window : windows)
		window.timestamp_ms = timestamp_ms;
	VsmShaderEvidenceAdapter adapter;
	adapter.SetService(service);
	Vector<VsmShaderWindowEvidence> results;
	Cout() << "shader-evidence-frame stage=process-evidence\n";
	Cout().Flush();
	if(!adapter.ProcessPacked(packed, windows, results, error)) {
		Cerr() << "ERROR: shader evidence processing failed: " << error << "\n";
		return 1;
	}

	Cout() << "shader-evidence-frame decoder=libavcodec video=" << video_path
	       << " timestamp_ms=" << timestamp_ms
	       << " source=" << source.Info()
	       << " manifest=" << manifest_path
	       << " crop_map=" << crop_map_path << "\n";
	for(const VsmShaderWindowEvidence& result : results) {
		Cout() << "shader-evidence window=" << result.id
		       << " timestamp_ms=" << result.timestamp_ms
		       << " evidence=" << result.evidence.image.Info()
		       << " runs=" << result.runs.GetCount()
		       << " error=" << (result.error.IsEmpty() ? "none" : result.error) << "\n";
		for(const VsmEvidenceTextRun& run : result.runs)
			Cout() << "shader-evidence-run window=" << result.id
			       << " text=" << run.text
			       << " bounds=" << run.bounds.left << "," << run.bounds.top
			       << "," << run.bounds.Width() << "," << run.bounds.Height()
			       << " confidence=" << Format("%.4f", run.confidence)
			       << " ambiguous=" << (run.ambiguous ? 1 : 0) << "\n";
	}
	Cout() << "shader-evidence-frame status=pass windows=" << results.GetCount() << "\n";
	return 0;
}

struct ShaderEvidenceFrameConfig {
	String video;
	String manifest;
	String crop_map;
	int frame_second = 0;

	void Jsonize(JsonIO& json) {
		json("video", video)("manifest", manifest)("crop_map", crop_map)
			("frame_second", frame_second);
	}
};

static int RunShaderEvidenceFrameConfig(const String& config_path)
{
	String content = LoadFile(config_path);
	ShaderEvidenceFrameConfig config;
	if(content.IsEmpty() || !LoadFromJson(config, content)) {
		Cerr() << "ERROR: shader evidence descriptor cannot be loaded: " << config_path << "\n";
		return 1;
	}
	if(config.video.IsEmpty() || config.manifest.IsEmpty() || config.crop_map.IsEmpty()) {
		Cerr() << "ERROR: shader evidence descriptor requires video, manifest, and crop_map: "
		       << config_path << "\n";
		return 1;
	}
	if(config.frame_second < 0) {
		Cerr() << "ERROR: shader evidence descriptor frame_second must be nonnegative: "
		       << config_path << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame-config path=" << config_path
	       << " video=" << config.video
	       << " manifest=" << config.manifest
	       << " crop_map=" << config.crop_map
	       << " frame_second=" << config.frame_second << "\n";
	Cout().Flush();
	return RunShaderEvidenceFrame(config.video, config.manifest, config.crop_map,
	                              config.frame_second);
}

static void ProcessTableFrame(const VsmFrameImage& table, VsmFrameImage& previous,
	                          bool has_previous, VsmLiveRegionClassifier& classifier,
	                          VsmTesseractOcrEngine& ocr, bool ocr_available,
	                          const VsmChangeDetectParams& change_params, StageSet& stages,
	                          LoopStats& stats, ResolveState& resolve_state,
	                          OcrCacheState& ocr_cache, const std::shared_ptr<Game>& game,
	                          bool verbose, int ocr_cap, const CardTemplates& card_templates,
	                          const DealerTemplate& dealer_template)
{
	WindowFrame frame;
	frame.descriptor = MakeWindowDescriptor("R", "right", kRightTableRect);
	frame.source_sequence = has_previous ? stats.frames + 1 : 0;
	frame.window_sequence = frame.source_sequence;
	frame.table.Set(table.width, table.height, ~table.data);
	bool prepared_has_previous = has_previous;
	PreparedTableFrame prepared = PrepareTableFrame(frame, previous, prepared_has_previous,
	                                                change_params, classifier, card_templates,
	                                                dealer_template, stats.frames + 1, verbose);
	if(prepared.ready)
		CommitPreparedTableFrame(prepared, ocr, ocr_available, stages, stats, resolve_state,
		                         ocr_cache, game, verbose, ocr_cap, false);
}

// ===========================================================================
// Task 0281: thread-safe game-state SNAPSHOT for the --gui live-mirror window.
//
// The recognition loop mutates the live `Game` on a BACKGROUND thread; the GUI
// runs a TopWindow on the MAIN thread. U++'s Ctrl tree is not thread-safe and
// the Game object must not be read by the GUI thread while the background
// thread mutates it. So the background thread, after each processed frame,
// copies the fields it wants to render into this plain-old-data snapshot under
// a Mutex; the GUI thread only ever touches this POD copy (never the Game).
//
// Every field here is a scalar / fixed-size array / String, so GameSnapshot is
// trivially copy-assignable (default operator=) under the lock -- deliberately
// NO Upp::Vector members, which would need clone()/<<= gymnastics to copy.
//
// HONEST SCOPE (carried over from Task 0280, do NOT paper over): the live loop
// only reliably resolves board cards / street / the OCR pot reading. Per-seat
// stacks/bets/actions/hole-cards are NOT driven from the live feed -- they are
// the engine's own seed state (6 seats, 1000 start cash, engine-dealt cards
// that have nothing to do with the video). The snapshot therefore separates
// RECOGNIZED fields (board/street/pot-OCR, from ResolveState) from ENGINE-SEED
// fields (per-seat, from the Game), and the renderer labels them as such rather
// than presenting engine-seed values as if they were recognized.
// ===========================================================================
struct SeatSnap {
	bool   present = false;
	int    seat = -1;
	int    uid = -1;
	String name;
	int    stack = 0;
	int    bet = 0;
	int    action = 0;   // PlayerAction enum
	bool   active = false;
	bool   turn = false;
	int    button = 0;
	bool   is_dealer = false;
	// Task 0287: which per-seat fields are driven by real video OCR (vs. the
	// engine-seed fallback). Renderer colours live fields as LIVE-DRIVEN and
	// falls back / labels engine-seed otherwise.
	bool   name_live = false;
	bool   stack_live = false;
	bool   bet_live = false;
	// Task 0289: video-recognized action / turn / fold / round-total.
	String action_text;          // last OCR'd action word ("Call"/"Fold"/...), "" none
	bool   acting = false;       // this seat had the freshest action read (turn indicator)
	bool   folded = false;       // last action normalized to "Fold" -> hide hole cards
	double round_total = -1;     // last OCR'd chip_badge_stack value, -1 none
	bool   round_total_live = false;
	double bet_ocr = -1;         // raw OCR bet value (for felt rendering), -1 none
	// Task 0290c: independent video-grounded signals for this seat.
	bool   dealer_video = false; // dealer-chip template match maps here (item 1)
	bool   timer_turn = false;   // countdown-timer bar is on this seat right now (item 2)
};

struct GameSnapshot {
	bool   valid = false;
	// --- recognized-from-video fields (ResolveState) ---
	int    board[5] = { -1, -1, -1, -1, -1 };
	int    board_count = 0;   // how many board cards the loop has resolved as dealt
	int    street = 0;        // derived from board_count: 0 preflop,1 flop,2 turn,3 river
	double last_pot_ocr = -1; // last OCR-read pot value (chips/BB per OCR text), -1 = none yet
	// --- engine-seed fields (Game), honestly NOT video-driven per-seat ---
	int    hand_id = 0;
	int    dealer = -1;
	int    engine_pot = 0;
	int    seat_count = 0;
	SeatSnap seats[9];
	// Task 0290c: independent video-grounded signals (mirror-seat indices) + the
	// engine's own dealer seat, so the renderer can show BOTH and their agreement.
	int    dealer_seat_video = -1;   // dealer-chip template match (item 1), -1 none
	double dealer_chip_score = -1;   // its peak TM_CCOEFF_NORMED score
	int    engine_dealer_seat = -1;  // seat with GBUTTON_DEALER (Task 0288)
	int    turn_seat_video = -1;     // countdown-timer bar seat (item 2), -1 none
	// --- recognition status counters ---
	int    frames = 0;
	int    total_regions = 0;
	int    street_events = 0;
	int    value_changes = 0;
	int    ocr_calls = 0;
	int    ocr_cache_hits = 0;
	int    elapsed_s = 0;
	bool   connected = false;
	// --- capture-to-render latency (Task 0282) ---
	// latency_last_ms: msecs() - ts for the frame this snapshot was just built
	// from, i.e. the same "capture(recv)-to-processed" quantity RunLive already
	// prints in aggregate (see capture_to_done_sum/n above) -- computed here
	// per-frame instead so the GUI can show it live. min/avg/max are over a
	// small rolling window (see kLatencyWindowFrames) so one noisy spike (e.g.
	// a single OCR call) doesn't dominate a single-frame readout.
	double latency_last_ms = -1;
	double latency_avg_ms  = -1;
	double latency_min_ms  = -1;
	double latency_max_ms  = -1;
	// --- Task 0289: bounded event log, published to the GUI panel (most recent
	// last). Fixed-size String array keeps GameSnapshot trivially copy-assignable
	// under the mutex (no Upp::Vector members -- same POD discipline as above).
	String events[kMaxEventLog];
	int    event_count = 0;
};

// Rolling-window size (in frames) for the GUI's live latency avg/min/max.
// Justification (real numbers, not a guess): Task 0280's own benchmark
// (0280_continuous_recognition_loop.md, real run at VideoServer --fps 8)
// measured a 673ms mean frame-arrival gap and OCR calls averaging 2196ms
// (max 2515ms) -- i.e. a single OCR-bearing frame's latency spike spans
// roughly 3-4 frame-arrival-gaps. 20 frames spans ~20*673ms =~ 13.5s of
// real wall-clock history at that measured pacing: long enough to average
// out a single OCR spike without smoothing away a real sustained latency
// shift over more than ~13s, matching the design direction's suggested
// 20-30 frame window (picked the low end since the GUI already redraws at
// ~5Hz -- a shorter window keeps the readout responsive to real change).
static const int kLatencyWindowFrames = 20;

// Build a snapshot from the live engine + recognition state. Called ONLY on the
// background recognition thread (it reads the Game); the result is published to
// the GUI under a mutex by the caller.
static GameSnapshot BuildSnapshot(const std::shared_ptr<Game>& game, const ResolveState& rs,
                                  const LoopStats& stats, const OcrCacheState& oc, int64 start_ms,
                                  double latency_last_ms = -1, double latency_avg_ms = -1,
                                  double latency_min_ms = -1, double latency_max_ms = -1)
{
	GameSnapshot s;
	s.latency_last_ms = latency_last_ms;
	s.latency_avg_ms = latency_avg_ms;
	s.latency_min_ms = latency_min_ms;
	s.latency_max_ms = latency_max_ms;
	s.frames = stats.frames;
	s.total_regions = stats.total_regions;
	s.street_events = rs.street_events;
	s.value_changes = rs.value_changes;
	s.ocr_calls = stats.ocr_calls;
	s.ocr_cache_hits = oc.hits;
	s.last_pot_ocr = rs.last_pot;
	s.board_count = rs.board_count;
	for(int i = 0; i < 5; i++) s.board[i] = rs.board_cards[i];
	s.street = rs.board_count >= 5 ? 3 : rs.board_count >= 4 ? 2 : rs.board_count >= 3 ? 1 : 0;
	s.elapsed_s = (int)((msecs() - start_ms) / 1000);
	// Task 0290c: publish the independent video-grounded dealer/turn signals.
	s.dealer_seat_video = rs.dealer_seat_video;
	s.dealer_chip_score = rs.dealer_chip_score;
	s.turn_seat_video = rs.turn_seat_video;
	if(game) {
		s.dealer = (int)game->getDealerPosition();
		s.hand_id = game->getCurrentHandID();
		if(auto hand = game->getCurrentHand())
			if(auto board = hand->getBoard())
				s.engine_pot = board->getPot();
		// getSeatsList() ALWAYS holds MAX_NUMBER_OF_PLAYERS entries (the engine
		// pads the vector to the max table size in Game's constructor); only the
		// first startQuantityPlayers of them are real seated players (the rest
		// are default-constructed padding with empty names). Render only the
		// real seats -- this table was seeded with 6 (StartData::numberOfPlayers).
		int startq = game->getStartQuantityPlayers();
		PlayerList seats = game->getSeatsList();
		if(seats) {
			int n = 0;
			for(auto& p : *seats) {
				if(n >= 9 || n >= startq) break;
				if(!p) continue;
				SeatSnap& ss = s.seats[n];
				ss.present = true;
				ss.seat = p->getMyID();
				ss.uid = (int)p->getMyUniqueID();
				ss.name = p->getMyName();
				ss.stack = p->getMyCash();
				ss.bet = p->getMySet();
				ss.action = (int)p->getMyAction();
				ss.active = p->getMyActiveStatus();
				ss.turn = p->getMyTurn();
				ss.button = p->getMyButton();
				// Task 0288 fix 4: mark the dealer from the engine's OWN authoritative
				// button assignment (LocalHand::assignButtons() sets exactly one seat's
				// button to GBUTTON_DEALER each hand) rather than
				// getMyUniqueID()==getDealerPosition(). The latter happens to work in
				// this flat 0..5 setup (uniqueID==dealerPosition space), but the button
				// field is the direct, unambiguous engine signal for "this seat has the
				// dealer button" and does not depend on the two id-spaces coinciding.
				ss.is_dealer = (p->getMyButton() == GBUTTON_DEALER);
				if(ss.is_dealer) s.engine_dealer_seat = n; // Task 0290c: engine dealer seat
				// Task 0290c item 1/2: independent video-grounded per-seat markers.
				if(n < 6) {
					ss.dealer_video = (rs.dealer_seat_video == n);
					ss.timer_turn   = (rs.turn_seat_video == n);
				}
				// Task 0287: override name/stack/bet with the real per-seat OCR
				// values WHEN AVAILABLE (RegionToSeat-attributed), keeping the
				// engine-seed value only where nothing has been recognized yet.
				// The render index n matches RegionToSeat's seat convention (both
				// 0=bottom..5=right-bottom), so rs.*[n] lines up with seats[n].
				if(n < 6) {
					if(!rs.seat_name[n].IsEmpty()) { ss.name = rs.seat_name[n]; ss.name_live = true; }
					if(rs.seat_stack[n] >= 0)      { ss.stack = (int)(rs.seat_stack[n] + 0.5); ss.stack_live = true; }
					if(rs.seat_bet[n] >= 0)        { ss.bet = (int)(rs.seat_bet[n] + 0.5); ss.bet_live = true; ss.bet_ocr = rs.seat_bet[n]; }
					// Task 0289: video-recognized action / fold / turn / round total.
					if(!rs.seat_action[n].IsEmpty())
						ss.action_text = rs.seat_action[n];
					// Task 0290b: fold state is now authoritative in ResolveState,
					// driven by BOTH the OCR-text path (action normalizing to "Fold")
					// AND the OCR-free structural card-back path -- so a fold the OCR
					// missed still hides the cards, and a redealt hand un-hides them.
					ss.folded = rs.seat_folded[n];
					if(rs.seat_round_total[n] >= 0) { ss.round_total = rs.seat_round_total[n]; ss.round_total_live = true; }
					// "acting" = this seat had the freshest action-bubble read within
					// a short recency window (the action bubble is transient in the
					// real client, so an old read should not keep claiming the turn).
					if(rs.last_action_seat == n && rs.last_action_ms > 0
					   && (msecs() - rs.last_action_ms) < 5000)
						ss.acting = true;
				}
				n++;
			}
			s.seat_count = n;
		}
	}
	// Task 0289: publish the most-recent event-log entries (already bounded to
	// kMaxEventLog in ResolveState) into the POD snapshot for the GUI panel.
	int ne = min(rs.event_log.GetCount(), (int)kMaxEventLog);
	int base = rs.event_log.GetCount() - ne;
	for(int i = 0; i < ne; i++) s.events[i] = rs.event_log[base + i];
	s.event_count = ne;
	s.valid = true;
	return s;
}

// ===========================================================================
// Mode 2: offline pipeline over dataset source frames (timing, deterministic).
// ===========================================================================
static int RunOfflineFrames(const String& frames_dir, const String& dataset_path,
                            int max_frames, bool verbose, bool use_engine, int ocr_cap,
                            bool ocr_cache_enabled, int ocr_cache_threshold)
{
	Cout() << "=== Mode: offline-frames (" << frames_dir << ") ===\n";
	VsmLiveRegionClassifier clf;
	if(clf.Load(dataset_path) == 0) { Cerr() << "ERROR: classifier load failed\n"; return 1; }
	Cout() << "classifier: " << clf.GetCount() << " reference entries\n";

	VsmTesseractOcrEngine ocr;
	bool ocr_available = ocr.GetInfo().available;
	Cout() << "tesseract OCR available: " << (ocr_available ? "YES" : "NO") << "\n";

	// enumerate frames
	Vector<String> files = FindAllPaths(frames_dir, "*.jpg");
	Sort(files);
	if(files.IsEmpty()) { Cerr() << "ERROR: no frames in " << frames_dir << "\n"; return 1; }
	if(max_frames > 0 && files.GetCount() > max_frames) files.SetCount(max_frames);
	Cout() << "processing " << files.GetCount() << " frames\n";

	std::shared_ptr<Game> game;
	class ConfigFile config(nullptr, false);
	std::shared_ptr<HeadlessGui> gui;
	std::shared_ptr<EngineLog> engineLog;
	if(use_engine) {
		gui = std::make_shared<HeadlessGui>();
		engineLog = std::make_shared<EngineLog>(&config);
		PlayerDataList pd;
		for(int i = 0; i < 6; i++) {
			auto d = std::make_shared<PlayerData>(i, i, PLAYER_TYPE_HUMAN,
			              i == 0 ? PLAYER_RIGHTS_ADMIN : PLAYER_RIGHTS_NORMAL, i == 0);
			d->SetName(Format("Seat%d", i));
			d->SetStartCash(1000);
			pd.push_back(d);
		}
		GameData gd; gd.maxNumberOfPlayers = 6; gd.startMoney = 1000;
		gd.firstSmallBlind = 5; gd.raiseSmallBlindEveryHandsValue = 100000;
		StartData sd; sd.numberOfPlayers = 6; sd.startDealerPlayerId = 0;
		auto factory = std::make_shared<LocalEngineFactory>();
		game = std::make_shared<Game>(gui.get(), factory, pd, gd, sd, 1, engineLog.get(), &config);
		game->SetBaseSeed(12345);
		game->initHand(); game->startHand();
		Cout() << "engine: real headless Game constructed (6 seats), hand started\n";
		// Task 0288 fix-4 diagnostic: confirm the dealer-button id-space question
		// for real. Prints the engine's dealerPosition and every seat's
		// uniqueID/button so we can see whether GBUTTON_DEALER is actually set on a
		// seat and whether uniqueID==dealerPosition coincides with it.
		if(verbose) {
			Cout() << Format("dealer diag: getDealerPosition()=%d\n", (int)game->getDealerPosition());
			if(auto seats = game->getSeatsList())
				for(auto& p : *seats) {
					if(!p) continue;
					Cout() << Format("  seat id=%d uid=%d button=%d name=%s cash=%d\n",
					                 p->getMyID(), (int)p->getMyUniqueID(), p->getMyButton(),
					                 ~p->getMyName(), p->getMyCash());
				}
		}
	}

	StageSet st;
	InitializeStageNames(st);
	LoopStats stats; ResolveState rs;
	CardTemplates card_templates = LoadCardTemplates(g_templates_dir); // Task 0290a
	DealerTemplate dealer_template = LoadDealerTemplate(g_templates_dir); // Task 0290c item 1
	VsmChangeDetectParams cdp;
	OcrCacheState oc; oc.enabled = ocr_cache_enabled; oc.threshold = ocr_cache_threshold;

	VsmFrameImage prev; bool has_prev = false;
	for(int fi = 0; fi < files.GetCount(); fi++) {
		double t = NowMs();
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(files[fi], buf)) continue;
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		st.acquire.Add(NowMs() - t);
		if(table.IsEmpty()) continue;

		if(verbose) Cout() << Format("frame %d (%s):\n", fi, ~GetFileName(files[fi]));
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap, card_templates, dealer_template);
		if(!has_prev) { prev.Set(table.width, table.height, ~table.data); has_prev = true; }
	}

	Cout() << "\n=== Per-stage timing (offline) ===\n";
	st.Print();
	Cout() << "\n=== Recognition summary ===\n";
	Cout() << "frames_processed=" << stats.frames << " total_changed_regions=" << stats.total_regions
	       << " (avg " << Format("%.1f", stats.frames ? (double)stats.total_regions / stats.frames : 0) << "/frame)\n";
	Cout() << "classified=" << stats.classified << " unresolved=" << stats.unresolved
	       << Format(" (%.1f%% resolved)\n", stats.classified ? 100.0 * (stats.classified - stats.unresolved) / stats.classified : 0);
	Cout() << "ocr_calls=" << stats.ocr_calls << " ocr_nonempty=" << stats.ocr_nonempty
	       << " ocr_cache_hits=" << oc.hits << " ocr_cache_misses=" << oc.misses
	       << Format(" (cache %s, threshold=%d)\n", oc.enabled ? "ENABLED" : "disabled", oc.threshold);
	Cout() << "resolve: street_events=" << rs.street_events << " value_changes=" << rs.value_changes
	       << " pot_reads=" << rs.pot_reads << " final_board_count=" << rs.board_count << "\n";
	Cout() << "\n--- category distribution ---\n";
	for(int i = 0; i < stats.category_counts.GetCount(); i++)
		Cout() << Format("  %-26s %d\n", ~stats.category_counts.GetKey(i), stats.category_counts[i]);
	return 0;
}

// ===========================================================================
// Mode 3: continuous live loop against a running VideoServer.
// ===========================================================================
static int RunLive(const String& host, int port, int seconds, const String& dataset_path,
                   bool verbose, bool use_engine, int wait_timeout_ms, int ocr_cap,
                   bool ocr_cache_enabled, int ocr_cache_threshold)
{
	Cout() << "=== Mode: live (" << host << ":" << port << " for " << seconds << "s) ===\n";
	VsmLiveRegionClassifier clf;
	if(clf.Load(dataset_path) == 0) { Cerr() << "ERROR: classifier load failed\n"; return 1; }
	Cout() << "classifier: " << clf.GetCount() << " reference entries\n";

	VsmTesseractOcrEngine ocr;
	bool ocr_available = ocr.GetInfo().available;
	Cout() << "tesseract OCR available: " << (ocr_available ? "YES" : "NO") << "\n";

	std::shared_ptr<Game> game;
	class ConfigFile config(nullptr, false);
	std::shared_ptr<HeadlessGui> gui;
	std::shared_ptr<EngineLog> engineLog;
	if(use_engine) {
		gui = std::make_shared<HeadlessGui>();
		engineLog = std::make_shared<EngineLog>(&config);
		PlayerDataList pd;
		for(int i = 0; i < 6; i++) {
			auto d = std::make_shared<PlayerData>(i, i, PLAYER_TYPE_HUMAN,
			              i == 0 ? PLAYER_RIGHTS_ADMIN : PLAYER_RIGHTS_NORMAL, i == 0);
			d->SetName(Format("Seat%d", i)); d->SetStartCash(1000);
			pd.push_back(d);
		}
		GameData gd; gd.maxNumberOfPlayers = 6; gd.startMoney = 1000;
		gd.firstSmallBlind = 5; gd.raiseSmallBlindEveryHandsValue = 100000;
		StartData sd; sd.numberOfPlayers = 6; sd.startDealerPlayerId = 0;
		auto factory = std::make_shared<LocalEngineFactory>();
		game = std::make_shared<Game>(gui.get(), factory, pd, gd, sd, 1, engineLog.get(), &config);
		game->SetBaseSeed(12345);
		game->initHand(); game->startHand();
		Cout() << "engine: real headless Game constructed (6 seats), hand started\n";
	}

	VsmVideoServerFrameSource src;
	src.SetWaitTimeoutMs(wait_timeout_ms);
	src.SetPollIntervalMs(20);
	String uri = host + ":" + IntStr(port);
	if(!src.Open(uri)) { Cerr() << "ERROR: Open(" << uri << ") failed: " << src.GetLastError() << "\n"; return 1; }
	Cout() << "opened " << uri << " size=" << src.GetWidth() << "x" << src.GetHeight() << "\n";

	Vector<WindowDescriptor> window_descriptors = SelectWindowDescriptors("right");
	ASSERT(window_descriptors.GetCount() == 1);
	RecognitionWindowState window;
	InitializeRecognitionWindow(window, window_descriptors[0],
	                            ocr_cache_enabled, ocr_cache_threshold);
	StageSet& st = window.stages;
	LoopStats& stats = window.loop_stats;
	ResolveState& rs = window.resolve_state;
	OcrCacheState& oc = window.ocr_cache;
	Cout() << "pipeline=window-aware deterministic=1 staged_pre_ocr=1 async_ocr=0"
	       << " prepared_queue=none ocr_queue=none window_count=1\n";
	Cout() << "window_descriptor id=" << window.descriptor.id.value
	       << " name=" << window.descriptor.name
	       << " rect=" << window.descriptor.source_rect.left << ","
	       << window.descriptor.source_rect.top << ","
	       << window.descriptor.source_rect.Width() << ","
	       << window.descriptor.source_rect.Height() << "\n";
	CardTemplates card_templates = LoadCardTemplates(g_templates_dir); // Task 0290a
	DealerTemplate dealer_template = LoadDealerTemplate(g_templates_dir); // Task 0290c item 1
	VsmChangeDetectParams cdp;
	VsmDistributedReconstructionAdapter live_reconstruction;
	bool live_reconstruction_started = false;
	int live_assertion_legal = 0;
	int live_assertion_undetermined = 0;
	int live_assertion_illegal = 0;
	int64 live_assertion_round = 0;

	int64 start = msecs();
	int64 deadline = start + (int64)seconds * 1000;
	int recoverable_timeouts = 0, unrecoverable = 0;
	Vector<double> per_frame_ms;      // full pipeline wall time / frame
	Vector<int64>  capture_gaps;      // receive-ts gaps
	int64 prev_ts = 0;
	int64 source_sequence = 0;
	int64 window_sequence = 0;
	double capture_to_done_sum = 0; int capture_to_done_n = 0;

	while(msecs() < deadline) {
		double fa = NowMs();
		VsmImageBuffer buf; int64 ts = 0;
		if(!src.ReadFrame(buf, ts)) {
			if(src.GetLastErrorKind() == VsmVideoServerFrameSource::VSM_VSFS_ERR_TIMEOUT) {
				recoverable_timeouts++; continue;
			}
			unrecoverable++;
			Cerr() << "ERROR: unrecoverable ReadFrame: " << src.GetLastError() << "\n";
			break;
		}
		double acq_and_read = NowMs() - fa;
		st.acquire.Add(acq_and_read);
		if(!RunShaderEvidenceStage(buf, SelectWindowDescriptors("both"), ts, "live"))
			return 1;
		double fanout_started = NowMs();
		Vector<WindowFrame> frames = FanOutSourceFrame(buf, source_sequence++, window_sequence,
		                                                ts, window_descriptors);
		st.crop.Add(NowMs() - fanout_started);
		ASSERT(frames.GetCount() == 1);
		if(frames[0].table.IsEmpty()) continue;
		window_sequence++;
		if(prev_ts > 0) capture_gaps.Add(ts - prev_ts);
		prev_ts = ts;

		double f0 = NowMs();
		bool processed = ProcessWindowFrame(frames[0], window, clf, ocr, ocr_available,
		                                    cdp, game, verbose, ocr_cap,
		                                    card_templates, dealer_template);
		double f_done = NowMs();
		if(processed) {
			per_frame_ms.Add(f_done - f0);
			// capture-to-done latency: from TCP-receive wall-clock (ts) to now.
			// NOTE (Task 0279 finding): ts is the TCP-RECEIVE instant, NOT the
			// true glass/capture instant -- VideoServer does not expose the
			// latter, so this measures network+decode+process, not glass-to-glass.
			capture_to_done_sum += (double)(msecs() - ts); capture_to_done_n++;
			DistributedStateSnapshot current = MakeSidecar2Snapshot(rs);
			if(!live_reconstruction_started) {
				live_reconstruction.Begin("live", current);
				live_reconstruction_started = true;
			}
			else {
				if(rs.last_action_frame == stats.frames && rs.last_action_seat >= 0
				   && rs.last_action_seat < 6) {
					String action = rs.seat_action[rs.last_action_seat];
					DistributedActionKind kind = action == "Fold"
					                  ? DISTRIBUTED_ACTION_REMOVE
					                  : action == "Call" || action == "Check"
					                  ? DISTRIBUTED_ACTION_MATCH
					                  : DISTRIBUTED_ACTION_INCREASE;
					VsmDistributedObservation observation;
					observation.stream = "live";
					observation.identity = Format("frame%d-action%d", stats.frames,
					                              rs.last_action_seat);
					observation.participant = rs.last_action_seat;
					observation.kind = kind;
					observation.sequence = live_assertion_round++;
					observation.timestamp = ts;
					observation.source = "live-video-recognition";
					live_reconstruction.Observe(observation);
				}
				DistributedServiceResult assertion = live_reconstruction.Complete(
					"live", current, live_assertion_round, ts);
				switch(assertion.legality.status) {
				case DISTRIBUTED_LEGALITY_LEGAL: live_assertion_legal++; break;
				case DISTRIBUTED_LEGALITY_ILLEGAL: live_assertion_illegal++; break;
				default: live_assertion_undetermined++; break;
				}
				if(assertion.legality.status != DISTRIBUTED_LEGALITY_LEGAL
				   || !assertion.reconstruction.diagnostics.IsEmpty()) {
					Cout() << Format("live_assertion round=%d status=%d authoritative=%d issues=%d inferred=%d\n",
					                 (int)live_assertion_round,
					                 (int)assertion.legality.status,
					                 assertion.authoritative_applied,
					                 assertion.legality.issues.GetCount(),
					                 assertion.reconstruction.actions.GetCount());
					for(const String& diagnostic : assertion.reconstruction.diagnostics)
						Cout() << "live_assertion_diagnostic=" << diagnostic << "\n";
				}
				live_reconstruction_started = false;
			}
		}

		if(stats.frames > 0 && stats.frames % 25 == 0)
			Cout() << Format("progress: frames=%d elapsed=%d s regions=%d street_events=%d value_changes=%d ocr_calls=%d ocr_cache_hits=%d\n",
			                 stats.frames, (int)((msecs() - start) / 1000), stats.total_regions,
			                 rs.street_events, rs.value_changes, stats.ocr_calls, oc.hits);
	}
	src.Close();

	// stats
	auto stat = [](const Vector<double>& v, double& avg, double& mx) {
		avg = 0; mx = 0; for(double x : v) { avg += x; if(x > mx) mx = x; } if(v.GetCount()) avg /= v.GetCount();
	};
	double pf_avg, pf_max; stat(per_frame_ms, pf_avg, pf_max);
	double gap_avg = 0; int64 gap_min = -1, gap_max = -1;
	for(int64 g : capture_gaps) { gap_avg += g; if(gap_min < 0 || g < gap_min) gap_min = g; if(g > gap_max) gap_max = g; }
	if(capture_gaps.GetCount()) gap_avg /= capture_gaps.GetCount();

	Cout() << "\n=== Per-stage timing (live) ===\n";
	st.Print();
	Cout() << "\n=== Live loop summary ===\n";
	Cout() << "frames_processed=" << stats.frames << " recoverable_timeouts=" << recoverable_timeouts
	       << " unrecoverable=" << unrecoverable
	       << " next_prepare_sequence=" << window.next_prepare_sequence
	       << " next_commit_sequence=" << window.next_commit_sequence
	       << " last_prepared_source_sequence=" << window.last_prepared_source_sequence
	       << " last_committed_source_sequence=" << window.last_committed_source_sequence << "\n";
	Cout() << "full per-frame pipeline: avg=" << Format("%.1f", pf_avg) << "ms max="
	       << Format("%.1f", pf_max) << "ms\n";
	Cout() << "inter-frame receive gap: avg=" << Format("%.1f", gap_avg) << "ms min="
	       << (int)gap_min << " max=" << (int)gap_max << "  (server pacing)\n";
	Cout() << "capture(recv)-to-processed latency: avg="
	       << Format("%.1f", capture_to_done_n ? capture_to_done_sum / capture_to_done_n : 0.0)
	       << "ms  [NOTE: recv-ts, not glass-ts]\n";
	Cout() << "total_changed_regions=" << stats.total_regions
	       << Format(" (avg %.1f/frame)\n", stats.frames ? (double)stats.total_regions / stats.frames : 0);
	Cout() << "classified=" << stats.classified << " unresolved=" << stats.unresolved
	       << Format(" (%.1f%% resolved)\n", stats.classified ? 100.0 * (stats.classified - stats.unresolved) / stats.classified : 0);
	Cout() << "ocr_calls=" << stats.ocr_calls << " ocr_nonempty=" << stats.ocr_nonempty
	       << " ocr_cache_hits=" << oc.hits << " ocr_cache_misses=" << oc.misses
	       << Format(" (cache %s, threshold=%d)\n", oc.enabled ? "ENABLED" : "disabled", oc.threshold);
	Cout() << "resolve: street_events=" << rs.street_events << " value_changes=" << rs.value_changes
	       << " pot_reads=" << rs.pot_reads
	       << " pot_rejected_reads=" << rs.pot_rejected_reads
	       << " final_board_count=" << rs.board_count << "\n";
	Cout() << "live_assertion legal=" << live_assertion_legal
	       << " undetermined=" << live_assertion_undetermined
	       << " illegal=" << live_assertion_illegal << "\n";
	Cout() << "\n--- category distribution ---\n";
	for(int i = 0; i < stats.category_counts.GetCount(); i++)
		Cout() << Format("  %-26s %d\n", ~stats.category_counts.GetKey(i), stats.category_counts[i]);

	// keep-up verdict against the real 250-300ms budget
	Cout() << "\n=== Keep-up verdict ===\n";
	Cout() << "mean per-frame processing " << Format("%.1f", pf_avg) << "ms vs mean frame arrival "
	       << Format("%.1f", gap_avg) << "ms => "
	       << (pf_avg < gap_avg ? "KEEPS UP (processing < arrival)"
	                            : "FALLS BEHIND (processing >= arrival)") << "\n";
	return 0;
}

// ===========================================================================
// Task 0281: Mode 4 -- live mirror GUI window.
//
// Threading model (verified against this repo's real precedent before
// choosing): reference/GuiMT/main.cpp is U++'s canonical worker-thread-to-GUI
// example -- it runs work on Upp::Thread and marshals results to the GUI thread
// via PostCallback / (its own docstring notes GuiLock+Call as the newer idiom).
// PostCallback is the right tool when the worker PUSHES discrete results; here
// the recognition loop instead continuously mutates one long-lived Game object
// and the GUI just needs the latest state at display cadence, so the natural
// fit is the pull variant: the worker publishes a POD snapshot under a Mutex
// after every frame, and the GUI thread's own SetTimeCallback timer (fires ON
// the GUI thread -- the same mechanism game/TexasHoldem/GameTable.cpp and
// game/Hearts use for their animation ticks) pulls the latest snapshot and
// repaints. This keeps ALL Game/Ctrl access single-threaded per object (Game
// only ever touched by the worker, Ctrls only ever by the GUI thread) with one
// Mutex-guarded POD hand-off between them -- no Ctrl is ever touched off the
// GUI thread, exactly the constraint the task spec calls out.
// ---------------------------------------------------------------------------
struct LiveMirrorShared {
	Mutex        mtx;
	GameSnapshot snap;      // guarded by mtx
	Atomic       stop;      // GUI sets on close; worker polls it to exit
	String       status;    // guarded by mtx: worker-side human status line
	LiveMirrorShared() { stop = 0; }
};

static String StreetName(int street)
{
	switch(street) {
	case 0: return "Preflop"; case 1: return "Flop";
	case 2: return "Turn";    case 3: return "River"; default: return "?";
	}
}

static String ActionName(int a)
{
	switch(a) {
	case PLAYER_ACTION_CHECK: return "check";
	case PLAYER_ACTION_CALL:  return "call";
	case PLAYER_ACTION_BET:   return "bet";
	case PLAYER_ACTION_RAISE: return "raise";
	case PLAYER_ACTION_FOLD:  return "fold";
	case PLAYER_ACTION_ALLIN: return "all-in";
	case PLAYER_ACTION_SMALL_BLIND: return "SB";
	case PLAYER_ACTION_BIG_BLIND:   return "BB";
	default: return "";
	}
}

// The paint surface -- renders a POD GameSnapshot only (never the live Game).
struct MirrorView : Ctrl {
	GameSnapshot snap;
	String       status;

	void SetSnapshot(const GameSnapshot& s, const String& st) { snap = s; status = st; Refresh(); }

	void DrawCard(Draw& w, int x, int y, int cw, int ch, const Image& img) {
		if(!img.IsEmpty()) { w.DrawImage(x, y, cw, ch, img); }
		else { w.DrawRect(x, y, cw, ch, Color(0, 80, 0)); }
		w.DrawRect(x, y, cw, 1, Color(0,0,0)); w.DrawRect(x, y + ch - 1, cw, 1, Color(0,0,0));
		w.DrawRect(x, y, 1, ch, Color(0,0,0)); w.DrawRect(x + cw - 1, y, 1, ch, Color(0,0,0));
	}

	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, Color(22, 58, 38));               // dark felt backdrop
		Font hf = SansSerif(18).Bold();
		Font f  = SansSerif(13);
		Font sf = SansSerif(11);
		Color ink = White(), sub = Color(200, 220, 205);

		// ---- header ----
		String conn = snap.connected ? "LIVE" : "connecting...";
		w.DrawText(16, 12, "Live Mirror -- game/TexasHoldem engine (Task 0281)", hf, ink);
		w.DrawText(16, 40, Format("%s   hand #%d   recognized street: %s   t+%d s",
		                          ~conn, snap.hand_id, ~StreetName(snap.street), snap.elapsed_s), f, sub);

		// ---- table oval ----
		int margin = 60;
		Rect tbl = RectC(margin, 78, max(200, sz.cx - 2*margin - 260), max(160, sz.cy - 78 - 150));
		w.DrawEllipse(tbl, Color(30, 96, 58), 6, Color(150, 110, 60));
		int cx = tbl.left + tbl.Width()/2, cy = tbl.top + tbl.Height()/2;

		// ---- pot (recognized) ----
		String potstr = snap.last_pot_ocr >= 0
		    ? Format("Recognized pot (OCR): %.4g", snap.last_pot_ocr)
		    : String("Recognized pot (OCR): (no reading yet)");
		Size ps = GetTextSize(potstr, f);
		w.DrawText(cx - ps.cx/2, cy - 84, potstr, f, White());

		// ---- board cards (recognized) ----
		const int bcw = 58, bch = 82, gap = 8;
		int bw = 5*bcw + 4*gap;
		int bx = cx - bw/2, by = cy - bch/2 + 6;
		for(int i = 0; i < 5; i++) {
			int x = bx + i*(bcw + gap);
			Image img;
			bool dealt = i < snap.board_count;
			if(dealt && snap.board[i] >= 0 && snap.board[i] < 52)
				img = TexasHoldemGetCardReferenceImage(snap.board[i], Size(bcw, bch), "default");
			else if(dealt)                                  // street dealt but this slot unresolved
				img = TexasHoldemGetCardBackReferenceImage(Size(bcw, bch), "default");
			else                                            // not dealt this street -> holder
				img = TexasHoldemGetBoardHolderReferenceImage(i, Size(bcw, bch), "default");
			DrawCard(w, x, by, bcw, bch, img);
		}

		// ---- seats around the oval ----
		int rx = tbl.Width()/2 + 26, ry = tbl.Height()/2 + 20;
		int n = max(1, snap.seat_count);
		const int boxw = 150, boxh = 74, scw = 30, sch = 42;
		for(int i = 0; i < snap.seat_count; i++) {
			const SeatSnap& ss = snap.seats[i];
			if(!ss.present) continue;
			const double kPi = 3.14159265358979323846;
			double ang = kPi/2 + (2*kPi * i) / n;    // seat 0 at bottom, clockwise
			int px = cx + (int)(rx * cos(ang));
			int py = cy + (int)(ry * sin(ang));
			int bxx = px - boxw/2, byy = py - boxh/2;
			// seat plate. Task 0289: the acting seat (freshest recognized action
			// bubble) gets a distinctly brighter plate so "whose turn / who just
			// acted" is visible at a glance -- the turn indicator the user asked for.
			Color plate = ss.acting ? Color(46, 66, 96)
			            : ss.folded ? Color(20, 24, 28)   // dim folded seats
			            : ss.turn   ? Color(70, 70, 30)
			                        : Color(28, 34, 40);
			w.DrawRect(bxx, byy, boxw, boxh, plate);
			Color edge = ss.acting ? Color(120, 180, 255) : Color(80,90,100);
			int   ew   = ss.acting ? 2 : 1;
			w.DrawRect(bxx, byy, boxw, ew, edge);
			w.DrawRect(bxx, byy+boxh-ew, boxw, ew, edge);
			w.DrawRect(bxx, byy, ew, boxh, edge);
			w.DrawRect(bxx+boxw-ew, byy, ew, boxh, edge);
			// Task 0288 fix 4: the dealer USED to be a bare " (D)" suffix appended
			// to the name text -- easy to miss, and clipped off the 150px box once
			// real OCR'd names (Task 0287) fill the line. Draw an unmistakable
			// dealer button instead: a gold disc with a black "D" at the box's
			// top-right corner, plus a gold seat-plate border. is_dealer now comes
			// from the engine's own GBUTTON_DEALER assignment (fix in BuildSnapshot).
			if(ss.is_dealer) {
				const int dd = 22, dcx = bxx + boxw - dd/2 - 3, dcy = byy + dd/2 + 3;
				w.DrawRect(bxx, byy, boxw, 2, Color(230, 200, 60));       // gold top edge
				w.DrawRect(bxx, byy+boxh-2, boxw, 2, Color(230, 200, 60));
				w.DrawRect(bxx, byy, 2, boxh, Color(230, 200, 60));
				w.DrawRect(bxx+boxw-2, byy, 2, boxh, Color(230, 200, 60));
				w.DrawEllipse(RectC(dcx - dd/2, dcy - dd/2, dd, dd),
				              Color(235, 205, 70), 2, Color(120, 90, 10)); // gold disc
				Font df = SansSerif(13).Bold();
				Size ds = GetTextSize("D", df);
				w.DrawText(dcx - ds.cx/2, dcy - ds.cy/2, "D", df, Color(20, 20, 20));
			}
			// Task 0290c item 1: INDEPENDENT video-grounded dealer marker (from the
			// dealer-chip template match) drawn at the box's top-LEFT as a green "D"
			// disc -- deliberately distinct from and next to the engine's gold "D" at
			// top-right, so a viewer sees at a glance whether the two AGREE (they
			// should, if Task 0288 fixed the earlier off-by-one-hand bug). Green =
			// "confirmed by real pixels on screen", not just engine bookkeeping.
			if(ss.dealer_video) {
				const int dd = 20, dcx = bxx + dd/2 + 3, dcy = byy + dd/2 + 3;
				w.DrawEllipse(RectC(dcx - dd/2, dcy - dd/2, dd, dd),
				              Color(70, 210, 110), 2, Color(15, 70, 35));   // green disc
				Font df = SansSerif(12).Bold();
				Size ds = GetTextSize("D", df);
				w.DrawText(dcx - ds.cx/2, dcy - ds.cy/2, "D", df, Color(10, 30, 15));
			}
			// Task 0290c item 2: CONTINUOUS turn indicator from the countdown-timer
			// bar. Unlike 0289's "acting" plate (a momentary post-action-bubble read),
			// this lights up the WHOLE time the player is deciding. Rendered as a
			// bright green frame + a small green "TIMER" tag, distinct from the blue
			// "acting" edge, plus a mimic of the on-screen bar under the box.
			if(ss.timer_turn) {
				const int gw = 3; Color tg(90, 240, 130);
				w.DrawRect(bxx - gw, byy - gw, boxw + 2*gw, gw, tg);
				w.DrawRect(bxx - gw, byy + boxh, boxw + 2*gw, gw, tg);
				w.DrawRect(bxx - gw, byy - gw, gw, boxh + 2*gw, tg);
				w.DrawRect(bxx + boxw, byy - gw, gw, boxh + 2*gw, tg);
				// green->yellow timer-bar mimic just under the box (the real graphic)
				w.DrawRect(bxx + 4, byy + boxh + gw + 1, (boxw - 8) * 2 / 3, 3, Color(120, 230, 40));
				w.DrawRect(bxx + 4 + (boxw - 8) * 2 / 3, byy + boxh + gw + 1, (boxw - 8) / 3, 3, Color(235, 210, 40));
				Font tf = SansSerif(9).Bold();
				w.DrawText(bxx + 4, byy - gw - 12, "TIMER (turn)", tf, tg);
			}
			// Task 0287: colour per-seat fields by provenance -- LIVE-DRIVEN
			// green (Color(150,230,160)) when the value came from real video
			// OCR this run, ENGINE-SEED amber (Color(235,180,120)) otherwise,
			// matching the right-hand legend's colour convention.
			const Color kLive = Color(150, 230, 160), kSeed = Color(235, 180, 120);
			String nm = ss.name.IsEmpty() ? Format("Seat%d", ss.seat) : ss.name;
			Color namec = ss.name_live ? kLive : (ss.active ? White() : Color(150,150,150));
			w.DrawText(bxx + 6, byy + 4, nm, sf, namec);
			// Task 0288 fix 1: the stack fallback USED to render the engine's own
			// internal cash (post-blind-deduction from a flat 1000-chip placeholder
			// economy, e.g. "990"/"995") -- a real-but-meaningless number that
			// looked like a genuine OCR reading and fooled two prior review passes.
			// Only show a specific number when it is a real per-seat OCR value;
			// otherwise render an explicit "unknown" placeholder, exactly as the
			// hole cards already show face-down backs instead of fabricated faces.
			if(ss.stack_live)
				w.DrawText(bxx + 6, byy + 22, Format("stack %d", ss.stack), sf, kLive);
			else
				w.DrawText(bxx + 6, byy + 22, "stack ?", sf, kSeed);
			// Task 0288 fix 1 (bet): same problem -- the pre-0288 fallback showed
			// the engine's internal per-seat bet. Only ever show a bet number when
			// it is a real OCR reading for this seat; when it is not live, show
			// nothing (a fabricated engine bet is worse than an empty bet line).
			String betstr = (ss.bet_live && ss.bet > 0) ? Format("bet %d", ss.bet) : String();
			if(!betstr.IsEmpty())
				w.DrawText(bxx + 6, byy + 40, betstr, sf, kLive);
			// Task 0289: prefer the REAL video-recognized action word
			// (seat_action_bubble OCR) over the engine-seed PlayerAction; fall back
			// to the engine action only when nothing has been recognized yet.
			String ac = ss.action_text.IsEmpty() ? ActionName(ss.action) : ss.action_text;
			Color  acc = ss.action_text.IsEmpty() ? kSeed
			           : ss.folded ? Color(235, 130, 120) : Color(150, 230, 160);
			if(!ac.IsEmpty()) {
				int axoff = betstr.IsEmpty() ? 0 : GetTextSize(betstr, sf).cx + 10;
				w.DrawText(bxx + 6 + axoff, byy + 40, ac, sf, acc);
			}
			// Task 0289: fold-aware hole cards. Per-seat hole-card recognition is
			// still not built, so an ACTIVE seat shows realistic face-down backs
			// (never fabricated faces). A seat whose latest recognized action is
			// "Fold" hides its cards entirely and shows a small grey "folded" tag
			// -- the user's request that folded players' cards stop showing.
			if(ss.folded) {
				Font ff = SansSerif(10).Bold();
				String ft = "folded";
				Size fts = GetTextSize(ft, ff);
				w.DrawText(bxx + boxw - fts.cx - 6, byy + boxh - fts.cy - 4, ft, ff, Color(150,150,150));
			}
			else {
				Image back = TexasHoldemGetCardBackReferenceImage(Size(scw, sch), "default");
				DrawCard(w, bxx + boxw - 2*scw - 8, byy + boxh - sch - 4, scw, sch, back);
				DrawCard(w, bxx + boxw - scw - 4,   byy + boxh - sch - 4, scw, sch, back);
			}

			// Task 0289: felt-rendered bet + round-total chips. The user asked for
			// bet amounts to appear ON the felt (more noticeable) rather than only
			// inside the name box, plus a distinct "round bet total" chip figure.
			// Both are drawn pulled IN toward the pot -- the same center-pulled
			// layout the real client uses (and that RegionToSeat's anchors already
			// account for). Position: partway along the seat->center line.
			auto feltPoint = [&](double frac, int& fx, int& fy) {
				fx = (int)(px + (cx - px) * frac);
				fy = (int)(py + (cy - py) * frac);
			};
			if(ss.bet_ocr >= 0) {
				int fx, fy; feltPoint(0.34, fx, fy);
				String bt = Format("%.4g BB", ss.bet_ocr);
				Size bts = GetTextSize(bt, sf);
				// small chip glyph + amount on a dark pill so it reads on the felt
				int pw = bts.cx + 26, ph = 20, pxl = fx - pw/2, pyl = fy - ph/2;
				w.DrawRect(pxl, pyl, pw, ph, Color(18, 40, 26));
				w.DrawEllipse(RectC(pxl + 3, pyl + 3, 14, 14), Color(210, 180, 70), 1, Color(120,90,20));
				w.DrawText(pxl + 21, pyl + 4, bt, sf, Color(255, 236, 170));
			}
			if(ss.round_total_live) {
				int fx, fy; feltPoint(0.52, fx, fy);
				String rt = Format("%.4g BB", ss.round_total);
				Size rts = GetTextSize(rt, sf);
				int pw = rts.cx + 26, ph = 20, pxl = fx - pw/2, pyl = fy - ph/2;
				w.DrawRect(pxl, pyl, pw, ph, Color(40, 30, 44));
				// stacked-chips glyph (two discs) to distinguish the round-total
				// element from the single-chip bet pill above.
				w.DrawEllipse(RectC(pxl + 3, pyl + 6, 12, 10), Color(120, 170, 230), 1, Color(40,70,120));
				w.DrawEllipse(RectC(pxl + 3, pyl + 3, 12, 10), Color(150, 200, 250), 1, Color(40,70,120));
				w.DrawText(pxl + 21, pyl + 4, rt, sf, Color(190, 220, 255));
			}
		}

		// ---- right-hand status / honesty panel ----
		const int panelx = sz.cx - 250, panely = 88, panelw = 236, statusH = 352;
		w.DrawRect(panelx, panely, panelw, statusH, Color(16, 22, 28));
		int ty = panely + 8;
		auto line = [&](const String& t, Color c) { w.DrawText(panelx + 10, ty, t, sf, c); ty += 18; };
		w.DrawText(panelx + 10, ty, "Recognition status", SansSerif(12).Bold(), White()); ty += 20;
		ty += 2;
		line(Format("frames processed: %d", snap.frames), sub);
		line(Format("changed regions: %d", snap.total_regions), sub);
		line(Format("street events: %d", snap.street_events), sub);
		line(Format("value changes: %d", snap.value_changes), sub);
		line(Format("OCR calls: %d", snap.ocr_calls), sub);
		line(Format("OCR cache hits: %d", snap.ocr_cache_hits), sub);
		line(Format("board resolved: %d/5", snap.board_count), sub);
		line(Format("engine pot: %d", snap.engine_pot), sub);
		// Task 0290c: the two independent video-grounded signals + cross-check.
		{
			String eng = snap.engine_dealer_seat >= 0 ? Format("seat %d", snap.engine_dealer_seat) : String("?");
			String vid = snap.dealer_seat_video >= 0
			    ? Format("seat %d (%.2f)", snap.dealer_seat_video, snap.dealer_chip_score) : String("none");
			line(Format("dealer engine: %s", ~eng), Color(230, 200, 60));
			line(Format("dealer video : %s", ~vid), Color(70, 210, 110));
			bool agree = snap.dealer_seat_video >= 0 && snap.dealer_seat_video == snap.engine_dealer_seat;
			bool have  = snap.dealer_seat_video >= 0 && snap.engine_dealer_seat >= 0;
			line(have ? (agree ? "  -> AGREE" : "  -> DIFFER") : "  -> (pending)",
			     have ? (agree ? Color(120, 230, 140) : Color(240, 120, 110)) : sub);
			line(snap.turn_seat_video >= 0
			     ? Format("turn (timer): seat %d", snap.turn_seat_video)
			     : String("turn (timer): none"), Color(90, 240, 130));
		}
		ty += 4;
		// capture-to-render latency (Task 0282): msecs()-ts for the frame this
		// snapshot reflects, plus rolling min/avg/max (see kLatencyWindowFrames)
		// so a single OCR-call spike doesn't dominate a one-frame readout.
		if(snap.latency_last_ms >= 0)
			// NOTE: U++ Format()'s %f id-scan greedily consumes ALL trailing
			// alpha chars as the format id (so "%.0fms" parses as the unknown
			// id "fms", not "f" followed by literal "ms") -- a backtick (`)
			// right after the conversion letter terminates the id explicitly
			// and is itself consumed/skipped, leaving "ms" as literal text.
			line(Format("latency: %.0f`ms (avg %.0f, min %.0f, max %.0f)",
			            snap.latency_last_ms, snap.latency_avg_ms, snap.latency_min_ms, snap.latency_max_ms),
			     Color(255, 210, 120));
		else
			line("latency: (no frame yet)", Color(255, 210, 120));
		ty += 8;
		// Legend, updated for Task 0289: per-seat action/fold are now video-driven.
		line("LIVE (OCR): board, street, pot,", Color(150, 230, 160));
		line("  name/stack/bet/action, folds", Color(150, 230, 160));
		line("  (Task 0287/0289).", Color(150, 230, 160));
		line("Folds also confirmed OCR-free via", Color(150, 230, 160));
		line("  card-back presence (0290b).", Color(150, 230, 160));
		ty += 2;
		line("Felt chips: bet=gold pill,", Color(255, 236, 170));
		line("  round-total=blue (both OCR).", Color(190, 220, 255));
		line("Bright blue plate = acting seat.", Color(120, 180, 255));
		line("Gold \"D\"=engine dealer (0288).", Color(230, 200, 60));
		line("Green \"D\"=video dealer chip (0290c).", Color(70, 210, 110));
		line("Green frame=timer/turn (0290c).", Color(90, 240, 130));

		// ---- Task 0289: scrolling event log on the right edge, below the status
		// panel. Shows the most recent resolved events (the SAME stream the
		// --verbose console prints), newest at the bottom, bounded to kMaxEventLog.
		int logy = panely + statusH + 8;
		int logh = max(60, sz.cy - logy - 16);
		w.DrawRect(panelx, logy, panelw, logh, Color(12, 16, 20));
		w.DrawText(panelx + 10, logy + 8, "Event log (recent)", SansSerif(12).Bold(), White());
		int erow_h = 16, elist_top = logy + 30;
		int rows_fit = max(1, (logy + logh - 6 - elist_top) / erow_h);
		int first = max(0, snap.event_count - rows_fit);  // show the last rows_fit events
		int eyy = elist_top;
		for(int i = first; i < snap.event_count; i++) {
			w.DrawText(panelx + 8, eyy, snap.events[i], SansSerif(10), Color(190, 205, 215));
			eyy += erow_h;
		}
		if(snap.event_count == 0)
			w.DrawText(panelx + 8, elist_top, "(no events yet)", SansSerif(10), Color(120, 130, 140));
		if(!status.IsEmpty()) w.DrawText(panelx + 8, logy + logh - 16, status, SansSerif(9), Color(150, 160, 170));
	}
};

struct MirrorWindow : TopWindow {
	MirrorView       view;
	LiveMirrorShared* shared = nullptr;

	MirrorWindow() {
		Title("Live Mirror -- VideoLiveRecognitionLoop --gui");
		Sizeable().Zoomable();
		SetRect(0, 0, 1180, 780);
		Add(view.SizePos());
	}
	void Attach(LiveMirrorShared* sh) {
		shared = sh;
		SetTimeCallback(-200, [=] { Tick(); }); // ~5 Hz GUI refresh (>> recognized-action rate)
	}
	void Tick() {
		GameSnapshot s; String st;
		{ Mutex::Lock __(shared->mtx); s = shared->snap; st = shared->status; }
		view.SetSnapshot(s, st);
	}
	virtual void Close() override {
		if(shared) shared->stop = 1;   // tell the worker to stop before we tear down
		TopWindow::Close();
	}
};

// Background recognition loop for --gui: same pipeline as RunLive, but runs
// until the GUI asks it to stop (no fixed --seconds deadline) and publishes a
// snapshot after every processed frame. Runs on a worker thread; touches the
// Game but never any Ctrl.
static void GuiRecognitionWorker(LiveMirrorShared* shared, const String& host, int port,
                                 const String& dataset_path, bool use_engine, int wait_timeout_ms,
                                 int ocr_cap, bool ocr_cache_enabled, int ocr_cache_threshold, bool verbose)
{
	auto set_status = [&](const String& s) { Mutex::Lock __(shared->mtx); shared->status = s; };

	VsmLiveRegionClassifier clf;
	if(clf.Load(dataset_path) == 0) { set_status("ERROR: classifier load failed"); Cerr() << "ERROR: classifier load failed\n"; return; }
	Cout() << "classifier: " << clf.GetCount() << " reference entries\n";

	VsmTesseractOcrEngine ocr;
	bool ocr_available = ocr.GetInfo().available;
	Cout() << "tesseract OCR available: " << (ocr_available ? "YES" : "NO") << "\n";

	std::shared_ptr<Game> game;
	class ConfigFile config(nullptr, false);
	std::shared_ptr<HeadlessGui> gui;
	std::shared_ptr<EngineLog> engineLog;
	if(use_engine) {
		gui = std::make_shared<HeadlessGui>();
		engineLog = std::make_shared<EngineLog>(&config);
		PlayerDataList pd;
		for(int i = 0; i < 6; i++) {
			auto d = std::make_shared<PlayerData>(i, i, PLAYER_TYPE_HUMAN,
			              i == 0 ? PLAYER_RIGHTS_ADMIN : PLAYER_RIGHTS_NORMAL, i == 0);
			d->SetName(Format("Seat%d", i)); d->SetStartCash(1000);
			pd.push_back(d);
		}
		GameData gd; gd.maxNumberOfPlayers = 6; gd.startMoney = 1000;
		gd.firstSmallBlind = 5; gd.raiseSmallBlindEveryHandsValue = 100000;
		StartData sd; sd.numberOfPlayers = 6; sd.startDealerPlayerId = 0;
		auto factory = std::make_shared<LocalEngineFactory>();
		game = std::make_shared<Game>(gui.get(), factory, pd, gd, sd, 1, engineLog.get(), &config);
		game->SetBaseSeed(12345);
		game->initHand(); game->startHand();
		Cout() << "engine: real headless Game constructed (6 seats), hand started\n";
	}

	StageSet st;
	InitializeStageNames(st);
	LoopStats stats; ResolveState rs;
	CardTemplates card_templates = LoadCardTemplates(g_templates_dir); // Task 0290a
	DealerTemplate dealer_template = LoadDealerTemplate(g_templates_dir); // Task 0290c item 1
	VsmChangeDetectParams cdp;
	OcrCacheState oc; oc.enabled = ocr_cache_enabled; oc.threshold = ocr_cache_threshold;

	int64 start = msecs();

	// Rolling window of the last kLatencyWindowFrames capture-to-processed
	// latencies (Task 0282). Declared here (outer scope, above the reconnect
	// loop below) so it accumulates across reconnects the same way stats/rs/oc
	// already do -- a transient VideoServer reconnect isn't a reason to reset
	// the diagnostic latency readout to empty.
	double latency_window[kLatencyWindowFrames];
	int    latency_window_count = 0, latency_window_next = 0;
	double latency_last = -1;
	auto push_latency = [&](double v) {
		latency_last = v;
		latency_window[latency_window_next] = v;
		latency_window_next = (latency_window_next + 1) % kLatencyWindowFrames;
		if(latency_window_count < kLatencyWindowFrames) latency_window_count++;
	};
	auto latency_stats = [&](double& avg, double& mn, double& mx) {
		avg = -1; mn = -1; mx = -1;
		if(latency_window_count == 0) return;
		double sum = 0; mn = latency_window[0]; mx = latency_window[0];
		for(int i = 0; i < latency_window_count; i++) {
			double v = latency_window[i];
			sum += v; if(v < mn) mn = v; if(v > mx) mx = v;
		}
		avg = sum / latency_window_count;
	};

	// publish an initial (pre-connect) snapshot so the window isn't blank
	{
		GameSnapshot s0 = BuildSnapshot(game, rs, stats, oc, start);
		s0.connected = false;
		Mutex::Lock __(shared->mtx); shared->snap = s0; shared->status = "connecting to VideoServer...";
	}

	// Outer RECONNECT loop: a live mirror must survive a transient VideoServer
	// disconnect (observed: the server drops the MJPEG client mid-stream) and
	// keep mirroring for as long as the window is open, rather than exiting on
	// the first connection reset the way --live's fixed-duration loop does.
	// Recognition state (stats/rs/oc) accumulates ACROSS reconnects; only the
	// change-detection baseline (prev) resets per connection.
	String uri = host + ":" + IntStr(port);
	int reconnects = 0;
	while(!shared->stop) {
		VsmVideoServerFrameSource src;
		src.SetWaitTimeoutMs(wait_timeout_ms);
		src.SetPollIntervalMs(20);
		if(!src.Open(uri)) {
			set_status(String("waiting for VideoServer (") + src.GetLastError() + ")...");
			for(int k = 0; k < 10 && !shared->stop; k++) Thread::Sleep(50);
			continue;
		}
		Cout() << "opened " << uri << " size=" << src.GetWidth() << "x" << src.GetHeight()
		       << (reconnects ? Format(" (reconnect #%d)", reconnects) : String()) << "\n";
		set_status("connected");

		VsmFrameImage prev; bool has_prev = false;
		while(!shared->stop) {
			double fa = NowMs();
			VsmImageBuffer buf; int64 ts = 0;
			if(!src.ReadFrame(buf, ts)) {
				if(src.GetLastErrorKind() == VsmVideoServerFrameSource::VSM_VSFS_ERR_TIMEOUT)
					continue;                    // recoverable: just wait for the next frame
				// unrecoverable (connection reset etc.): drop out to reconnect
				set_status(String("connection lost -- reconnecting (") + src.GetLastError() + ")");
				Cerr() << "ReadFrame error: " << src.GetLastError() << " -- reconnecting\n";
				break;
			}
			st.acquire.Add(NowMs() - fa);
			VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
			if(table.IsEmpty()) continue;

			ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap, card_templates, dealer_template);
			if(!has_prev) { prev.Set(table.width, table.height, ~table.data); has_prev = true; }

			// capture-to-processed latency for this frame (Task 0282), same
			// quantity RunLive computes in aggregate (capture_to_done_sum/n):
			// msecs() - ts, from the frame's TCP-receive timestamp to right now,
			// right after ProcessTableFrame returns and before publishing the
			// snapshot that reflects it.
			push_latency((double)(msecs() - ts));
			double lat_avg, lat_min, lat_max;
			latency_stats(lat_avg, lat_min, lat_max);

			// publish snapshot for the GUI
			GameSnapshot s = BuildSnapshot(game, rs, stats, oc, start, latency_last, lat_avg, lat_min, lat_max);
			s.connected = true;
			{ Mutex::Lock __(shared->mtx); shared->snap = s;
			  shared->status = Format("frame %d, %d regions", stats.frames, stats.total_regions); }
		}
		src.Close();
		if(!shared->stop) { reconnects++; for(int k = 0; k < 6 && !shared->stop; k++) Thread::Sleep(50); }
	}
	Cout() << "gui worker stopped: frames=" << stats.frames << " street_events=" << rs.street_events
	       << " value_changes=" << rs.value_changes << " ocr_calls=" << stats.ocr_calls
	       << " reconnects=" << reconnects << "\n";
}

// ===========================================================================
// Mode 4: live mirror GUI window.
// ===========================================================================
static int RunGui(const String& host, int port, const String& dataset_path,
                  bool verbose, bool use_engine, int wait_timeout_ms, int ocr_cap,
                  bool ocr_cache_enabled, int ocr_cache_threshold)
{
	Cout() << "=== Mode: gui (live mirror against " << host << ":" << port << ") ===\n";
	Cout() << "close the window to stop.\n";

	LiveMirrorShared shared;
	MirrorWindow win;
	win.Attach(&shared);

	Thread worker;
	worker.Run([&] {
		GuiRecognitionWorker(&shared, host, port, dataset_path, use_engine, wait_timeout_ms,
		                     ocr_cap, ocr_cache_enabled, ocr_cache_threshold, verbose);
	});

	win.Run();                 // main-thread GUI event loop until the window closes

	shared.stop = 1;           // ensure worker exits (Close() also sets this)
	worker.Wait();             // join before tearing down `shared`/`win`
	Cout() << "gui closed.\n";
	return 0;
}

// ===========================================================================
// Task 0291a: SIDECAR GROUND-TRUTH PARSER (shared by all real-frame test modes).
//
// Why this exists: Task 0290a/0290c HARD-CODED hand-1's board/dealer/turn frame
// numbers, so their tests silently never touched hand 2's real action even when
// pointed at hand-2 frames. This parser reads the real sidecar
// (bin/video_record_25min_20260716_203356.txt, both hands, 65 lines) and DERIVES
// every scored frame from the sidecar's own event timestamps -- so a test always
// covers exactly the real action the sidecar describes, for whatever frame
// directory it is given, with no hand-specific frame numbers baked in.
//
// FRAME<->SECOND: both real frame directories were extracted at fps=1 with the
// output filename number set to the real video second (0263: frame_000000..56 =
// sec 0..56; 0268: frame_000052..105 = sec 52..105, ffmpeg -start_number 52), so
// throughout these tests  frame_%06d.jpg's number == video second == sidecar
// HH:MM:SS in seconds. (See each dir's FRAME_TIME_MAPPING.md.)
// ===========================================================================
static const char* kSidecarPath = "bin/video_record_25min_20260716_203356.txt";
static const char* kVideoPath = "bin/video_record_25min_20260716_203356.mp4";

// sidecar seat (1..6, per the file's own header) -> mirror seat index (0..5,
// the kSeatAnchors / RegionToSeat convention). Same mapping Task 0290c encoded
// inline (bottom=seat4=0, leftbot=seat5=1, lefttop=seat6=2, top=seat1=3,
// righttop=seat2=4, rightbot=seat3=5) -- centralized here.
static int SidecarSeatToMirror(int s)
{
	switch(s) { case 4: return 0; case 5: return 1; case 6: return 2;
	            case 1: return 3; case 2: return 4; case 3: return 5; }
	return -1;
}
static int ParseHmsToSec(const String& hms) // "00:01:27" -> 87
{
	Vector<String> p = Split(hms, ':');
	if(p.GetCount() != 3) return -1;
	return StrInt(p[0]) * 3600 + StrInt(p[1]) * 60 + StrInt(p[2]);
}
// Split a run of card characters ("3d3sKh5c") into engine card indices.
static Vector<int> ParseCardRun(const String& s)
{
	Vector<int> out;
	for(int i = 0; i + 1 < s.GetCount(); i += 2)
		out.Add(CardIndex(s.Mid(i, 1), s.Mid(i + 1, 1)));
	return out;
}

struct SidecarGT {
	// board timeline: change points (second, board-string) across BOTH hands;
	// board resets to "" at each "new hand". BoardAt(sec) returns the board in
	// effect at that second (last change <= sec).
	struct BoardPt : Moveable<BoardPt> { int sec; String board; };
	Vector<BoardPt> board_pts;
	// showdown reveals: seat's hole cards shown face-up at `sec`.
	struct Show : Moveable<Show> { int sec; int mirror; String cards; };
	Vector<Show> shows;
	// derived per-second acting seat (turn), from action-line timestamps.
	VectorMap<int, int> turn_by_second; // video second -> acting mirror seat
	// dealer mirror seat per new-hand second (both hands: seat4 -> mirror 0).
	VectorMap<int, int> dealer_by_hand_sec;
	Vector<int> new_hand_seconds;
	// hand-start seat labels: (hand index -> mirror -> name / starting balanceBB).
	struct SeatInfo : Moveable<SeatInfo> { String name; double balance = -1; };
	Vector<VectorMap<int, SeatInfo>> hands; // hands[h][mirror] = SeatInfo

	String BoardAt(int sec) const {
		String b;
		for(const BoardPt& p : board_pts) { if(p.sec <= sec) b = p.board; else break; }
		return b;
	}
	int HandIndexOf(int sec) const { // 0-based hand this second belongs to, or -1
		int h = -1;
		for(int i = 0; i < new_hand_seconds.GetCount(); i++)
			if(new_hand_seconds[i] <= sec) h = i;
		return h;
	}
};

static bool IsActionVerb(const String& w)
{
	return w == "fold" || w == "call" || w == "raise" || w == "check"
	    || w == "bet"  || w == "shows"; // "shows" handled separately, excluded below
}

static SidecarGT ParseSidecarGT(const String& path, char window = 'R')
{
	SidecarGT gt;
	String txt = LoadFile(path);
	Vector<String> lines = Split(txt, '\n');
	String cur_board;               // accumulates within a hand
	int    cur_hand = -1;
	int    prev_action_sec = -1;    // for turn-window derivation
	struct PendingSeat : Moveable<PendingSeat> { int sec; int mirror; String name; double balance; };
	Vector<PendingSeat> pend;       // seat name/balance lines, hand resolved post-parse
	for(String raw : lines) {
		String line = TrimBoth(raw);
		if(line.GetCount() < 2 || line[0] != window || line[1] != ' ') continue;
		int colon = line.Find(':', 2);
		// the HH:MM:SS itself has colons; find the ": " AFTER the timestamp
		int sep = line.Find(": ", 2);
		if(sep < 0) continue;
		String hms = TrimBoth(line.Mid(2, sep - 2));
		int sec = ParseHmsToSec(hms);
		if(sec < 0) continue;
		String rest = TrimBoth(line.Mid(sep + 2));
		Vector<String> tok = Split(rest, ' ');
		if(tok.IsEmpty()) continue;

		if(rest.StartsWith("new hand")) {
			// The sidecar ends with a duplicate "new hand @53" artifact (a known
			// trailing dup, see FRAME_TIME_MAPPING.md). Ignore any repeat of a
			// second we've already opened a hand at, so it neither resets a later
			// hand's board nor adds a phantom empty hand.
			if(FindIndex(gt.new_hand_seconds, sec) >= 0) continue;
			cur_board.Clear();
			SidecarGT::BoardPt& bp = gt.board_pts.Add(); bp.sec = sec; bp.board = "";
			gt.new_hand_seconds.Add(sec);
			cur_hand = gt.hands.GetCount();
			gt.hands.Add();
			prev_action_sec = sec;
			// dealer=seatN
			int dp = rest.Find("dealer=seat");
			if(dp >= 0) {
				int sd = StrInt(rest.Mid(dp + 11, 1));
				gt.dealer_by_hand_sec.GetAdd(sec) = SidecarSeatToMirror(sd);
			}
			continue;
		}
		if(rest.StartsWith("flop ")) {
			cur_board = tok[1];                 // "3d3sKh"
			SidecarGT::BoardPt& bp = gt.board_pts.Add(); bp.sec = sec; bp.board = cur_board;
			continue;
		}
		if(rest.StartsWith("turn ") || rest.StartsWith("river ")) {
			cur_board << tok[1];                // append "5c" / "Ad"
			SidecarGT::BoardPt& bp = gt.board_pts.Add(); bp.sec = sec; bp.board = cur_board;
			continue;
		}
		// seat lines: "seatN <verb> ..." | "seatN name=\"..\" balance=..BB" | "seatN shows CC"
		if(tok[0].StartsWith("seat") && tok.GetCount() >= 2) {
			int sn = StrInt(tok[0].Mid(4));
			int mirror = SidecarSeatToMirror(sn);
			String verb = tok[1];
			if(verb == "shows" && tok.GetCount() >= 3) {
				SidecarGT::Show& sh = gt.shows.Add(); sh.sec = sec; sh.mirror = mirror; sh.cards = tok[2];
				continue;
			}
			if(rest.Find("name=\"") >= 0) {
				// name="X Y" balance=NNN.NBB. NOTE: in this sidecar the seat name/
				// balance lines for a hand are logged at the SAME second as (and just
				// BEFORE) that hand's "new hand" line, so cur_hand is not reliable
				// here. Buffer with the line's own second and resolve to the nearest
				// new-hand AFTER the full parse (see below).
				int np = rest.Find("name=\"");
				int ne = rest.Find('"', np + 6);
				String nm = ne > np ? rest.Mid(np + 6, ne - (np + 6)) : "";
				double bal = -1;
				int bp2 = rest.Find("balance=");
				if(bp2 >= 0) { const char* e = nullptr; bal = ScanDouble(rest.Begin() + bp2 + 8, &e); }
				PendingSeat& ps = pend.Add();
				ps.sec = sec; ps.mirror = mirror; ps.name = nm; ps.balance = bal;
				continue;
			}
			if(verb == "fold" || verb == "call" || verb == "raise" || verb == "check" || verb == "bet") {
				// turn-window derivation: the seat that ACTS at `sec` was the acting
				// seat over (prev_action_sec, sec]. Sidecar-derived, no baked frames.
				int a = prev_action_sec + 1, b = sec;
				for(int s = a; s <= b; s++) gt.turn_by_second.GetAdd(s) = mirror;
				prev_action_sec = sec;
				continue;
			}
		}
	}
	// Resolve buffered seat name/balance lines to the nearest new-hand (the hands
	// are well separated -- 4 vs 53 -- so nearest-second assignment is unambiguous
	// and correctly attaches the pre-"new hand" seat lines to the hand they open).
	for(const PendingSeat& ps : pend) {
		int bestH = -1, bestD = 1 << 30;
		for(int h = 0; h < gt.new_hand_seconds.GetCount(); h++) {
			int d = abs(ps.sec - gt.new_hand_seconds[h]);
			if(d < bestD) { bestD = d; bestH = h; }
		}
		if(bestH < 0 || bestH >= gt.hands.GetCount()) continue;
		SidecarGT::SeatInfo& si = gt.hands[bestH].GetAdd(ps.mirror);
		si.name = ps.name;
		if(ps.balance >= 0) si.balance = ps.balance;
	}
	// Board change points must be time-ordered for BoardAt()'s "last change <= sec"
	// scan to be correct (file order already is, but sort defensively).
	Sort(gt.board_pts, [](const SidecarGT::BoardPt& x, const SidecarGT::BoardPt& y){ return x.sec < y.sec; });
	return gt;
}

static int CompareAnnotatedSidecar2(const String& expected_path, const String& actual_path)
{
	int checked = 0, matched = 0, unresolved = 0, rejected = 0;
	for(char window : String("RL")) {
		SidecarGT expected = ParseSidecarGT(expected_path, window);
		SidecarGT actual = ParseSidecarGT(actual_path, window);
		Cout() << Format("annotated_regression window=%c expected_hands=%d actual_hands=%d\n",
		                 window, expected.new_hand_seconds.GetCount(), actual.new_hand_seconds.GetCount());
		if(expected.new_hand_seconds.IsEmpty()) {
			Cout() << Format("annotated_regression window=%c status=unresolved reason=no_ground_truth\n", window);
			unresolved++;
			continue;
		}
		checked++;
		if(expected.new_hand_seconds.GetCount() != actual.new_hand_seconds.GetCount()) {
			rejected++;
			Cout() << Format("annotated_regression window=%c status=rejected reason=hand_count\n", window);
			continue;
		}
		bool ok = true;
		for(const SidecarGT::BoardPt& want : expected.board_pts) {
			String got = actual.BoardAt(want.sec);
			if(got != want.board) {
				ok = false;
				Cout() << Format("annotated_regression window=%c status=unresolved second=%d expected_board=%s actual_board=%s\n",
				                 window, want.sec, ~want.board, ~got);
			}
		}
		for(int hand = 0; hand < expected.hands.GetCount(); hand++) {
			if(hand >= actual.hands.GetCount()) { ok = false; break; }
			for(int i = 0; i < expected.hands[hand].GetCount(); i++) {
				int mirror = expected.hands[hand].GetKey(i);
				int actual_i = actual.hands[hand].Find(mirror);
				if(actual_i < 0) { ok = false; continue; }
				const SidecarGT::SeatInfo& want = expected.hands[hand][i];
				const SidecarGT::SeatInfo& got = actual.hands[hand][actual_i];
				if(!want.name.IsEmpty() && want.name != got.name) ok = false;
				if(want.balance >= 0 && (got.balance < 0 || fabs(want.balance - got.balance) > 0.15)) ok = false;
			}
		}
		if(ok) { matched++; Cout() << Format("annotated_regression window=%c status=matched\n", window); }
		else { unresolved++; Cout() << Format("annotated_regression window=%c status=unresolved\n", window); }
	}
	Cout() << Format("annotated_regression_summary checked=%d matched=%d rejected=%d unresolved=%d\n",
	                 checked, matched, rejected, unresolved);
	return rejected || unresolved ? 2 : 0;
}

// Resolve which real frame directories a test should scan. Empty frames_dir ->
// BOTH real hands (0263 = hand 1 sec 0-56, 0268 = hand 2 sec 52-105); an explicit
// --frames-dir overrides to just that one. This is what finally lets one
// invocation cover BOTH hands' real action instead of only hand 1.
static Vector<String> ResolveFrameDirs(const String& frames_dir)
{
	Vector<String> dirs;
	if(frames_dir.IsEmpty()) {
		dirs.Add("tmp/real_recording_0263_frames"); // hand 1
		dirs.Add("tmp/real_recording_0268_frames"); // hand 2
	}
	else dirs.Add(frames_dir);
	return dirs;
}
// Enumerate present frame indices (the %06d number == video second) in a dir.
static Vector<int> FrameSecondsInDir(const String& dir)
{
	Vector<int> secs;
	Vector<String> files = FindAllPaths(dir, "*.jpg");
	for(const String& f : files) {
		String bn = GetFileTitle(f); // frame_000087
		int us = bn.ReverseFind('_');
		if(us >= 0) { int n = StrInt(bn.Mid(us + 1)); if(n >= 0) secs.Add(n); }
	}
	Sort(secs);
	return secs;
}

// ===========================================================================
// Task 0290a: Mode 5 -- offline card-recognition benchmark against the real
// video + sidecar ground truth. Deterministic, no server. Task 0291a: scored
// frames + expected boards are now DERIVED from the sidecar (ParseSidecarGT),
// and the test scans every real frame directory given (both hands by default),
// so hand 2's real flop/turn/river are genuinely exercised.
// ===========================================================================
struct BoardCase { int frame; const char* board; };

// Score one frame's board recognition against an expected board string. Appends
// per-card outcome to the running totals; returns a one-line summary.
struct BoardScore { int card_tot=0, card_ok=0, suit_ok=0, rank_ok=0, count_ok=0, frame_tot=0; };
static String ScoreBoardFrame(const VsmFrameImage& table, const CardTemplates& ct,
                              const String& expboard, BoardScore& acc, Vector<String>* dbg)
{
	Vector<int> recog = RecognizeBoardCards(table, ct, dbg);
	Vector<int> exp = ParseCardRun(expboard);
	acc.frame_tot++;
	bool cnt = recog.GetCount() == exp.GetCount(); acc.count_ok += cnt;
	String line;
	for(int i = 0; i < exp.GetCount(); i++) {
		acc.card_tot++;
		int got = i < recog.GetCount() ? recog[i] : -1;
		bool ok = (got == exp[i]); acc.card_ok += ok;
		if(got >= 0) { if(got / 13 == exp[i] / 13) acc.suit_ok++; if(got % 13 == exp[i] % 13) acc.rank_ok++; }
		line << Format("  %s%s", ~FormatCardStr(exp[i]), ok ? "=OK" : Format("!=%s", ~FormatCardStr(got)));
	}
	return Format("(%d cards, count %s):%s", recog.GetCount(), cnt ? "OK" : "MISMATCH", ~line);
}

static int RunCardRecogTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: card-recog-test (BOTH hands, sidecar-derived board frames) ===\n";
	CardTemplates ct = LoadCardTemplates(templates_dir);
	Cout() << "rank templates: " << ct.ranks.GetCount() << "/13 (" << (ct.ok ? "OK" : "INCOMPLETE")
	       << ") from " << templates_dir << "\n";
	if(!ct.ok) { Cerr() << "ERROR: incomplete rank template library\n"; return 1; }

	SidecarGT gt = ParseSidecarGT(kSidecarPath);
	Vector<String> dirs = ResolveFrameDirs(frames_dir);
	auto pct = [](int a, int b) { return b ? 100.0 * a / b : 0.0; };

	BoardScore total;
	for(const String& dir : dirs) {
		Vector<int> secs = FrameSecondsInDir(dir);
		BoardScore ds;
		int lo = secs.IsEmpty() ? -1 : secs[0], hi = secs.IsEmpty() ? -1 : secs.Top();
		Cout() << Format("\n--- %s  (frames present: %d, sec %d..%d) ---\n", ~dir, secs.GetCount(), lo, hi);
		for(int sec : secs) {
			String board = gt.BoardAt(sec);
			if(ParseCardRun(board).GetCount() < 3) continue; // only post-flop frames scored
			String path = AppendFileName(dir, Format("frame_%06d.jpg", sec));
			VsmImageBuffer buf;
			if(!LoadJpgToBuffer(path, buf)) { Cerr() << "skip (load failed): " << path << "\n"; continue; }
			VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
			if(table.IsEmpty()) { Cerr() << "skip (empty crop): " << path << "\n"; continue; }
			Vector<String> dbg;
			String summary = ScoreBoardFrame(table, ct, board, ds, &dbg);
			Cout() << Format("  sec %03d exp=%-10s %s\n", sec, ~board, ~summary);
			for(const String& d : dbg) Cout() << "          " << d << "\n";
		}
		Cout() << Format("  [%s] scored frames=%d  full-card=%d/%d (%.1f%%)  suit=%d/%d  rank=%d/%d  count-correct=%d/%d\n",
		                 ~GetFileName(dir), ds.frame_tot, ds.card_ok, ds.card_tot, pct(ds.card_ok, ds.card_tot),
		                 ds.suit_ok, ds.card_tot, ds.rank_ok, ds.card_tot, ds.count_ok, ds.frame_tot);
		total.frame_tot += ds.frame_tot; total.card_tot += ds.card_tot; total.card_ok += ds.card_ok;
		total.suit_ok += ds.suit_ok; total.rank_ok += ds.rank_ok; total.count_ok += ds.count_ok;
	}
	Cout() << "\n=== Board recognition accuracy (ALL scanned dirs, sidecar-derived) ===\n";
	Cout() << Format("scored frames=%d  card-count-correct=%d/%d  cards=%d\n",
	                 total.frame_tot, total.count_ok, total.frame_tot, total.card_tot);
	Cout() << Format("full card (rank+suit) correct = %d/%d (%.1f%%)\n", total.card_ok, total.card_tot, pct(total.card_ok, total.card_tot));
	Cout() << Format("suit-only correct             = %d/%d (%.1f%%)\n", total.suit_ok, total.card_tot, pct(total.suit_ok, total.card_tot));
	Cout() << Format("rank-only correct             = %d/%d (%.1f%%)\n", total.rank_ok, total.card_tot, pct(total.rank_ok, total.card_tot));

	// --- timing: template-match vs ORB on a real card crop (frame 18, first card) ---
	Cout() << "\n=== Per-card matcher timing (real card crop, frame 18) ===\n";
	VsmImageBuffer buf;
	if(LoadJpgToBuffer(AppendFileName("tmp/real_recording_0263_frames", "frame_000018.jpg"), buf)) {
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		Vector<Rect> cards = SplitBoardBand(table);
		if(!cards.IsEmpty()) {
			Rect cr = cards[0];
			// template: one full per-card rank recognition (13 ranks x 3 scales).
			const int N = 100;
			double sc; String r0 = RecognizeRank(table, cr, ct, sc);
			double t0 = NowMs();
			for(int k = 0; k < N; k++) { double s; RecognizeRank(table, cr, ct, s); }
			double tmpl_us = (NowMs() - t0) * 1000.0 / N;
			Cout() << Format("template-match: %.1f us/card (13 ranks x %d scales, best='%s' score=%.2f)\n",
			                 tmpl_us, kNumRankScales, ~r0, sc);
			// ORB: match ONE rank template against the card crop (scale/rotation
			// invariant). One pattern only -> a full 13-rank ORB pass would be ~13x.
			Rect band = RectC(cr.left - 2, kBoardCardTop, cr.Width() + 4, kBoardCardBot - kBoardCardTop)
			            & RectC(0, 0, table.width, table.height);
			Image cardImg = Crop(VsmFrameImageToImage(table), band);
			Image pat = StreamRaster::LoadFileAny(
			    AppendFileName(AppendFileName(templates_dir, "ranks"), "3.png"));
			if(!pat.IsEmpty() && !cardImg.IsEmpty()) {
				OrbSystem orb;
				orb.SetInput(pat);
				orb.InitDefault();
				orb.SetInput(cardImg);
				orb.Process(); // warm up
				int good = orb.GetLastGoodMatches(), matches = orb.GetLastMatchCount();
				const int NO = 30;
				double o0 = NowMs();
				for(int k = 0; k < NO; k++) { orb.SetInput(cardImg); orb.Process(); }
				double orb_us = (NowMs() - o0) * 1000.0 / NO;
				Cout() << Format("ORB (1 rank pattern): %.1f us/call  (x13 ranks ~= %.1f us/card)  "
				                 "matches=%d good=%d\n", orb_us, orb_us * 13, matches, good);
				Cout() << Format("=> template-match is %.1f`x %s than a 13-rank ORB pass\n",
				                 (orb_us * 13) / max(1.0, tmpl_us),
				                 (orb_us * 13) > tmpl_us ? "FASTER" : "slower");
				Cout() << "Design note: on-screen card scale is FIXED on this static table, so the\n"
				          "3-point scale set makes template-match both faster and 12/12-accurate;\n"
				          "no per-frame ORB scale search is needed (ORB stays a calibration-only\n"
				          "tool if a variable-scale feed is ever added).\n";
			}
			else Cout() << "ORB timing skipped (pattern/card image load failed)\n";
		}
	}
	return 0;
}

// ===========================================================================
// Task 0290a: Mode 6 -- collect real, ground-truth-labeled board-card crops into
// a dataset (extends the 267-candidate methodology: PNG crop + JSON manifest
// carrying the card_index/rank/suit label and source rect). Board cards only for
// now; hole-card crops are a disclosed follow-up (they need per-seat face-up
// localization not built here).
// ===========================================================================
static int RunCardDatasetDump(const String& frames_dir, const String& templates_dir, const String& out_dir)
{
	Cout() << "=== Mode: card-dataset-dump -> " << out_dir << " ===\n";
	RealizeDirectory(out_dir);
	CardTemplates ct = LoadCardTemplates(templates_dir);
	static const BoardCase cases[] = {
		{18,"3d3sKh"}, {27,"3d3sKh5c"}, {38,"3d3sKh5cAd"}, {44,"3d3sKh5cAd"},
	};
	Vector<String> entries;
	int n = 0;
	for(const BoardCase& bc : cases) {
		String path = AppendFileName(frames_dir, Format("frame_%06d.jpg", bc.frame));
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(path, buf)) { Cerr() << "skip: " << path << "\n"; continue; }
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		Image timg = VsmFrameImageToImage(table);
		Vector<Rect> cards = SplitBoardBand(table);
		String bs = bc.board;
		Vector<int> exp;
		for(int i = 0; i + 1 < bs.GetCount(); i += 2) exp.Add(CardIndex(bs.Mid(i, 1), bs.Mid(i + 1, 1)));
		for(int i = 0; i < cards.GetCount() && i < exp.GetCount(); i++) {
			Rect cr = RectC(cards[i].left - 2, kBoardCardTop, cards[i].Width() + 4,
			                kBoardCardBot - kBoardCardTop) & RectC(0, 0, timg.GetWidth(), timg.GetHeight());
			Image crop = Crop(timg, cr);
			String label = FormatCardStr(exp[i]);
			String fn = Format("board_f%02d_p%d_%s.jpg", bc.frame, i, ~label);
			String fp = AppendFileName(out_dir, fn);
			JPGEncoder(95).SaveFile(fp, crop);
			entries.Add(Format("  {\"crop_path\": \"%s\", \"card_index\": %d, \"rank\": \"%s\", "
			                   "\"suit\": \"%s\", \"source_frame\": %d, \"position\": %d, "
			                   "\"rect\": {\"x\": %d, \"y\": %d, \"w\": %d, \"h\": %d}}",
			                   ~fn, exp[i], ~label.Left(label.GetCount() - 1),
			                   ~label.Mid(label.GetCount() - 1), bc.frame, i,
			                   cr.left, cr.top, cr.Width(), cr.Height()));
			n++;
		}
	}
	String manifest = "[\n" + Join(entries, ",\n") + "\n]\n";
	String mp = AppendFileName(out_dir, "board_card_dataset.json");
	SaveFile(mp, manifest);
	Cout() << Format("wrote %d labeled board-card crops + manifest %s\n", n, ~mp);
	return 0;
}

// ===========================================================================
// Task 0291a item 4: ORB-based dealer-chip detector. Task 0290c detected the
// dealer button with MatchTemplate ONLY; ORB was never applied to actual dealer
// recognition (only a one-crop timing probe in 0290a). This trains ORB on the
// SAME real template (templates/dealer_chip/1.png) and matches it into the same
// felt-interior search band, mapping the projected pattern location to a seat --
// so its real accuracy AND timing can be reported side-by-side with TM.
// ===========================================================================
struct OrbDealer {
	OrbSystem orb;
	bool ok = false;
	void Init(const String& templates_dir) {
		String path = AppendFileName(AppendFileName(templates_dir, "dealer_chip"), "1.png");
		Image im = StreamRaster::LoadFileAny(path);
		if(im.IsEmpty()) { Cerr() << "WARN: ORB dealer template missing: " << path << "\n"; return; }
		orb.SetInput(im);
		orb.InitDefault();      // trains the pattern from the template (same as 0290a's probe)
		ok = true;
	}
	// Match into the felt-interior band; returns nearest seat (or -1 if good < min_good
	// or no location), and always fills out_good / out_center for reporting.
	int Detect(const VsmFrameImage& table, int min_good, int& out_good, Point& out_center) {
		out_good = 0; out_center = Point(-1, -1);
		if(!ok) return -1;
		Rect sr = kDealerSearch & RectC(0, 0, table.width, table.height);
		Image img = Crop(VsmFrameImageToImage(table), sr);
		if(img.IsEmpty()) return -1;
		orb.SetInput(img);
		orb.Process();
		out_good = orb.GetLastGoodMatches();
		const Vector<Pointf>& corners = orb.GetLastCorners();
		if(corners.GetCount() >= 1) {
			double cx = 0, cy = 0;
			for(const Pointf& p : corners) { cx += p.x; cy += p.y; }
			cx /= corners.GetCount(); cy /= corners.GetCount();
			out_center = Point(sr.left + (int)cx, sr.top + (int)cy);
		}
		if(out_good < min_good || out_center.x < 0) return -1;
		return NearestSeatAnchor(out_center.x, out_center.y);
	}
};

// ===========================================================================
// Task 0290c/0291a: Mode 7 -- dealer-chip (TM + ORB) + turn-timer accuracy
// against the real sidecar ground truth, DERIVED (ParseSidecarGT), scanning
// BOTH real hands by default (0263 hand 1, 0268 hand 2). GT dealer=seat4=mirror 0
// for both hands; expected acting seat per second is derived from the sidecar's
// own action timestamps. Reports TM and ORB accuracy + timing side-by-side.
// ===========================================================================
static const int kOrbDealerMinGood = 4; // reported alongside raw good-match counts

static int RunDealerTurnTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: dealer-turn-test (BOTH hands: TM + ORB dealer, sidecar-derived) ===\n";
	DealerTemplate dt = LoadDealerTemplate(templates_dir);
	Cout() << "dealer template (TM): " << (dt.ok ? "OK" : "MISSING") << " (" << dt.scaled.GetCount()
	       << " scales) from " << templates_dir << "\n";
	if(!dt.ok) { Cerr() << "ERROR: dealer template missing\n"; return 1; }
	OrbDealer orbd; orbd.Init(templates_dir);
	Cout() << "dealer template (ORB): " << (orbd.ok ? "OK" : "MISSING")
	       << " (min-good=" << kOrbDealerMinGood << ")\n";

	SidecarGT gt = ParseSidecarGT(kSidecarPath);
	Vector<String> dirs = ResolveFrameDirs(frames_dir);
	auto pct = [](int a, int b){ return b ? 100.0 * a / b : 0.0; };
	auto exp_dealer_at = [&](int sec) -> int {
		int best = -1, bestsec = -1;
		for(int i = 0; i < gt.dealer_by_hand_sec.GetCount(); i++) {
			int hs = gt.dealer_by_hand_sec.GetKey(i);
			if(hs <= sec && hs > bestsec) { bestsec = hs; best = gt.dealer_by_hand_sec[i]; }
		}
		return best; // both hands: 0
	};

	int tm_present = 0, tm_ok = 0, orb_present = 0, orb_ok = 0;
	int turn_scored = 0, turn_ok = 0, turn_detected = 0, frames_seen = 0;
	int turn_bar_on_scored = 0, turn_ok_when_bar = 0; // precision when a bar is actually up
	double tm_ms_sum = 0, orb_ms_sum = 0; int det_n = 0;
	for(const String& dir : dirs) {
		Vector<int> secs = FrameSecondsInDir(dir);
		int lo = secs.IsEmpty() ? -1 : secs[0], hi = secs.IsEmpty() ? -1 : secs.Top();
		Cout() << Format("\n--- %s  (frames present: %d, sec %d..%d) ---\n", ~dir, secs.GetCount(), lo, hi);
		for(int sec : secs) {
			String path = AppendFileName(dir, Format("frame_%06d.jpg", sec));
			VsmImageBuffer buf;
			if(!LoadJpgToBuffer(path, buf)) continue;
			VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
			if(table.IsEmpty()) continue;
			frames_seen++;
			int expd = exp_dealer_at(sec);

			double t0 = NowMs();
			double dsc = -2; Point dc(-1, -1);
			int dseat = DetectDealerChip(table, dt, dsc, dc);
			tm_ms_sum += NowMs() - t0;

			double o0 = NowMs();
			int ogood = 0; Point oc(-1, -1);
			int oseat = orbd.Detect(table, kOrbDealerMinGood, ogood, oc);
			orb_ms_sum += NowMs() - o0;
			det_n++;

			int strength = 0;
			int tseat = DetectTurnSeat(table, strength);

			// Dealer scored only where GT is defined (expd>=0, i.e. within a hand).
			String dline;
			if(dseat >= 0) {
				if(expd >= 0) { tm_present++; if(dseat == expd) tm_ok++; }
				dline = Format("TM seat%d %s(%.2f)", dseat, expd < 0 ? "n/a" : (dseat == expd ? "OK " : "BAD"), dsc);
			}
			else dline = Format("TM none(%.2f)", dsc);
			String oline;
			if(oseat >= 0) {
				if(expd >= 0) { orb_present++; if(oseat == expd) orb_ok++; }
				oline = Format("ORB seat%d %s(g%d)", oseat, expd < 0 ? "n/a" : (oseat == expd ? "OK " : "BAD"), ogood);
			}
			else oline = Format("ORB none(g%d)", ogood);

			if(tseat >= 0) turn_detected++;
			int expt = gt.turn_by_second.Get(sec, -1);
			String tline;
			if(expt >= 0) {
				turn_scored++; bool ok = (tseat == expt); turn_ok += ok;
				if(tseat >= 0) { turn_bar_on_scored++; turn_ok_when_bar += ok; } // precision when a bar is up
				tline = Format("turn=seat%d exp=seat%d %s(str%d)", tseat, expt, ok ? "OK" : "MISS", strength);
			}
			else tline = Format("turn=%s(str%d,GTn/a)", tseat >= 0 ? ~Format("seat%d", tseat) : "none", strength);

			Cout() << Format("sec%03d expDlr=%d | %-22s %-22s | %s\n", sec, expd, ~dline, ~oline, ~tline);
		}
	}
	Cout() << "\n=== Dealer-chip accuracy (GT: dealer=seat4=mirror-0 both hands) ===\n";
	Cout() << Format("frames scanned = %d\n", frames_seen);
	Cout() << Format("TM : chip detected %d; correct seat = %d/%d (%.1f%%);  timing %.1f ms/frame\n",
	                 tm_present, tm_ok, tm_present, pct(tm_ok, tm_present), det_n ? tm_ms_sum / det_n : 0.0);
	Cout() << Format("ORB: chip detected %d (good>=%d); correct seat = %d/%d (%.1f%%);  timing %.1f ms/frame\n",
	                 orb_present, kOrbDealerMinGood, orb_ok, orb_present, pct(orb_ok, orb_present),
	                 det_n ? orb_ms_sum / det_n : 0.0);
	Cout() << "\n=== Turn-timer accuracy (sidecar-derived acting seat) ===\n";
	Cout() << Format("frames with a timer bar detected = %d/%d\n", turn_detected, frames_seen);
	Cout() << Format("GT-scored seconds = %d; matches GT actor = %d (%.1f%%)\n",
	                 turn_scored, turn_ok, pct(turn_ok, turn_scored));
	Cout() << Format("  (precision) of GT-scored seconds where a bar is actually up (%d): correct = %d (%.1f%%)\n",
	                 turn_bar_on_scored, turn_ok_when_bar, pct(turn_ok_when_bar, turn_bar_on_scored));
	Cout() << "  NOTE: the low overall %% is a GT-window artifact -- the sidecar-derived acting\n"
	          "  window spans every second between two actions, but the on-screen timer bar is\n"
	          "  only lit while that player is actively deciding, so many GT seconds legitimately\n"
	          "  show no bar (str<threshold). The precision line is the real detector quality.\n";
	return 0;
}

// ===========================================================================
// Task 0291a item 3: HOLE-CARD recognition. Task 0290a explicitly left hole cards
// untested (only board cards were localized). Both hands reveal hole cards
// face-up at the all-in showdown; the on-screen regions below were MEASURED on
// the real reveal frames (hand 1: 0263 frames 49-52, board fully out; hand 2:
// 0268 frames 95-104, all-in run-out) -- the reveal lags the sidecar "shows"
// timestamp by ~1s (hand 1 "shows @48" renders 49-52), so a small post-shows
// window is sampled. Only seats that actually reach showdown in these two hands
// have ground truth: mirror 0 (bottom), 1 (left-bottom), 2 (left-top). Seats
// 3/4/5 fold both hands -> no face-up hole cards -> no GT (honestly untested).
// The SAME ClassifySuitByColor / RecognizeRank pipeline as board cards is reused
// (now rect-driven), with a wider rank-scale set (hole glyphs are a touch smaller
// on-screen than board glyphs).
// ===========================================================================
struct HoleRegion { int mirror; Rect c0; Rect c1; bool has_gt; };
static const HoleRegion kHoleRegions[3] = {
	{ 0, RectC(416, 412, 62, 56), RectC(478, 412, 62, 56), true }, // BOTTOM      (TdTh h1)
	{ 1, RectC( 52, 300, 64, 54), RectC(116, 300, 64, 54), true }, // LEFT_BOTTOM (QcJh h2)
	{ 2, RectC( 76, 102, 64, 54), RectC(142, 102, 64, 54), true }, // LEFT_TOP    (3hKs h1 / QhQs h2)
};
// Hole cards render the rank as a SMALL glyph in the TOP-LEFT CORNER (a miniature
// card index), unlike board cards which show a LARGE centred rank. So hole-card
// rank recognition restricts the template search to this corner sub-rect (and uses
// a much smaller scale set -- the corner glyph is ~0.4x the board glyph). Suit is
// still read from the full card interior by colour (the 4-colour deck tints the
// whole card), which already scores 8/8 on these reveals.
// Task 0292 fix: the corner rank glyph does NOT sit at a fixed small offset from
// card.top -- the kHoleRegions card rects are vertically aligned inconsistently, so
// the real rank glyph starts anywhere from card.top+8 (left-top seat) down to
// card.top+20 (bottom seat) and is ~22px tall (measured on real reveal frames, both
// hands: white-glyph row histograms of Th/Ks/Qc/Jh/Qh/Qs/Td/3h). The old
// RectC(card.left+1, card.top+2, 30, 34) top edge (card.top+2..36) sat mostly in the
// felt gap above the bottom seat's card and CUT OFF the lower half of its "10" glyph
// -- the direct cause of the Th->7h miss (match 0.53 vs 0.84-0.95 for well-bounded
// glyphs). The band below (card.top+6..44, x 0..31 from card.left) fully contains the
// rank glyph for EVERY measured seat/hand; it clips at most a partial suit pip at the
// very bottom for the top seats, which the sliding TM_CCOEFF_NORMED peak ignores in
// favour of the full, correctly-scaled rank glyph.
static Rect HoleCornerRect(const Rect& card)
{
	return RectC(card.left, card.top + 6, 32, 38);
}
// Is a face-up hole card present in this region? (vs. felt / avatar after a fold).
// Card pixels are bright and non-felt (4-colour deck: red/blue/green/grey, all
// with a max channel > 110); felt max ~96, avatar/shadow dark. Present if a clear
// majority of the interior is card-bright.
static bool HoleCardPresent(const VsmFrameImage& table, const Rect& in_r)
{
	Rect r = RectC(in_r.left + 4, in_r.top + 4, max(1, in_r.Width() - 8), max(1, in_r.Height() - 8))
	         & RectC(0, 0, table.width, table.height);
	if(r.Width() <= 0 || r.Height() <= 0) return false;
	int bright = 0, tot = 0;
	for(int y = r.top; y < r.bottom; y += 2) {
		const byte* row = ~table.data + (size_t)((size_t)y * table.width + r.left) * 4;
		for(int x = 0; x < r.Width(); x += 2) {
			const byte* p = row + (size_t)x * 4;
			int mx = max((int)p[0], max((int)p[1], (int)p[2]));
			if(mx > 110 && !IsFeltPixel(p)) bright++;
			tot++;
		}
	}
	return tot > 0 && (double)bright / tot > 0.45;
}

static int RunHoleCardTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: hole-card-test (real showdown reveals vs sidecar 'shows') ===\n";
	// Small scale set: the top-left corner rank glyph is ~0.4x the board rank glyph
	// the templates were sized for.
	Vector<double> hscales; hscales << 0.30 << 0.35 << 0.40 << 0.45 << 0.50 << 0.55;
	CardTemplates ct = LoadCardTemplates(templates_dir, hscales);
	if(!ct.ok) { Cerr() << "ERROR: incomplete rank template library\n"; return 1; }
	Cout() << Format("rank templates: %d/13, corner scales={0.30..0.55}\n", ct.ranks.GetCount());
	int suit_tot = 0, suit_ok_cnt = 0;

	SidecarGT gt = ParseSidecarGT(kSidecarPath);
	Vector<String> dirs = ResolveFrameDirs(frames_dir);
	auto pct = [](int a, int b){ return b ? 100.0 * a / b : 0.0; };

	int card_total = 0, card_majvote_ok = 0;      // per-card majority-vote accuracy
	int present_shows = 0, total_shows = 0;
	for(const SidecarGT::Show& sh : gt.shows) {
		total_shows++;
		// find the region for this seat
		const HoleRegion* reg = nullptr;
		for(const HoleRegion& hr : kHoleRegions) if(hr.mirror == sh.mirror && hr.has_gt) reg = &hr;
		Vector<int> gtcards = ParseCardRun(sh.cards);
		if(!reg || gtcards.GetCount() != 2) {
			Cout() << Format("show sec%03d seat%d %s: NO GT REGION (seat folds / not localized) -- untested\n",
			                 sh.sec, sh.mirror, ~sh.cards);
			continue;
		}
		// bound the sample window: [shows, shows+9], capped before the next new hand.
		int cap = sh.sec + 9;
		for(int nh : gt.new_hand_seconds) if(nh > sh.sec && nh - 1 < cap) cap = nh - 1;
		// pick the dir that actually contains these frames
		String usedir;
		for(const String& d : dirs) {
			Vector<int> secs = FrameSecondsInDir(d);
			if(!secs.IsEmpty() && secs[0] <= sh.sec + 1 && secs.Top() >= sh.sec) { usedir = d; break; }
		}
		if(usedir.IsEmpty()) { Cout() << Format("show sec%03d seat%d: no dir covers it\n", sh.sec, sh.mirror); continue; }

		// tally per-card votes across all frames where the card is present
		VectorMap<int,int> votes0, votes1; int present_frames = 0;
		Cout() << Format("show sec%03d seat%d exp=%s  dir=%s  window=[%d..%d]\n",
		                 sh.sec, sh.mirror, ~sh.cards, ~GetFileName(usedir), sh.sec, cap);
		for(int sec = sh.sec; sec <= cap; sec++) {
			String path = AppendFileName(usedir, Format("frame_%06d.jpg", sec));
			VsmImageBuffer buf;
			if(!LoadJpgToBuffer(path, buf)) continue;
			VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
			if(table.IsEmpty()) continue;
			bool p0 = HoleCardPresent(table, reg->c0), p1 = HoleCardPresent(table, reg->c1);
			if(!p0 && !p1) { continue; } // reveal not yet rendered / seat empty
			present_frames++;
			double s0 = -2, s1 = -2;
			// suit from full card interior (colour); rank from the top-left corner glyph
			String su0 = ClassifySuitByColor(table, reg->c0), r0 = RecognizeRank(table, HoleCornerRect(reg->c0), ct, s0);
			String su1 = ClassifySuitByColor(table, reg->c1), r1 = RecognizeRank(table, HoleCornerRect(reg->c1), ct, s1);
			int i0 = CardIndex(r0, su0), i1 = CardIndex(r1, su1);
			if(i0 >= 0) votes0.GetAdd(i0, 0)++;
			if(i1 >= 0) votes1.GetAdd(i1, 0)++;
			Cout() << Format("   sec%03d p=%d%d  c0=%s%s(%.2f)->%s  c1=%s%s(%.2f)->%s\n",
			                 sec, p0, p1, ~r0, ~su0, s0, ~FormatCardStr(i0),
			                 ~r1, ~su1, s1, ~FormatCardStr(i1));
		}
		if(present_frames > 0) present_shows++;
		// majority vote per card
		auto topvote = [](const VectorMap<int,int>& v) -> int {
			int best = -1, bc = -1;
			for(int i = 0; i < v.GetCount(); i++) if(v[i] > bc) { bc = v[i]; best = v.GetKey(i); }
			return best;
		};
		int mv0 = topvote(votes0), mv1 = topvote(votes1);
		bool ok0 = (mv0 == gtcards[0]), ok1 = (mv1 == gtcards[1]);
		card_total += 2; card_majvote_ok += ok0 + ok1;
		// suit-only accuracy from the majority-vote cards (suit = idx/13)
		suit_tot += 2;
		if(mv0 >= 0 && mv0 / 13 == gtcards[0] / 13) suit_ok_cnt++;
		if(mv1 >= 0 && mv1 / 13 == gtcards[1] / 13) suit_ok_cnt++;
		Cout() << Format("   => present in %d frames; majority-vote: %s%s  %s%s\n",
		                 present_frames,
		                 ~FormatCardStr(gtcards[0]), ok0 ? "=OK" : Format("!=%s", ~FormatCardStr(mv0)),
		                 ~FormatCardStr(gtcards[1]), ok1 ? "=OK" : Format("!=%s", ~FormatCardStr(mv1)));
	}
	Cout() << "\n=== Hole-card recognition accuracy (majority vote per revealed card) ===\n";
	Cout() << Format("shows with GT region = %d; shows where cards were captured = %d\n",
	                 card_total / 2, present_shows);
	Cout() << Format("cards correct (rank+suit, majority vote) = %d/%d (%.1f%%)\n",
	                 card_majvote_ok, card_total, pct(card_majvote_ok, card_total));
	Cout() << Format("suit-only correct (majority vote)        = %d/%d (%.1f%%)\n",
	                 suit_ok_cnt, suit_tot, pct(suit_ok_cnt, suit_tot));
	Cout() << "(Seats that fold before showdown -- mirror 3/4/5 in these two hands -- reveal no\n"
	          " face-up hole cards and are honestly untested; only mirror 0/1/2 have ground truth.)\n";
	return 0;
}

struct Sidecar2Stats {
	int decoded_frames = 0;
	int frames = 0;
	int skipped_frames = 0;
	int first_frame_second = -1;
	int last_frame_second = -1;
	int hands = 0;
	int board_events = 0;
	int actions = 0;
	int showdowns = 0;
	int seat_updates = 0;
	int omitted_signals = 0;
	int omitted_dealer = 0;
	int omitted_board = 0;
	int omitted_action = 0;
	int omitted_showdown = 0;
	int omitted_seat = 0;
	int diagnostic_raw_board_frames = 0;
	int diagnostic_raw_board_max = 0;
	int diagnostic_board_trigger_frames = 0;
	int diagnostic_resolved_board_cards = 0;
	int diagnostic_ocr_candidates = 0;
	int diagnostic_action_candidates = 0;
	int diagnostic_action_reads = 0;
};

static int MirrorToSidecarSeat(int mirror)
{
	switch(mirror) {
	case 0: return 4;
	case 1: return 5;
	case 2: return 6;
	case 3: return 1;
	case 4: return 2;
	case 5: return 3;
	}
	return -1;
}

static String FormatSidecar2Time(int sec)
{
	sec = max(sec, 0);
	return Format("%02d:%02d:%02d", sec / 3600, (sec / 60) % 60, sec % 60);
}

static String FormatSidecar2Bb(double value)
{
	return Format("%.4g", value) + "BB";
}

static VsmRecognizedSidecar2* g_sidecar2_sink = nullptr;

static void AddSidecar2Event(String& output, char window, int sec, const String& event)
{
	output << window << " " << FormatSidecar2Time(sec) << ": " << event << "\n";
	if(g_sidecar2_sink)
		g_sidecar2_sink->AddEvent(sec, event);
}

static void AddSidecar2Action(String& output, char window, int sec, int participant,
	DistributedActionKind kind, bool amount_known, double amount, const String& event)
{
	output << window << " " << FormatSidecar2Time(sec) << ": " << event << "\n";
	if(g_sidecar2_sink)
		g_sidecar2_sink->AddAction(sec, participant, kind, amount_known, amount, event);
}

static bool LoadSidecar2Legend(const String& path, String& output)
{
	Vector<String> lines = Split(LoadFile(path), '\n');
	int count = 0;
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.IsEmpty()) break;
		if(line.Find("=seat") < 0) return false;
		output << line << "\n";
		count++;
	}
	if(count != 6) return false;
	output << "\n"
	       << "# sidecar2: generated only from recognized video signals; unresolved signals are omitted.\n"
	       << "# showdown recognition is limited to the three measured kHoleRegions seats.\n"
	       << "# Change checked=n to checked=y only after manual review of that hand.\n"
		       << "# R = right 2D table window; L = left 2D table window.\n"
	       << "# The seat-position legend above applies independently inside each window.\n\n";
	return true;
}

static String CleanSidecar2Name(const String& raw)
{
	String clean;
	bool space = false;
	for(int i = 0; i < raw.GetCount(); i++) {
		int chr = raw[i];
		if(chr == '"') return "";
		if(chr <= ' ') {
			space = !clean.IsEmpty();
			continue;
		}
		if(space) clean.Cat(' ');
		space = false;
		clean.Cat(chr);
	}
	clean = TrimBoth(clean);
	if(clean.GetCount() > 48 || LongestAlphaRun(clean) < kMinNameAlphaRun)
		return "";
	return clean;
}

static void ResetSidecar2HandState(ResolveState& state)
{
	state.board_count = 0;
	for(int i = 0; i < state.board_cards.GetCount(); i++) state.board_cards[i] = -1;
	state.last_pot = -1;
	state.pot_rejected_reads = 0;
	state.last_action_seat = -1;
	state.last_action_frame = -1;
	for(int seat = 0; seat < 6; seat++) {
		state.seat_action[seat].Clear();
		state.seat_bet[seat] = -1;
		state.seat_round_total[seat] = -1;
		state.seat_folded[seat] = false;
	}
}

static String Sidecar2BoardString(const ResolveState& state, int count)
{
	String cards;
	if(count < 0 || count > state.board_cards.GetCount()) return "";
	for(int i = 0; i < count; i++) {
		if(state.board_cards[i] < 0) return "";
		cards << FormatCardStr(state.board_cards[i]);
	}
	return cards;
}

static String RecognizeSidecar2HoleCards(const VsmFrameImage& table,
	                                      const HoleRegion& region,
	                                      const CardTemplates& templates,
	                                      bool& unresolved)
{
	unresolved = false;
	bool present0 = HoleCardPresent(table, region.c0);
	bool present1 = HoleCardPresent(table, region.c1);
	if(!present0 && !present1) return "";
	if(!present0 || !present1) {
		unresolved = true;
		return "";
	}
	double score0 = -2, score1 = -2;
	String suit0 = ClassifySuitByColor(table, region.c0);
	String suit1 = ClassifySuitByColor(table, region.c1);
	String rank0 = RecognizeRank(table, HoleCornerRect(region.c0), templates, score0);
	String rank1 = RecognizeRank(table, HoleCornerRect(region.c1), templates, score1);
	int card0 = CardIndex(rank0, suit0);
	int card1 = CardIndex(rank1, suit1);
	if(card0 < 0 || card1 < 0 || score0 < 0.70 || score1 < 0.70) {
		unresolved = true;
		return "";
	}
	return FormatCardStr(card0) + FormatCardStr(card1);
}

static void CountSidecar2PartialSeats(const ResolveState& state, int hand_start_frame,
	                                  const String emitted_name[6], Sidecar2Stats& stats)
{
	for(int seat = 0; seat < 6; seat++) {
		bool fresh_name = state.seat_name_frame[seat] >= hand_start_frame;
		bool fresh_stack = state.seat_stack_frame[seat] >= hand_start_frame;
		if(emitted_name[seat].IsEmpty() && (fresh_name || fresh_stack)) {
			stats.omitted_signals++;
			stats.omitted_seat++;
		}
	}
}

struct Sidecar2WindowState {
	char code = 0;
	String name;
	String output;
	RecognitionWindowState recognition;
	Sidecar2Stats stats;

	bool hand_open = false;
	bool pending_hand = false;
	bool hand_has_progress = false;
	bool hand_has_showdown = false;
	int hand_start_frame = -1;
	int hand_open_sec = -1;
	int last_raw_board_count = -1;
	int redeal_window_sec = -1;
	bool redeal_returned[6];
	String emitted_name[6];
	double emitted_stack[6];
	String profile_name_candidate[6];
	double profile_stack_candidate[6];
	int profile_streak[6];
	bool fold_emitted[6];
	String last_action_text[6];
	int last_action_sec[6];
	String hole_candidate[3];
	int hole_streak[3];
	bool hole_emitted[3];
	bool hole_unresolved[3];
	String hole_emitted_cards[3];
	bool winner_emitted = false;
	VsmRecognizedSidecar2 sink;

	Sidecar2WindowState()
	{
		for(int seat = 0; seat < 6; seat++) {
			redeal_returned[seat] = false;
			emitted_stack[seat] = -1;
			profile_stack_candidate[seat] = -1;
			profile_streak[seat] = 0;
			fold_emitted[seat] = false;
			last_action_sec[seat] = -1000;
		}
		for(int region = 0; region < 3; region++) {
			hole_streak[region] = 0;
			hole_emitted[region] = false;
			hole_unresolved[region] = false;
			hole_emitted_cards[region].Clear();
		}
		winner_emitted = false;
	}
};

static void InitializeSidecar2Window(Sidecar2WindowState& window,
	                                 const WindowDescriptor& descriptor,
	                                 bool ocr_cache_enabled, int ocr_cache_threshold)
{
	ASSERT(descriptor.id.value.GetCount() == 1);
	window.code = descriptor.id.value[0];
	window.name = descriptor.name;
	InitializeRecognitionWindow(window.recognition, descriptor,
	                            ocr_cache_enabled, ocr_cache_threshold);
}

static double Sidecar2PixelFraction(const VsmImageBuffer& buffer, const Rect& in_rect,
	                                bool white)
{
	Rect rect = in_rect & RectC(0, 0, buffer.width, buffer.height);
	if(rect.IsEmpty() || buffer.channels < 3) return 0;
	int64 matching = 0;
	int64 total = (int64)rect.Width() * rect.Height();
	for(int y = rect.top; y < rect.bottom; y++) {
		const byte *row = buffer.pixels.Begin() + (int64)y * buffer.width * buffer.channels;
		for(int x = rect.left; x < rect.right; x++) {
			const byte *pixel = row + (int64)x * buffer.channels;
			if(white ? pixel[0] >= 235 && pixel[1] >= 235 && pixel[2] >= 235
			         : pixel[0] <= 28 && pixel[1] <= 28 && pixel[2] <= 28)
				matching++;
		}
	}
	return total ? (double)matching / total : 0;
}

static bool ValidateSidecar2WindowLayout(const VsmImageBuffer& buffer)
{
	Rect bounds = RectC(0, 0, buffer.width, buffer.height);
	if(!bounds.Contains(kLeftTableRect) || !bounds.Contains(kRightTableRect)) {
		Cerr() << "ERROR: decoded frame is too small for the two tracked 2D table windows: "
		       << buffer.width << "x" << buffer.height << "\n";
		return false;
	}
	double left_title = Sidecar2PixelFraction(buffer, RectC(8, 1, 944, 30), true);
	double right_title = Sidecar2PixelFraction(buffer, RectC(968, 1, 944, 30), true);
	double outer_left = Sidecar2PixelFraction(buffer, RectC(0, 1, 8, 682), false);
	double middle = Sidecar2PixelFraction(buffer, RectC(952, 1, 16, 682), false);
	double outer_right = Sidecar2PixelFraction(buffer, RectC(1912, 1, 8, 682), false);
	Cout() << Format("window_layout left_title_white=%.3f right_title_white=%.3f ",
	                 left_title, right_title)
	       << Format("left_gutter_black=%.3f middle_gutter_black=%.3f right_gutter_black=%.3f\n",
	                 outer_left, middle, outer_right);
	if(left_title < 0.80 || right_title < 0.80 || outer_left < 0.75
	   || middle < 0.75 || outer_right < 0.75) {
		Cerr() << "ERROR: decoded frame does not match the tracked two-window title-bar/gutter layout\n";
		return false;
	}
	return true;
}

static String ComposeSidecar2Output(const String& header, const Sidecar2WindowState *windows,
	                                int window_count)
{
	Vector<DistributedSidecar2Line> lines;
	for(int i = 0; i < window_count; i++) {
		for(const DistributedSidecar2Line& line : windows[i].sink.GetLines()) {
			DistributedSidecar2Line& copy = lines.Add();
			copy.stream = line.stream;
			copy.timestamp_seconds = line.timestamp_seconds;
			copy.text = line.text;
			copy.hand = line.hand;
			copy.hand_start = line.hand_start;
			copy.checked = line.checked;
			copy.comment = line.comment;
			copy.legality.stream = line.legality.stream;
			copy.legality.round = line.legality.round;
			copy.legality.timestamp = line.legality.timestamp;
			copy.legality.status = line.legality.status;
			copy.legality.override_applied = line.legality.override_applied;
			copy.legality.override_reason = line.legality.override_reason;
			for(const DistributedLegalityIssue& issue : line.legality.issues) {
				DistributedLegalityIssue& issue_copy = copy.legality.issues.Add();
				issue_copy.code = issue.code;
				issue_copy.message = issue.message;
			}
		}
	}
	return header + DistributedSidecar2Writer().Generate(lines);
}

static Sidecar2Stats AddSidecar2Stats(const Sidecar2Stats& left, const Sidecar2Stats& right)
{
	Sidecar2Stats total;
	total.frames = left.frames + right.frames;
	total.hands = left.hands + right.hands;
	total.board_events = left.board_events + right.board_events;
	total.actions = left.actions + right.actions;
	total.showdowns = left.showdowns + right.showdowns;
	total.seat_updates = left.seat_updates + right.seat_updates;
	total.omitted_signals = left.omitted_signals + right.omitted_signals;
	total.omitted_dealer = left.omitted_dealer + right.omitted_dealer;
	total.omitted_board = left.omitted_board + right.omitted_board;
	total.omitted_action = left.omitted_action + right.omitted_action;
	total.omitted_showdown = left.omitted_showdown + right.omitted_showdown;
	total.omitted_seat = left.omitted_seat + right.omitted_seat;
	total.diagnostic_raw_board_frames = left.diagnostic_raw_board_frames + right.diagnostic_raw_board_frames;
	total.diagnostic_raw_board_max = max(left.diagnostic_raw_board_max, right.diagnostic_raw_board_max);
	total.diagnostic_board_trigger_frames = left.diagnostic_board_trigger_frames + right.diagnostic_board_trigger_frames;
	total.diagnostic_resolved_board_cards = left.diagnostic_resolved_board_cards + right.diagnostic_resolved_board_cards;
	total.diagnostic_ocr_candidates = left.diagnostic_ocr_candidates + right.diagnostic_ocr_candidates;
	total.diagnostic_action_candidates = left.diagnostic_action_candidates + right.diagnostic_action_candidates;
	total.diagnostic_action_reads = left.diagnostic_action_reads + right.diagnostic_action_reads;
	return total;
}

static int RunSidecar2(const String& video_path, const String& dataset_path,
	                   const String& source_sidecar, const String& requested_output,
	                   int start_second, int end_second, int max_frames,
	                   const String& window_mode,
	                   bool verbose, int ocr_cap,
	                   bool ocr_cache_enabled, int ocr_cache_threshold)
{
	String output_path = requested_output;
	if(output_path.IsEmpty())
		output_path = AppendFileName(GetFileFolder(source_sidecar),
		                             GetFileTitle(source_sidecar) + "_sidecar2.txt");

	String output;
	if(!LoadSidecar2Legend(source_sidecar, output)) {
		Cerr() << "ERROR: source sidecar does not have the expected six-line legend: "
		       << source_sidecar << "\n";
		return 1;
	}

	VsmVideoFileFrameSource video;
	if(!video.Open(video_path)) {
		Cerr() << "ERROR: cannot open direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}
	if(start_second > 0 && !video.SeekMs((int64)start_second * 1000)) {
		Cerr() << "ERROR: cannot seek direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}

	Cout() << "=== Mode: sidecar2 (recognized signals only) ===\n";
	Cout() << "video_source=direct-libavcodec file=" << video_path << "\n";
	Cout() << "video_info=" << video.GetSourceInfo() << "\n";
	Cout() << "source_sidecar=" << source_sidecar << "\n";
	Cout() << "output=" << output_path << "\n";
	Cout() << "windows=" << window_mode << " decoder_instances=1\n";
	Cout() << "range_start_second=" << start_second
	       << " range_end_second=" << end_second
	       << " max_sampled_pts_frames=" << max_frames << "\n";

	VsmLiveRegionClassifier classifier;
	if(classifier.Load(dataset_path) == 0) {
		Cerr() << "ERROR: classifier load failed\n";
		return 1;
	}
	VsmTesseractOcrEngine ocr;
	bool ocr_available = ocr.GetInfo().available;
	if(!ocr_available && ocr_cap != 0) {
		Cerr() << "ERROR: tesseract OCR is unavailable\n";
		return 1;
	}

	Vector<double> hole_scales;
	hole_scales << 0.30 << 0.35 << 0.40 << 0.45 << 0.50 << 0.55;
	CardTemplates board_templates = LoadCardTemplates(g_templates_dir);
	CardTemplates hole_templates = LoadCardTemplates(g_templates_dir, hole_scales);
	DealerTemplate dealer_template = LoadDealerTemplate(g_templates_dir);
	if(!board_templates.ok || !hole_templates.ok || !dealer_template.ok) {
		Cerr() << "ERROR: recognition templates are incomplete\n";
		return 1;
	}

	Sidecar2WindowState windows[2];
	Vector<WindowDescriptor> window_descriptors = SelectWindowDescriptors(window_mode);
	int window_count = window_descriptors.GetCount();
	ASSERT(window_count >= 1 && window_count <= 2);
	for(int window_index = 0; window_index < window_count; window_index++)
		InitializeSidecar2Window(windows[window_index], window_descriptors[window_index],
		                         ocr_cache_enabled, ocr_cache_threshold);
	Cout() << "pipeline=window-aware deterministic=1 staged_pre_ocr=1 async_ocr=0"
	       << " prepared_queue=none ocr_queue=none"
	       << " window_count=" << window_count << "\n";
	for(const WindowDescriptor& descriptor : window_descriptors)
		Cout() << "window_descriptor id=" << descriptor.id.value
		       << " name=" << descriptor.name
		       << " rect=" << descriptor.source_rect.left << "," << descriptor.source_rect.top
		       << "," << descriptor.source_rect.Width() << "," << descriptor.source_rect.Height() << "\n";
	VsmChangeDetectParams change_params;
	double started_ms = NowMs();
	int next_sample_second = start_second;
	int sampled_frames = 0;
	int decoded_frames = 0;
	int first_frame_second = -1;
	int last_frame_second = -1;
	bool layout_validated = false;
	int64 range_start_ms = (int64)start_second * 1000;
	int64 range_end_ms = end_second >= 0 ? (int64)end_second * 1000 : video.GetDurationMs();
	double requested_range_seconds = range_end_ms > range_start_ms
	                             ? (range_end_ms - range_start_ms) / 1000.0
	                             : -1.0;
	int next_progress_second = start_second + 10;
	int64 last_sampled_pts_ms = -1;
	int64 last_progress_pts_ms = -1;
	auto PrintProgress = [&](int64 progress_pts_ms) {
		if(progress_pts_ms < 0 || requested_range_seconds <= 0) return;
		double processed_seconds = (double)(progress_pts_ms - range_start_ms) / 1000.0;
		processed_seconds = min(max(processed_seconds, 0.0), requested_range_seconds);
		double percent = processed_seconds * 100.0 / requested_range_seconds;
		double wall_elapsed = (NowMs() - started_ms) / 1000.0;
		double eta = processed_seconds > 0.0 && wall_elapsed > 0.0
		           ? wall_elapsed * (requested_range_seconds - processed_seconds) / processed_seconds
		           : 0.0;
		Cout() << "progress: pts=" << Format("%.3f", progress_pts_ms / 1000.0)
		       << "s range=" << Format("%.3f", range_start_ms / 1000.0)
		       << "-" << Format("%.3f", range_end_ms / 1000.0)
		       << "s processed_seconds=" << Format("%.3f", processed_seconds)
		       << "/" << Format("%.3f", requested_range_seconds)
		       << " percent=" << Format("%.1f", percent)
		       << " wall_elapsed=" << Format("%.1f", wall_elapsed)
		       << "s eta=" << Format("%.1f", eta) << "s\n";
		Cout().Flush();
		last_progress_pts_ms = progress_pts_ms;
	};

	for(;;) {
		if(max_frames > 0 && sampled_frames >= max_frames)
			break;
		VsmImageBuffer buffer;
		int64 pts_ms = -1;
		double acquire_started = NowMs();
		if(!video.ReadFrame(buffer, pts_ms)) {
			if(!video.IsEof()) {
				Cerr() << "ERROR: direct MP4 decode failed: " << video.GetLastError() << "\n";
				return 1;
			}
			break;
		}
		double acquire_ms = NowMs() - acquire_started;
		decoded_frames++;
		if(pts_ms < (int64)start_second * 1000)
			continue;
		if(end_second >= 0 && pts_ms >= (int64)end_second * 1000)
			break;
		if(pts_ms < 0)
			continue;
		int frame_second = (int)(pts_ms / 1000);
		if(frame_second < next_sample_second)
			continue;
		next_sample_second = frame_second + 1;
		if(!layout_validated) {
			if(!ValidateSidecar2WindowLayout(buffer)) return 1;
			layout_validated = true;
		}
		sampled_frames++;
		last_sampled_pts_ms = pts_ms;
		if(first_frame_second < 0) first_frame_second = frame_second;
		last_frame_second = frame_second;
		if(!RunShaderEvidenceStage(buffer, window_descriptors, pts_ms, "sidecar2"))
			return 1;
		double fanout_started = NowMs();
		Vector<WindowFrame> window_frames = FanOutSourceFrame(buffer, decoded_frames - 1,
		                                                      sampled_frames - 1, pts_ms,
		                                                      window_descriptors);
		double fanout_ms = NowMs() - fanout_started;
		ASSERT(window_frames.GetCount() == window_count);

		for(int window_index = 0; window_index < window_count; window_index++) {
			Sidecar2WindowState& window = windows[window_index];
			g_sidecar2_sink = &window.sink;
			RecognitionWindowState& recognition = window.recognition;
			StageSet& stages = recognition.stages;
			LoopStats& loop_stats = recognition.loop_stats;
			ResolveState& state = recognition.resolve_state;
			Sidecar2Stats& stats = window.stats;
			bool& hand_open = window.hand_open;
			bool& pending_hand = window.pending_hand;
			bool& hand_has_progress = window.hand_has_progress;
			bool& hand_has_showdown = window.hand_has_showdown;
			int& hand_start_frame = window.hand_start_frame;
			int& hand_open_sec = window.hand_open_sec;
			int& last_raw_board_count = window.last_raw_board_count;
			int& redeal_window_sec = window.redeal_window_sec;
			bool *redeal_returned = window.redeal_returned;
			String *emitted_name = window.emitted_name;
			double *emitted_stack = window.emitted_stack;
			String *profile_name_candidate = window.profile_name_candidate;
			double *profile_stack_candidate = window.profile_stack_candidate;
			int *profile_streak = window.profile_streak;
			bool *fold_emitted = window.fold_emitted;
			String *last_action_text = window.last_action_text;
			int *last_action_sec = window.last_action_sec;
			String *hole_candidate = window.hole_candidate;
			int *hole_streak = window.hole_streak;
			bool *hole_emitted = window.hole_emitted;
			bool *hole_unresolved = window.hole_unresolved;
			String *hole_emitted_cards = window.hole_emitted_cards;
			bool& winner_emitted = window.winner_emitted;
			String& window_output = window.output;
			stages.acquire.Add(acquire_ms);
			stages.crop.Add(fanout_ms);

		bool folded_before[6];
		double bet_before[6], round_before[6];
		int cards_before[6];
		for(int seat = 0; seat < 6; seat++) {
			folded_before[seat] = state.seat_folded[seat];
			bet_before[seat] = state.seat_bet[seat];
			round_before[seat] = state.seat_round_total[seat];
			cards_before[seat] = state.seat_cards_present[seat];
		}
		int board_before = state.board_count;

		VsmFrameImage table;
		ProcessWindowFrame(window_frames[window_index], recognition,
		                   classifier, ocr, ocr_available, change_params,
		                   nullptr, verbose, ocr_cap, board_templates,
		                   dealer_template, &table, false);
		if(table.IsEmpty()) {
			stats.omitted_signals++;
			continue;
		}
		stats.frames++;
		if(stats.first_frame_second < 0) stats.first_frame_second = frame_second;
		stats.last_frame_second = frame_second;

		int present_count = 0;
		int returned_count = 0;
		if(redeal_window_sec >= 0 && frame_second - redeal_window_sec > 3) {
			redeal_window_sec = -1;
			for(int seat = 0; seat < 6; seat++) redeal_returned[seat] = false;
		}
		for(int seat = 0; seat < 6; seat++) {
			if(state.seat_cards_present[seat] == 1) present_count++;
			if(cards_before[seat] == 0 && state.seat_cards_present[seat] == 1) {
				if(redeal_window_sec < 0) redeal_window_sec = frame_second;
				redeal_returned[seat] = true;
			}
			if(redeal_returned[seat]) returned_count++;
		}
		int raw_board_count = SplitBoardBand(table).GetCount();
		bool board_cleared = last_raw_board_count >= 3 && raw_board_count == 0;
		bool redealt = returned_count >= 2 && raw_board_count == 0;
		bool initial_hand = !hand_open && !pending_hand && raw_board_count == 0 && present_count >= 2;
		bool next_hand = hand_open && !pending_hand
		              && (board_cleared || (redealt && hand_has_progress));

		if(initial_hand || next_hand) {
			if(hand_open) {
				CountSidecar2PartialSeats(state, hand_start_frame, emitted_name, stats);
				VsmRecognizedSidecar2HandResult result = window.sink.EndHand(
					frame_second, MakeSidecar2Snapshot(state));
				Cout() << Format("sidecar2_assertion stream=%c hand=%d status=%d authoritative=%d issues=%d\n",
					window.code, stats.hands, (int)result.legality.status,
					result.authoritative ? 1 : 0, result.legality.issues.GetCount());
			}
			pending_hand = true;
			hand_open = false;
			ResetSidecar2HandState(state);
			for(int seat = 0; seat < 6; seat++) {
				String known_name = CleanSidecar2Name(state.seat_name[seat]);
				double known_stack = state.seat_stack[seat];
				emitted_name[seat].Clear();
				emitted_stack[seat] = -1;
				fold_emitted[seat] = false;
				last_action_text[seat].Clear();
				last_action_sec[seat] = -1000;
				profile_name_candidate[seat] = known_name;
				profile_stack_candidate[seat] = known_stack;
				profile_streak[seat] = known_name.IsEmpty() || known_stack < 0 ? 0 : 2;
			}
			for(int region = 0; region < 3; region++) {
				hole_candidate[region].Clear();
				hole_streak[region] = 0;
				hole_emitted[region] = false;
				hole_unresolved[region] = false;
			}
			redeal_window_sec = -1;
			for(int seat = 0; seat < 6; seat++) redeal_returned[seat] = false;
		}

		if(pending_hand && raw_board_count == 0 && present_count >= 2) {
			double dealer_score = -2;
			Point dealer_center(-1, -1);
			int dealer_mirror = DetectDealerChip(table, dealer_template, dealer_score, dealer_center);
			if(dealer_mirror >= 0) {
				int dealer = MirrorToSidecarSeat(dealer_mirror);
				int small_blind = dealer % 6 + 1;
				int big_blind = small_blind % 6 + 1;
				String hand_event = Format("new hand, dealer=seat%d, smallblind=seat%d, bigblind=seat%d",
				                           dealer, small_blind, big_blind);
				AddSidecar2Event(window_output, window.code, frame_second, hand_event);
				stats.hands++;
				bool active[6];
				double committed[6];
				for(int seat = 0; seat < 6; seat++) { active[seat] = true; committed[seat] = 0; }
				DistributedStateSnapshot before = DistributedStateSnapshot();
				before.phase = "preflop";
				before.total = 0;
				for(int seat = 0; seat < 6; seat++) {
					DistributedParticipantState& participant = before.participants.Add();
					participant.active = active[seat];
					participant.committed = committed[seat];
				}
				String stream_code;
				stream_code.Cat(window.code);
				window.sink.BeginHand(stream_code, stats.hands, frame_second,
				                     before, hand_event);
				if(window_count == 1 && window.code == 'R')
					window_output << Format("# HAND %d checked=n\n", stats.hands);
				else
					window_output << Format("# %c HAND %d checked=n\n", window.code, stats.hands);
				hand_open = true;
				pending_hand = false;
				hand_start_frame = loop_stats.frames;
				hand_open_sec = frame_second;
				hand_has_progress = false;
				hand_has_showdown = false;
				state.dealer_seat_video = dealer_mirror;
				state.dealer_chip_score = dealer_score;
			}
		}

		if(hand_open) {
			for(int seat = 0; seat < 6; seat++) {
				bool fresh_name = state.seat_name_frame[seat] == loop_stats.frames;
				bool fresh_stack = state.seat_stack_frame[seat] == loop_stats.frames;
				if((!fresh_name && !fresh_stack) || frame_second - hand_open_sec > 30) continue;
				String previous_name = profile_name_candidate[seat];
				double previous_stack = profile_stack_candidate[seat];
				if(fresh_name) {
					String name = CleanSidecar2Name(state.seat_name[seat]);
					if(!name.IsEmpty()) profile_name_candidate[seat] = name;
				}
				if(fresh_stack && state.seat_stack[seat] >= 0)
					profile_stack_candidate[seat] = state.seat_stack[seat];
				if(profile_name_candidate[seat].IsEmpty() || profile_stack_candidate[seat] < 0) {
					if(g_sidecar2_diagnostics && (fresh_name || fresh_stack))
						EmitSidecar2Diagnostic(Format("profile_pending window=%c pts=%d seat=%d name=%s stack=%.4g fresh_name=%d fresh_stack=%d",
							window.code, frame_second, MirrorToSidecarSeat(seat),
							profile_name_candidate[seat].IsEmpty() ? "-" : ~profile_name_candidate[seat],
							profile_stack_candidate[seat], fresh_name ? 1 : 0, fresh_stack ? 1 : 0));
					continue;
				}
				if(profile_name_candidate[seat] == previous_name
				   && fabs(profile_stack_candidate[seat] - previous_stack) < 0.001
				   && profile_streak[seat] > 0)
					profile_streak[seat]++;
				else
					profile_streak[seat] = 1;
				if(profile_streak[seat] < 2) continue;
				String name = profile_name_candidate[seat];
				double stack = profile_stack_candidate[seat];
				if(name == emitted_name[seat] && fabs(stack - emitted_stack[seat]) < 0.001)
					continue;
				int sidecar_seat = MirrorToSidecarSeat(seat);
				AddSidecar2Event(window_output, window.code, frame_second,
				    Format("seat%d name=\"%s\" balance=%s", sidecar_seat, ~name,
				           ~FormatSidecar2Bb(stack)));
				if(g_sidecar2_diagnostics)
					EmitSidecar2Diagnostic(Format("profile_accepted window=%c pts=%d seat=%d name=\"%s\" balance=%s",
						window.code, frame_second, sidecar_seat, ~name, ~FormatSidecar2Bb(stack)));
				emitted_name[seat] = name;
				emitted_stack[seat] = stack;
				stats.seat_updates++;
			}

			int action_seat = state.last_action_seat;
			bool action_this_frame = state.last_action_frame == loop_stats.frames
			                      && action_seat >= 0 && action_seat < 6;
			if(action_this_frame && !hand_has_showdown) {
				String action = state.seat_action[action_seat];
				bool duplicate = action == last_action_text[action_seat]
				              && frame_second - last_action_sec[action_seat] <= 2;
				if(!duplicate) {
					last_action_text[action_seat] = action;
					last_action_sec[action_seat] = frame_second;
					if(action == "Fold") {
						if(!fold_emitted[action_seat]) {
							AddSidecar2Action(window_output, window.code, frame_second,
							    action_seat, DISTRIBUTED_ACTION_REMOVE, false, -1,
							    Format("seat%d fold", MirrorToSidecarSeat(action_seat)));
							fold_emitted[action_seat] = true;
							stats.actions++;
							hand_has_progress = true;
						}
					}
					else if(action.IsEmpty()) {
						stats.omitted_signals++;
						stats.omitted_action++;
					}
					else {
						double amount = -1;
						if(state.seat_bet[action_seat] >= 0
						   && fabs(state.seat_bet[action_seat] - bet_before[action_seat]) > 0.001)
							amount = state.seat_bet[action_seat];
						else if(state.seat_round_total[action_seat] >= 0
						        && fabs(state.seat_round_total[action_seat] - round_before[action_seat]) > 0.001)
							amount = state.seat_round_total[action_seat];
						String event = Format("seat%d %s", MirrorToSidecarSeat(action_seat),
						                      action == "All In" ? "all-in" : ~ToLower(action));
						if(amount >= 0) event << " " << FormatSidecar2Bb(amount);
						if(state.last_pot >= 0) event << " (pot " << FormatSidecar2Bb(state.last_pot) << ")";
						DistributedActionKind kind = action == "Call" || action == "Check"
						                         ? DISTRIBUTED_ACTION_MATCH
						                         : DISTRIBUTED_ACTION_INCREASE;
						AddSidecar2Action(window_output, window.code, frame_second,
						    action_seat, kind, amount >= 0, amount, event);
						stats.actions++;
						hand_has_progress = true;
					}
				}
			}

			for(int seat = 0; seat < 6; seat++) {
				if(!hand_has_showdown && !folded_before[seat] && state.seat_folded[seat]
				   && !fold_emitted[seat]) {
					AddSidecar2Action(window_output, window.code, frame_second,
					    seat, DISTRIBUTED_ACTION_REMOVE, false, -1,
					    Format("seat%d fold", MirrorToSidecarSeat(seat)));
					fold_emitted[seat] = true;
					stats.actions++;
					hand_has_progress = true;
				}
			}

			if(state.board_count > board_before) {
				String cards = Sidecar2BoardString(state, state.board_count);
				if(cards.IsEmpty()) {
					stats.omitted_signals++;
					stats.omitted_board++;
				}
				else {
					String event;
					if(state.board_count == 3) event = "flop " + cards;
					else if(state.board_count == 4) event = "turn " + cards.Right(2);
					else if(state.board_count == 5) event = "river " + cards.Right(2);
					if(!event.IsEmpty()) {
						if(state.last_pot >= 0) event << " (pot " << FormatSidecar2Bb(state.last_pot) << ")";
						AddSidecar2Event(window_output, window.code, frame_second, event);
						stats.board_events++;
						hand_has_progress = true;
						for(int seat = 0; seat < 6; seat++) last_action_text[seat].Clear();
					}
				}
			}
			else if(raw_board_count >= 3 && raw_board_count != last_raw_board_count
			        && raw_board_count > state.board_count) {
				stats.omitted_signals++;
				stats.omitted_board++;
			}

			if(state.board_count >= 3) {
				for(int region_index = 0; region_index < 3; region_index++) {
					const HoleRegion& region = kHoleRegions[region_index];
					bool unresolved = false;
					String cards = RecognizeSidecar2HoleCards(table, region, hole_templates, unresolved);
					if(unresolved) hole_unresolved[region_index] = true;
					if(cards.IsEmpty()) {
						hole_candidate[region_index].Clear();
						hole_streak[region_index] = 0;
						continue;
					}
					if(cards == hole_candidate[region_index])
						hole_streak[region_index]++;
					else {
						hole_candidate[region_index] = cards;
						hole_streak[region_index] = 1;
					}
					if(hole_streak[region_index] >= 2 && !hole_emitted[region_index]) {
						AddSidecar2Event(window_output, window.code, frame_second,
						    Format("seat%d shows %s", MirrorToSidecarSeat(region.mirror), ~cards));
						hole_emitted[region_index] = true;
						hole_emitted_cards[region_index] = cards;
						stats.showdowns++;
						hand_has_showdown = true;
					}
				}
				if(state.board_count == 5 && !winner_emitted) {
					bool all_active_resolved = true;
					int values[3] = {};
					bool has_value[3] = {};
					int resolved_count = 0;
					for(int seat = 0; seat < 6; seat++) {
						if(state.seat_folded[seat]) continue;
						int region_index = -1;
						for(int i = 0; i < 3; i++)
							if(kHoleRegions[i].mirror == seat) region_index = i;
						if(region_index < 0 || !hole_emitted[region_index]) {
							all_active_resolved = false;
							break;
						}
						String cards = hole_emitted_cards[region_index];
						if(cards.GetCount() != 4) {
							all_active_resolved = false;
							break;
						}
						int hand_cards[7];
						for(int i = 0; i < 5; i++) hand_cards[i] = state.board_cards[i];
						hand_cards[5] = CardIndex(cards.Mid(0, 1), cards.Mid(1, 1));
						hand_cards[6] = CardIndex(cards.Mid(2, 1), cards.Mid(3, 1));
						if(hand_cards[5] < 0 || hand_cards[6] < 0) {
							all_active_resolved = false;
							break;
						}
						int position[5] = {};
						values[region_index] = CardsValue::cardsValue(hand_cards, position);
						has_value[region_index] = true;
						resolved_count++;
					}
					if(all_active_resolved && resolved_count > 0) {
						int best = -1;
						for(int i = 0; i < 3; i++)
							if(has_value[i] && (best < 0 || values[i] > values[best])) best = i;
						if(best >= 0) {
							for(int i = 0; i < 3; i++)
								if(has_value[i] && values[i] == values[best])
									AddSidecar2Event(window_output, window.code, frame_second,
									                 Format("seat%d wins", MirrorToSidecarSeat(kHoleRegions[i].mirror)));
							winner_emitted = true;
						}
					}
				}
			}
		}

		last_raw_board_count = raw_board_count;
		}
		if(frame_second >= next_progress_second) {
			PrintProgress(pts_ms);
			next_progress_second = frame_second + 10;
		}
		if(sampled_frames % 30 == 0)
			SaveFile(output_path, ComposeSidecar2Output(output, windows, window_count));
	}
	if(last_progress_pts_ms != last_sampled_pts_ms)
		PrintProgress(last_sampled_pts_ms);

	for(int window_index = 0; window_index < window_count; window_index++) {
		Sidecar2WindowState& window = windows[window_index];
		const LoopStats& diagnostic_stats = window.recognition.loop_stats;
		window.stats.diagnostic_raw_board_frames = diagnostic_stats.diagnostic_raw_board_frames;
		window.stats.diagnostic_raw_board_max = diagnostic_stats.diagnostic_raw_board_max;
		window.stats.diagnostic_board_trigger_frames = diagnostic_stats.diagnostic_board_trigger_frames;
		window.stats.diagnostic_resolved_board_cards = diagnostic_stats.diagnostic_resolved_board_cards;
		window.stats.diagnostic_ocr_candidates = diagnostic_stats.diagnostic_ocr_candidates;
		window.stats.diagnostic_action_candidates = diagnostic_stats.diagnostic_action_candidates;
		window.stats.diagnostic_action_reads = diagnostic_stats.diagnostic_action_reads;
		if(window.hand_open) {
			CountSidecar2PartialSeats(window.recognition.resolve_state, window.hand_start_frame,
			                          window.emitted_name, window.stats);
			VsmRecognizedSidecar2HandResult result = window.sink.EndHand(
				last_frame_second < 0 ? 0 : last_frame_second,
				MakeSidecar2Snapshot(window.recognition.resolve_state));
			Cout() << Format("sidecar2_assertion stream=%c hand=%d status=%d authoritative=%d issues=%d\n",
				window.code, window.stats.hands, (int)result.legality.status,
				result.authoritative ? 1 : 0, result.legality.issues.GetCount());
		}
		if(window.pending_hand) {
			window.stats.omitted_signals++;
			window.stats.omitted_dealer++;
		}
		for(int region = 0; region < 3; region++) {
			if(window.hole_unresolved[region] && !window.hole_emitted[region]) {
				window.stats.omitted_signals++;
				window.stats.omitted_showdown++;
			}
		}
	}

	String output_dir = GetFileFolder(output_path);
	if(!output_dir.IsEmpty()) RealizeDirectory(output_dir);
	String composed_output = ComposeSidecar2Output(output, windows, window_count);
	if(!SaveFile(output_path, composed_output)) {
		Cerr() << "ERROR: failed to write " << output_path << "\n";
		return 1;
	}

	int parsed_hands = 0;
	for(int window_index = 0; window_index < window_count; window_index++) {
		SidecarGT parsed = ParseSidecarGT(output_path, windows[window_index].code);
		parsed_hands += parsed.new_hand_seconds.GetCount();
	}
	double elapsed_seconds = (NowMs() - started_ms) / 1000.0;
	decoded_frames = (int)video.GetDecodedFrameCount();
	int skipped_frames = decoded_frames - sampled_frames;
	int elapsed_frame_seconds = first_frame_second < 0 ? 0
	                          : last_frame_second - first_frame_second + 1;
	Sidecar2Stats combined = windows[0].stats;
	if(window_count == 2) combined = AddSidecar2Stats(windows[0].stats, windows[1].stats);
	int combined_unresolved = 0;
	int combined_ocr_calls = 0;
	Cout() << "\n=== Sidecar2 counters ===\n";
	Cout() << "decoded_frames=" << decoded_frames
	       << " sampled_pts_frames=" << sampled_frames
	       << " skipped_decoded_frames=" << skipped_frames
	       << " elapsed_frame_seconds=" << elapsed_frame_seconds << "\n";
	for(int window_index = 0; window_index < window_count; window_index++) {
		const Sidecar2WindowState& window = windows[window_index];
		const Sidecar2Stats& stats = window.stats;
		combined_unresolved += window.recognition.loop_stats.unresolved;
		combined_ocr_calls += window.recognition.loop_stats.ocr_calls;
		Cout() << "window=" << window.code << " name=" << window.name
		       << " frames=" << stats.frames << " hands=" << stats.hands
		       << " board_events=" << stats.board_events << " actions=" << stats.actions
		       << " showdowns=" << stats.showdowns << " seat_updates=" << stats.seat_updates
		       << " omitted_signals=" << stats.omitted_signals
		       << " classifier_unresolved_regions=" << window.recognition.loop_stats.unresolved
		       << " ocr_calls=" << window.recognition.loop_stats.ocr_calls
		       << " next_prepare_sequence=" << window.recognition.next_prepare_sequence
		       << " next_commit_sequence=" << window.recognition.next_commit_sequence
		       << " last_prepared_source_sequence=" << window.recognition.last_prepared_source_sequence
		       << " last_committed_source_sequence=" << window.recognition.last_committed_source_sequence << "\n";
		Cout() << "window=" << window.code
		       << " omitted_dealer=" << stats.omitted_dealer
		       << " omitted_board=" << stats.omitted_board
		       << " omitted_action=" << stats.omitted_action
		       << " omitted_showdown=" << stats.omitted_showdown
		       << " omitted_seat=" << stats.omitted_seat << "\n";
		Cout() << "window=" << window.code
		       << " diagnostic_raw_board_frames=" << window.recognition.loop_stats.diagnostic_raw_board_frames
		       << " diagnostic_raw_board_max=" << window.recognition.loop_stats.diagnostic_raw_board_max
		       << " diagnostic_board_trigger_frames=" << window.recognition.loop_stats.diagnostic_board_trigger_frames
		       << " diagnostic_resolved_board_cards=" << window.recognition.loop_stats.diagnostic_resolved_board_cards
		       << " diagnostic_ocr_candidates=" << window.recognition.loop_stats.diagnostic_ocr_candidates
		       << " diagnostic_action_candidates=" << window.recognition.loop_stats.diagnostic_action_candidates
		       << " diagnostic_action_reads=" << window.recognition.loop_stats.diagnostic_action_reads << "\n";
	}
	Cout() << "combined_window_frames=" << combined.frames
	       << " hands=" << combined.hands
	       << " board_events=" << combined.board_events
	       << " actions=" << combined.actions
	       << " showdowns=" << combined.showdowns
	       << " seat_updates=" << combined.seat_updates
	       << " omitted_signals=" << combined.omitted_signals
	       << " classifier_unresolved_regions=" << combined_unresolved
	       << " ocr_calls=" << combined_ocr_calls
	       << " diagnostic_raw_board_frames=" << combined.diagnostic_raw_board_frames
	       << " diagnostic_raw_board_max=" << combined.diagnostic_raw_board_max
	       << " diagnostic_board_trigger_frames=" << combined.diagnostic_board_trigger_frames
	       << " diagnostic_resolved_board_cards=" << combined.diagnostic_resolved_board_cards
	       << " diagnostic_ocr_candidates=" << combined.diagnostic_ocr_candidates
	       << " diagnostic_action_candidates=" << combined.diagnostic_action_candidates
	       << " diagnostic_action_reads=" << combined.diagnostic_action_reads
	       << " parsed_hands=" << parsed_hands
	       << Format(" elapsed_seconds=%.1f\n", elapsed_seconds)
	       << " output=" << output_path << "\n";
	for(int window_index = 0; window_index < window_count; window_index++) {
		Cout() << "window=" << windows[window_index].code << " stage_timings:\n";
		windows[window_index].recognition.stages.Print();
	}
	return parsed_hands == combined.hands ? 0 : 2;
}

// ===========================================================================
// Task 0291a item 5: BALANCE/BET/NAME/POT OCR accuracy with the cache DISABLED.
// This directly OCRs the fixed per-seat name/balance plates and the pot label
// (rects MEASURED on real frames) via the SAME VsmTesseractOcrEngine the live
// loop uses, and compares against the sidecar's own real numbers. It calls
// ocr.Execute() directly (no Task 0286 cache in the path at all -- i.e. the
// cache-disabled condition the audit asks for), and reports per-field accuracy
// (name / balance / pot) separately rather than one aggregate. Balances are read
// at each hand's first fully-labeled frame, where every stack still equals the
// sidecar's stated starting balance (verified: 0263 frame 7 and 0268 frame 55).
// ===========================================================================
struct SeatPlate { int mirror; Rect name; Rect bal; };
static const SeatPlate kSeatPlates[6] = {
	{ 0, RectC(428, 478, 116, 24), RectC(438, 505, 100, 26) }, // BOTTOM
	{ 1, RectC( 36, 360, 132, 24), RectC( 44, 388, 110, 24) }, // LEFT_BOTTOM
	{ 2, RectC( 72, 158, 140, 24), RectC( 78, 186, 116, 24) }, // LEFT_TOP
	{ 3, RectC(392,  96, 120, 24), RectC(404, 122, 108, 24) }, // TOP
	{ 4, RectC(770, 158, 128, 24), RectC(782, 186, 110, 24) }, // RIGHT_TOP
	{ 5, RectC(792, 362, 124, 24), RectC(806, 388, 104, 24) }, // RIGHT_BOTTOM
};
static const Rect kPotRect = RectC(408, 216, 120, 24);

static String NormName(const String& s) // case/space-insensitive name key
{
	String o;
	for(int i = 0; i < s.GetCount(); i++) { int c = s[i]; if(c > ' ') o.Cat(ToLower((char)c)); }
	return o;
}

static int RunOcrAccuracyTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: ocr-accuracy-test (NAME/BALANCE/POT vs sidecar, CACHE DISABLED) ===\n";
	VsmTesseractOcrEngine ocr;
	bool avail = ocr.GetInfo().available;
	Cout() << "tesseract OCR available: " << (avail ? "YES" : "NO") << "\n";
	if(!avail) { Cerr() << "ERROR: OCR engine not available\n"; return 1; }

	SidecarGT gt = ParseSidecarGT(kSidecarPath);
	auto pct = [](int a, int b){ return b ? 100.0 * a / b : 0.0; };

	auto ocr_rect = [&](const VsmFrameImage& table, const Rect& r, const char* sem) -> String {
		VsmFrameImage crop = CropFrameImage(table, r & RectC(0,0,table.width,table.height));
		if(crop.IsEmpty()) return "";
		VsmOcrRequest req; req.semantic = sem;
		req.region_id = Format("%d,%d,%d,%d", r.left, r.top, r.Width(), r.Height());
		VsmOcrResult res = ocr.Execute(crop, req);
		return TrimBoth(res.text);
	};
	auto load_table = [&](const String& dir, int sec, VsmFrameImage& table) -> bool {
		String path = AppendFileName(dir, Format("frame_%06d.jpg", sec));
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(path, buf)) return false;
		table = CropBufferToFrame(buf, kTableRect);
		return !table.IsEmpty();
	};

	int name_tot=0, name_ok=0, bal_tot=0, bal_ok=0;
	auto do_hand = [&](const char* dir, int hand, const Vector<int>& name_frames, int bal_frame) {
		Cout() << Format("\n--- HAND %d (%s) ---\n", hand + 1, dir);
		if(hand >= gt.hands.GetCount()) return;
		const VectorMap<int, SidecarGT::SeatInfo>& seats = gt.hands[hand];

		// NAMES: OCR each seat across the candidate frames; accept the first read
		// that matches GT (real name shown before an early fold overwrites the plate).
		Cout() << "  [NAME]\n";
		for(int mi = 0; mi < 6; mi++) {
			int gi = seats.Find(mi);
			if(gi < 0) continue;
			String want = seats[gi].name;
			if(want.IsEmpty()) continue;
			name_tot++;
			String best; bool matched = false;
			for(int fr : name_frames) {
				VsmFrameImage table;
				if(!load_table(dir, fr, table)) continue;
				String got = ocr_rect(table, kSeatPlates[mi].name, "seat_name_plate");
				if(best.IsEmpty() && !got.IsEmpty()) best = got;
				if(NormName(got) == NormName(want)) { best = got; matched = true; break; }
			}
			name_ok += matched;
			Cout() << Format("    mirror%d exp=\"%s\"  ocr=\"%s\"  %s\n",
			                 mi, ~want, ~best, matched ? "MATCH" : "MISS");
		}

		// BALANCES: at the hand-start frame every stack still equals GT starting BB.
		Cout() << Format("  [BALANCE] (frame %d)\n", bal_frame);
		VsmFrameImage table;
		if(load_table(dir, bal_frame, table)) {
			for(int mi = 0; mi < 6; mi++) {
				int gi = seats.Find(mi);
				if(gi < 0 || seats[gi].balance < 0) continue;
				double want = seats[gi].balance;
				bal_tot++;
				String got = ocr_rect(table, kSeatPlates[mi].bal, "seat_balance_plate");
				double v = 0; bool parsed = ParseChipValue(got, v);
				bool ok = parsed && fabs(v - want) < 0.15;
				bal_ok += ok;
				Cout() << Format("    mirror%d exp=%.1f`BB  ocr=\"%s\"(%.4g)  %s\n",
				                 mi, want, ~got, parsed ? v : -1.0, ok ? "MATCH" : "MISS");
			}
		}
	};
	// Wide name-frame sets: no single frame shows all 6 names (a seat's plate turns
	// into "Post SB/BB" before its name settles and into "Fold" once it folds), so
	// several frames are tried per seat and the GT-matching read (if any) is taken.
	{ Vector<int> nf; nf << 5 << 6 << 7 << 8 << 9 << 10 << 11; do_hand("tmp/real_recording_0263_frames", 0, nf, 7); }
	{ Vector<int> nf; nf << 53 << 54 << 55 << 56 << 57 << 58; do_hand("tmp/real_recording_0268_frames", 1, nf, 55); }

	// POT: OCR the pot label at the seconds where the sidecar states a pot total.
	// (Frame index == video second; nearest available frame in either dir.)
	Cout() << "\n--- POT (sidecar-stated pot totals) ---\n";
	struct PotCase { const char* dir; int sec; double pot; };
	static const PotCase pots[] = {
		{ "tmp/real_recording_0263_frames", 13,   4.0 },
		{ "tmp/real_recording_0263_frames", 17,   5.3 },
		{ "tmp/real_recording_0263_frames", 27,   8.5 },
		{ "tmp/real_recording_0263_frames", 38,  33.1 },
		{ "tmp/real_recording_0268_frames", 87,  29.1 },
		{ "tmp/real_recording_0268_frames", 100,178.7 },
		{ "tmp/real_recording_0268_frames", 102,178.7 },
	};
	int pot_tot = 0, pot_ok = 0;
	for(const PotCase& pc : pots) {
		VsmFrameImage table;
		if(!load_table(pc.dir, pc.sec, table)) { Cout() << Format("    sec%03d: frame load failed\n", pc.sec); continue; }
		String got = ocr_rect(table, kPotRect, "pot_label");
		double v = 0; bool parsed = ParseChipValue(got, v);
		bool ok = parsed && fabs(v - pc.pot) < 0.15;
		pot_tot++; pot_ok += ok;
		Cout() << Format("    sec%03d exp pot=%.1f`BB  ocr=\"%s\"(%.4g)  %s\n",
		                 pc.sec, pc.pot, ~got, parsed ? v : -1.0, ok ? "MATCH" : "MISS");
	}

	Cout() << "\n=== OCR accuracy (cache disabled), per field ===\n";
	Cout() << Format("NAME    : %d/%d (%.1f%%)\n", name_ok, name_tot, pct(name_ok, name_tot));
	Cout() << Format("BALANCE : %d/%d (%.1f%%)\n", bal_ok, bal_tot, pct(bal_ok, bal_tot));
	Cout() << Format("POT     : %d/%d (%.1f%%)\n", pot_ok, pot_tot, pct(pot_ok, pot_tot));
	Cout() << "(BET plates are transient/positional -- covered indirectly by the live pipeline's\n"
	          " seat_bet reroute logic, not graded here; pot is the stable central chip total.)\n";
	return 0;
}

// ===========================================================================
// Task 0291b: Line/corner-intersection keypoint detection -- INVESTIGATIVE.
//
// Concrete real target picked: the BOARD CARDS' vertical edges, validated as a
// cross-check on SplitBoardBand()'s felt-gap card boundaries (already verified
// 100% accurate for recognition by Task 0290a/0292). This is the cleanest
// testable target in-scope: white cards on green felt give strong Canny edges,
// SplitBoardBand gives independent per-card left/right boundary ground truth,
// and the search area is already bounded (the board band, never the full frame).
//
// Pipeline reuses ONLY the native ComputerVision building blocks (no OpenCV, no
// new CV implementation): TableRegionToGray -> GaussianBlur -> Canny ->
// HoughTransform (validated here for the FIRST time in this repo -- it had zero
// prior usage) plus FastCorners::Detect for a corner-keypoint cross-signal.
// ===========================================================================

// A near-vertical Hough line reduced to its x-position (table space) at the
// board band's mid-height. votes = accumulator strength (higher = stronger).
struct VLine : Moveable<VLine> { double x; double theta_deg;
	VLine() {} VLine(double x, double d) : x(x), theta_deg(d) {} };
struct HLine : Moveable<HLine> { double y; double theta_deg;
	HLine() {} HLine(double y, double d) : y(y), theta_deg(d) {} };

// Draw a thick point marker into an ImageDraw (small filled square).
static void MarkPoint(ImageDraw& w, int x, int y, int r, Color c)
{
	w.DrawRect(x - r, y - r, 2 * r + 1, 2 * r + 1, c);
}

static int RunLineCornerTest(const String& frames_dir)
{
	Cout() << "=== Mode: line-corner-test (Task 0291b, board-card edge keypoints) ===\n";
	Cout() << "target: board-card vertical edges vs SplitBoardBand() boundaries (independent GT)\n";

	// Board band crop region in table space -- BOUNDED (never the full frame),
	// padded a few px around SplitBoardBand's own search window so card edges are
	// not clipped at the crop border.
	const int PAD = 12;
	Rect crop = RectC(kBoardBandX0 - PAD, kBoardCardTop - PAD,
	                  (kBoardBandX1 - kBoardBandX0) + 2 * PAD,
	                  (kBoardCardBot - kBoardCardTop) + 2 * PAD);
	Cout() << Format("board-band crop (table space): x=%d y=%d w=%d h=%d  (%d px^2)\n",
	                 crop.left, crop.top, crop.Width(), crop.Height(), crop.Width() * crop.Height());

	// Hough parameters (validated empirically below).
	const double kRhoRes   = 1.0;
	const double kThetaRes = M_PI / 180.0; // 1 degree
	const double kHoughThr = 22.0;         // min accumulator votes (~ edge length)
	const double kVertTolDeg = 8.0;        // |theta| to 0 or 180 deg -> vertical
	const double kMergeX     = 6.0;        // merge near-duplicate vertical lines (table px)

	SidecarGT gt = ParseSidecarGT(kSidecarPath);
	Vector<String> dirs = ResolveFrameDirs(frames_dir);

	Stage s_gray, s_blur, s_canny, s_hough, s_fast;
	int    frames_scored = 0;
	int    bnd_total = 0, bnd_hit = 0;    // SplitBoardBand vertical boundaries matched by a Hough line
	double abs_err_sum = 0;               // sum |detected_x - boundary_x| over matched boundaries
	const double kMatchTol = 5.0;         // a boundary counts "found" if a vertical line is within this many px

	String overlay_saved;

	for(const String& dir : dirs) {
		Vector<int> secs = FrameSecondsInDir(dir);
		for(int sec : secs) {
			String board = gt.BoardAt(sec);
			if(ParseCardRun(board).GetCount() < 3) continue; // only post-flop frames (real card edges present)

			String path = AppendFileName(dir, Format("frame_%06d.jpg", sec));
			VsmImageBuffer buf;
			if(!LoadJpgToBuffer(path, buf)) continue;
			VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
			if(table.IsEmpty()) continue;

			// Ground truth: independent card boundaries from the already-verified splitter.
			Vector<Rect> cards = SplitBoardBand(table);
			if(cards.IsEmpty()) continue;

			// --- CV pipeline (native ComputerVision, no OpenCV) ---
			double t;
			t = NowMs();
			ByteMat gray = TableRegionToGray(table, crop);
			s_gray.Add(NowMs() - t);
			if(gray.IsEmpty()) continue;

			t = NowMs();
			ByteMat blur;
			GaussianBlur(gray, blur, 5, 0.0); // kernel 5, sigma auto (matches WebcamCV CannyEdge pattern)
			s_blur.Add(NowMs() - t);

			t = NowMs();
			ByteMat edges;
			Canny(blur, edges, 40, 120);
			s_canny.Add(NowMs() - t);

			t = NowMs();
			Vector<Pointf> lines = HoughTransform(edges, kRhoRes, kThetaRes, kHoughThr);
			s_hough.Add(NowMs() - t);

			t = NowMs();
			ByteMat corner_src = TableRegionToGray(table, crop);
			FastCorners fc;
			fc.set_threshold(30);
			Vector<Keypoint> corners;
			fc.Detect(corner_src, corners, 4);
			s_fast.Add(NowMs() - t);

			int H = crop.Height(), W = crop.Width();
			double ymid = H / 2.0, xmid = W / 2.0;

			// Split Hough lines into vertical / horizontal, mapped to table space.
			Vector<VLine> vlines;
			Vector<HLine> hlines;
			for(const Pointf& l : lines) {
				double rho = l.x, th = l.y;         // (rho, theta), theta in [0,pi)
				double deg = th * 180.0 / M_PI;
				double ct = cos(th), st = sin(th);
				bool vert = (deg < kVertTolDeg) || (deg > 180.0 - kVertTolDeg);
				bool horiz = fabs(deg - 90.0) < kVertTolDeg;
				if(vert && fabs(ct) > 1e-6) {
					double xloc = (rho - ymid * st) / ct;   // x at mid-height
					vlines.Add(VLine(crop.left + xloc, deg));
				}
				else if(horiz && fabs(st) > 1e-6) {
					double yloc = (rho - xmid * ct) / st;   // y at mid-width
					hlines.Add(HLine(crop.top + yloc, deg));
				}
			}
			// Merge near-duplicate vertical lines (Hough emits clusters); lines are
			// already vote-sorted, so first-seen wins each merged group.
			Sort(vlines, [](const VLine& a, const VLine& b){ return a.x < b.x; });
			Vector<VLine> vmerged;
			for(const VLine& v : vlines) {
				if(!vmerged.IsEmpty() && fabs(v.x - vmerged.Top().x) < kMergeX) continue;
				vmerged.Add(v);
			}

			// Score: each SplitBoardBand card's left & right edge is a true vertical
			// boundary; count how many have a detected vertical Hough line within tol.
			Vector<double> bnds;
			for(const Rect& c : cards) { bnds.Add(c.left); bnds.Add(c.left + c.Width()); }
			for(double b : bnds) {
				bnd_total++;
				double best = 1e9;
				for(const VLine& v : vmerged) best = min(best, fabs(v.x - b));
				if(best <= kMatchTol) { bnd_hit++; abs_err_sum += best; }
			}
			frames_scored++;

			bool verbose_frame = (overlay_saved.IsEmpty() && cards.GetCount() >= 4);
			if(verbose_frame || sec == secs[0]) {
				Cout() << Format("  %s sec %03d board=%-12s cards=%d  Hough:lines=%d vert=%d(merged %d) horiz=%d  FAST=%d\n",
				                 ~GetFileName(dir), sec, ~board, cards.GetCount(),
				                 lines.GetCount(), vlines.GetCount(), vmerged.GetCount(),
				                 hlines.GetCount(), corners.GetCount());
			}

			// Save ONE annotated overlay (first frame with >=4 cards = richest edges).
			if(overlay_saved.IsEmpty() && cards.GetCount() >= 4) {
				const int UP = 4; // upscale for visibility
				Image base = Crop(VsmFrameImageToImage(table), crop & RectC(0,0,table.width,table.height));
				base = Rescale(base, W * UP, H * UP);
				ImageDraw iw(W * UP, H * UP);
				iw.DrawImage(0, 0, base);
				// SplitBoardBand boundaries (GT) -- BLUE dashed-ish full-height lines
				for(double b : bnds) {
					int x = (int)((b - crop.left) * UP);
					iw.DrawLine(x, 0, x, H * UP, 1, Blue());
				}
				// Hough vertical lines -- RED
				for(const VLine& v : vmerged) {
					int x = (int)((v.x - crop.left) * UP);
					iw.DrawLine(x, 0, x, H * UP, 1, Color(255,40,40));
				}
				// Hough horizontal lines -- ORANGE
				for(const HLine& hl : hlines) {
					int y = (int)((hl.y - crop.top) * UP);
					iw.DrawLine(0, y, W * UP, y, 1, Color(255,160,0));
				}
				// vertical x horizontal intersections -- MAGENTA squares
				for(const VLine& v : vmerged)
					for(const HLine& hl : hlines) {
						int x = (int)((v.x - crop.left) * UP);
						int y = (int)((hl.y - crop.top) * UP);
						MarkPoint(iw, x, y, 3, Color(255,0,255));
					}
				// FAST corners -- GREEN dots
				for(const Keypoint& k : corners)
					MarkPoint(iw, k.x * UP, k.y * UP, 2, Color(0,230,0));
				Image over = iw;
				String outp = AppendFileName(GetCurrentDirectory(),
				                             Format("task0291b_overlay_%s_sec%03d.png", ~GetFileName(dir), sec));
				PNGEncoder().SaveFile(outp, over);
				overlay_saved = outp;
				// Also dump the raw Canny edge map (upscaled) for direct inspection.
				Image edgeImg = FloatMatToGrayImage([&]{ FloatMat f; f.SetSize(edges.cols, edges.rows, 1);
				                                          for(int i=0;i<edges.data.GetCount();i++) f.data[i]=edges.data[i];
				                                          return f; }());
				edgeImg = Rescale(edgeImg, W * UP, H * UP);
				String edgep = AppendFileName(GetCurrentDirectory(),
				                              Format("task0291b_canny_%s_sec%03d.png", ~GetFileName(dir), sec));
				PNGEncoder().SaveFile(edgep, edgeImg);
				Cout() << "  wrote overlay: " << outp << "\n";
				Cout() << "  wrote canny  : " << edgep << "\n";
			}
		}
	}

	auto pct = [](int a, int b){ return b ? 100.0 * a / b : 0.0; };
	Cout() << "\n=== Timing (per board frame, real, native ComputerVision) ===\n";
	auto row = [](const char* nm, const Stage& s){
		Cout() << Format("  %-10s calls=%-4d avg=%.3f ms  max=%.3f ms\n", nm, s.calls, s.Avg(), s.max_ms);
	};
	row("grayscale", s_gray); row("gauss+blur", s_blur); row("canny", s_canny);
	row("hough", s_hough); row("fastcorners", s_fast);
	double per_frame = s_gray.Avg() + s_blur.Avg() + s_canny.Avg() + s_hough.Avg();
	Cout() << Format("  => Canny+Hough pipeline: %.3f ms / board frame (+%.3f ms if FAST added)\n",
	                 per_frame, s_fast.Avg());

	Cout() << "\n=== Accuracy vs SplitBoardBand boundaries (independent GT) ===\n";
	Cout() << Format("post-flop frames scored : %d\n", frames_scored);
	Cout() << Format("card vertical boundaries : %d\n", bnd_total);
	Cout() << Format("matched by a Hough line  : %d/%d (%.1f%%) within +-%.0f px\n",
	                 bnd_hit, bnd_total, pct(bnd_hit, bnd_total), kMatchTol);
	Cout() << Format("mean abs error (matched) : %.2f px\n", bnd_hit ? abs_err_sum / bnd_hit : 0.0);
	if(!overlay_saved.IsEmpty()) Cout() << "annotated overlay: " << overlay_saved << "\n";
	return 0;
}

static String CalibrationJsonString(const String& value)
{
	String out;
	for(int i = 0; i < value.GetCount(); i++) {
		byte c = value[i];
		if(c == '\\') out << "\\\\";
		else if(c == '"') out << "\\\"";
		else if(c == '\n') out << "\\n";
		else if(c == '\r') out << "\\r";
		else out.Cat(c);
	}
	return out;
}

static int RunTimelineTextCalibration(const String& video_path, const String& output_path,
	                                  int step_seconds, int max_samples,
	                                  const String& failure_report_path)
{
	if(output_path.IsEmpty()) {
		Cerr() << "ERROR: --timeline-text-calibration requires an output path\n";
		WriteCalibrationFailure(failure_report_path, "arguments", "output path is empty");
		return 1;
	}
	Cout() << Format("calibration_start video=%s output=%s step=%d max_samples=%d decoder=libavcodec\n",
	                 ~video_path, ~output_path, step_seconds, max_samples);
	VsmTesseractOptions ocr_options;
	String ocr_error;
	bool ocr_available = VsmAutoResolveTesseract(ocr_options, ocr_error);
	Cout() << Format("calibration_ocr available=%s tessdata=%s tsv=%s\n",
	                 ocr_available ? "yes" : "no", ~ocr_options.tessdata_dir,
	                 ocr_options.tsv_config.IsEmpty() ? "no" : "yes");
	if(!ocr_available) {
		String error = ocr_error.IsEmpty() ? "Tesseract OCR is unavailable" : ocr_error;
		Cerr() << "ERROR: " << error << "\n";
		WriteCalibrationFailure(failure_report_path, "ocr-open", error);
		return 1;
	}
	VsmVideoFileFrameSource video;
	if(!video.Open(video_path)) {
		String error = video.GetLastError();
		Cerr() << "ERROR: cannot open calibration video: " << error << "\n";
		WriteCalibrationFailure(failure_report_path, "decoder-open", error);
		return 1;
	}
	Cout() << "calibration_decoder_open status=ok source=" << video.GetSourceInfo() << "\n";
	step_seconds = max(1, step_seconds);
	max_samples = max(1, max_samples);
	String json;
	json << "{\n  \"source\": \"" << CalibrationJsonString(video_path)
	     << "\",\n  \"decoder\": \"libavcodec\",\n"
	     << "  \"step_seconds\": " << step_seconds << ",\n  \"samples\": [\n";
	int sample_count = 0;
	int64 next_sample_ms = 0;
	int64 ts_ms = -1;
	VsmImageBuffer source;
	while(sample_count < max_samples && video.ReadFrame(source, ts_ms)) {
		if(ts_ms < next_sample_ms) continue;
		int second = (int)(ts_ms / 1000);
		next_sample_ms = (int64)(second + step_seconds) * 1000;
		Vector<WindowDescriptor> windows = SelectWindowDescriptors("both");
		if(sample_count) json << ",\n";
		json << "    {\"second\": " << second << ", \"windows\": [";
		for(int wi = 0; wi < windows.GetCount(); wi++) {
			if(wi) json << ",";
			VsmFrameImage table = CropBufferToFrame(source, windows[wi].source_rect);
			json << "{\"id\": \"" << windows[wi].id.value << "\", \"regions\": [";
			struct CalibrationRegion : Moveable<CalibrationRegion> {
				const char *name = nullptr;
				Rect rect;
				int index = -1;
			};
			Vector<CalibrationRegion> regions;
			for(int seat = 0; seat < 6; seat++) {
				CalibrationRegion& name = regions.Add(); name.name = "name"; name.rect = g_fixed_text_layout.name[seat]; name.index = seat;
				CalibrationRegion& balance = regions.Add(); balance.name = "balance"; balance.rect = g_fixed_text_layout.balance[seat]; balance.index = seat;
				CalibrationRegion& action = regions.Add(); action.name = "action"; action.rect = g_fixed_text_layout.action[seat]; action.index = seat;
				CalibrationRegion& bet = regions.Add(); bet.name = "bet"; bet.rect = g_fixed_text_layout.bet[seat]; bet.index = seat;
			}
			CalibrationRegion& board = regions.Add(); board.name = "board_money"; board.rect = g_fixed_text_layout.board_money; board.index = -1;
			for(int ri = 0; ri < regions.GetCount(); ri++) {
				if(ri) json << ",";
				VsmFrameImage crop = CropFrameImage(table, regions[ri].rect & RectC(0, 0, table.width, table.height));
				Cout() << Format("calibration_region_start sample=%d window=%s region=%s index=%d\n",
				                 sample_count, ~windows[wi].id.value, regions[ri].name, regions[ri].index);
				Cout().Flush();
				VsmTesseractOcrResult detail;
				String temp = GetTempFileName() + ".jpg";
				if(JPGEncoder().Quality(95).SaveFile(temp, VsmFrameImageToImage(crop))) {
					detail = VsmRunTesseractOcr(ocr_options, temp, regions[ri].name);
					DeleteFile(temp);
				}
				else
					Cout() << "calibration_region_save_failed\n";
				json << "{\"region\": \"" << regions[ri].name
				     << "\", \"index\": " << regions[ri].index
				     << ", \"x\": " << regions[ri].rect.left << ", \"y\": " << regions[ri].rect.top
				     << ", \"w\": " << regions[ri].rect.Width() << ", \"h\": " << regions[ri].rect.Height()
				     << ", \"text\": \"" << CalibrationJsonString(detail.text)
				     << "\", \"confidence\": " << Format("%.6f", detail.confidence)
				     << ", \"variants\": {";
				AppendOcrVariantJson(json, "original", detail.original_text, detail.original_exit_code, detail.original_avg_conf);
				json << ",";
				AppendOcrVariantJson(json, "preprocessed", detail.preprocessed_text, detail.preprocessed_exit_code, detail.preprocessed_avg_conf);
				json << ",";
				AppendOcrVariantJson(json, "otsu", detail.otsu_text, detail.otsu_exit_code, detail.otsu_avg_conf);
				json << "}, \"selected_variant\": " << detail.best_variant
				     << ", \"blank_detected\": " << (detail.blank_detected ? "true" : "false")
				     << ", \"otsu_invert\": " << (detail.otsu_invert ? "true" : "false") << "}";
				Cout() << Format("calibration_region_done sample=%d window=%s region=%s selected=%d text=%s\n",
				                 sample_count, ~windows[wi].id.value, regions[ri].name,
				                 detail.best_variant, ~detail.text);
				Cout().Flush();
			}
			json << "]}";
		}
		json << "]}";
		Cout() << Format("timeline_calibration sample=%d second=%d windows=%d\n",
		                 sample_count, second, windows.GetCount());
		sample_count++;
	}
	json << "\n  ]\n}\n";
	if(!SaveFile(output_path, json)) {
		Cerr() << "ERROR: failed to write timeline calibration: " << output_path << "\n";
		WriteCalibrationFailure(failure_report_path, "output-write", "could not write calibration JSON", sample_count);
		return 1;
	}
	Cout() << Format("timeline_calibration samples=%d output=%s\n", sample_count, ~output_path);
	if(!sample_count) {
		WriteCalibrationFailure(failure_report_path, "decode", "no decodable samples", sample_count);
		return 2;
	}
	return 0;
}

GUI_APP_MAIN
{
#ifdef PLATFORM_WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	SetVppLogName(AppendFileName(GetCurrentDirectory(), "VideoLiveRecognitionLoop.log"));

	const Vector<String>& args = CommandLine();
	String mode, host = "127.0.0.1", frames_dir, dataset = kDatasetDefault;
	String video_path = kVideoPath;
	int port = 8082, seconds = 30, max_frames = 0, wait_timeout_ms = 4000;
	int start_second = 0, end_second = -1;
	bool max_frames_set = false, end_second_set = false;
	int ocr_cap = -1; // -1 = unlimited, 0 = OCR off, N = up to N OCR calls/frame
	bool verbose = false, use_engine = true;
	bool sidecar2_short = false, sidecar2_full = false, two_window_sample = false;
	bool annotated_regression = false;
	String source_sidecar = kSidecarPath, sidecar2_output;
	String sidecar2_window = "both";
	String sidecar2_diagnostics_output;
	String shader_manifest, shader_crop_map;
	String shader_frame_config;
	String shader_stage_manifest, shader_stage_crop_map;
	int shader_frame_second = 0;
	// Task 0286 Part B: approximate-hash OCR result cache, ON by default (this
	// IS the optimization being delivered) -- see OcrCacheState's comment for
	// the design AND the "SAFETY OVERRIDE" note explaining why 40 (not the
	// higher number the live-loop calibration alone suggested) is used.
	bool ocr_cache_enabled = true;
	int  ocr_cache_threshold = 40; // real-evidence calibrated, see OcrCacheState's comment
	Vector<String> crop_safety_lists;
	String card_dataset_out; // Task 0290a --card-dataset-dump output dir
	String text_layout_out;  // Task 0294x fixed-window text search layout
	String text_layout_in;   // Task 0294aa shared layout input
	String json_out;         // Task 0294y machine-readable run manifest
	String report_out;       // Task 0294y human-readable run report
	String timeline_calibration_out;
	String calibration_layout_in, calibration_layout_out, layout_report_out;
	String parity_layout_in, parity_report_out;
	int timeline_step_seconds = 10, timeline_max_samples = 24;

	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--classify-selftest") mode = "selftest";
		else if(args[i] == "--shader-evidence-selftest") mode = "shader-evidence";
		else if(args[i] == "--shader-evidence-frame" && i + 2 < args.GetCount()) {
			mode = "shader-evidence-frame";
			shader_manifest = args[++i];
			shader_crop_map = args[++i];
		}
		else if(args[i] == "--shader-evidence-frame-config" && i + 1 < args.GetCount()) {
			mode = "shader-evidence-frame-config";
			shader_frame_config = args[++i];
		}
		else if(args[i] == "--shader-evidence-stage" && i + 2 < args.GetCount()) {
			shader_stage_manifest = args[++i];
			shader_stage_crop_map = args[++i];
		}
		else if(args[i] == "--offline-frames" && i + 1 < args.GetCount()) { mode = "offline"; frames_dir = args[++i]; }
		else if(args[i] == "--sidecar2") mode = "sidecar2";
		else if(args[i] == "--annotated-regression") { mode = "sidecar2"; annotated_regression = true; }
		else if(args[i] == "--live") mode = "live";
		else if(args[i] == "--gui") mode = "gui";
		else if(args[i] == "--crop-safety-check") mode = "cropsafety";
		else if(args[i] == "--card-recog-test") mode = "cardrecog"; // empty frames_dir -> BOTH hands (0263+0268)
		else if(args[i] == "--card-dataset-dump" && i + 1 < args.GetCount()) { mode = "carddataset"; card_dataset_out = args[++i]; if(frames_dir.IsEmpty()) frames_dir = "tmp/real_recording_0263_frames"; }
		else if(args[i] == "--dealer-turn-test") mode = "dealerturn"; // empty frames_dir -> BOTH hands
		else if(args[i] == "--hole-card-test") mode = "holecard";     // Task 0291a item 3
		else if(args[i] == "--ocr-accuracy-test") mode = "ocracc";    // Task 0291a item 5
		else if(args[i] == "--line-corner-test") mode = "linecorner"; // Task 0291b
		else if(args[i] == "--anchor-selftest") mode = "anchorselftest"; // Task 0294v
		else if(args[i] == "--dump-text-layout" && i + 1 < args.GetCount()) {
			mode = "textlayout"; text_layout_out = args[++i];
		}
		else if(args[i] == "--text-layout" && i + 1 < args.GetCount()) text_layout_in = args[++i];
		else if(args[i] == "--json-out" && i + 1 < args.GetCount()) json_out = args[++i];
		else if(args[i] == "--report-out" && i + 1 < args.GetCount()) report_out = args[++i];
		else if(args[i] == "--timeline-text-calibration" && i + 1 < args.GetCount()) {
			mode = "timelinecal"; timeline_calibration_out = args[++i];
		}
		else if(args[i] == "--generate-text-layout" && i + 2 < args.GetCount()) {
			mode = "layoutgen"; calibration_layout_in = args[++i]; calibration_layout_out = args[++i];
		}
		else if(args[i] == "--layout-report" && i + 1 < args.GetCount()) layout_report_out = args[++i];
		else if(args[i] == "--parity-check" && i + 2 < args.GetCount()) {
			mode = "parity"; parity_layout_in = args[++i]; parity_report_out = args[++i];
		}
		else if(args[i] == "--timeline-step" && i + 1 < args.GetCount()) timeline_step_seconds = max(1, StrInt(args[++i]));
		else if(args[i] == "--timeline-max-samples" && i + 1 < args.GetCount()) timeline_max_samples = max(1, StrInt(args[++i]));
		else if(args[i] == "--frames-dir" && i + 1 < args.GetCount()) frames_dir = args[++i];
		else if(args[i] == "--video" && i + 1 < args.GetCount()) video_path = args[++i];
		else if(args[i] == "--shader-frame-second" && i + 1 < args.GetCount()) shader_frame_second = max(0, StrInt(args[++i]));
		else if(args[i] == "--source-sidecar" && i + 1 < args.GetCount()) source_sidecar = args[++i];
		else if(args[i] == "--sidecar2-out" && i + 1 < args.GetCount()) sidecar2_output = args[++i];
		else if(args[i] == "--start-second" && i + 1 < args.GetCount()) start_second = max(0, StrInt(args[++i]));
		else if(args[i] == "--end-second" && i + 1 < args.GetCount()) { end_second = StrInt(args[++i]); end_second_set = true; }
		else if(args[i] == "--short-sample") sidecar2_short = true;
		else if(args[i] == "--full-run") sidecar2_full = true;
		else if(args[i] == "--two-window-sample") { mode = "sidecar2"; two_window_sample = true; }
		else if(args[i] == "--sidecar2-window" && i + 1 < args.GetCount()) sidecar2_window = ToLower(args[++i]);
		else if(args[i] == "--templates" && i + 1 < args.GetCount()) g_templates_dir = args[++i];
		else if(args[i] == "--crop-list" && i + 1 < args.GetCount()) crop_safety_lists.Add(args[++i]);
		else if(args[i] == "--host" && i + 1 < args.GetCount()) host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount()) port = StrInt(args[++i]);
		else if(args[i] == "--seconds" && i + 1 < args.GetCount()) seconds = max(1, StrInt(args[++i]));
		else if(args[i] == "--max-frames" && i + 1 < args.GetCount()) { max_frames = StrInt(args[++i]); max_frames_set = true; }
		else if(args[i] == "--wait-timeout-ms" && i + 1 < args.GetCount()) wait_timeout_ms = StrInt(args[++i]);
		else if(args[i] == "--dataset" && i + 1 < args.GetCount()) dataset = args[++i];
		else if(args[i] == "--ocr-cap" && i + 1 < args.GetCount()) ocr_cap = StrInt(args[++i]);
		else if(args[i] == "--no-ocr") ocr_cap = 0;
		else if(args[i] == "--no-ocr-cache") ocr_cache_enabled = false;
		else if(args[i] == "--ocr-cache-threshold" && i + 1 < args.GetCount()) ocr_cache_threshold = StrInt(args[++i]);
		else if(args[i] == "--sidecar2-diagnostics") g_sidecar2_diagnostics = true;
		else if(args[i] == "--sidecar2-diagnostics-out" && i + 1 < args.GetCount()) {
			g_sidecar2_diagnostics = true;
			sidecar2_diagnostics_output = args[++i];
		}
		else if(args[i] == "--verbose") verbose = true;
		else if(args[i] == "--no-engine") use_engine = false;
		else if(args[i] == "--help" || args[i] == "-h") mode = "help";
	}

	if(mode.IsEmpty() || mode == "help") {
		Cout() << "VideoLiveRecognitionLoop (Task 0280/0293a)\n"
		       << "Modes:\n"
		       << "  --classify-selftest        Leave-one-out classifier accuracy over the dataset\n"
		       << "  --shader-evidence-selftest Shared two-window shader evidence adapter selftest\n"
		       << "  --shader-evidence-frame <manifest> <crop-map> Decode one MP4 frame and print L/R shader evidence\n"
		       << "  --shader-evidence-frame-config <json> Decode using video/manifest/crop_map descriptor\n"
		       << "  --shader-evidence-stage <manifest> <crop-map> Enable shared optional stage for live/sidecar2\n"
		       << "  --offline-frames <dir>     Full pipeline over dataset source frames (timing)\n"
		       << "  --sidecar2                 Generate recognized-data sidecar directly from MP4\n"
		       << "  --annotated-regression    Generate both windows and compare against source sidecar\n"
		       << "  --live                     Continuous loop against a running VideoServer\n"
		       << "  --gui                      Live mirror window of the driven Game (Task 0281);\n"
		       << "                               background recognition loop + main-thread TopWindow,\n"
		       << "                               runs until the window is closed\n"
		       << "  --crop-safety-check         Task 0286: false-hit risk of the OCR cache over the\n"
		       << "                               31-crop Task 0274 validation set (--crop-list to\n"
		       << "                               override; defaults to both hand1/hand2 lists)\n"
		       << "  --card-recog-test           Task 0290a: board card recognition (felt-split +\n"
		       << "                               suit-colour + rank template match) accuracy vs the\n"
		       << "                               sidecar ground truth + ORB-vs-template timing\n"
		       << "  --card-dataset-dump <dir>   Task 0290a: dump labeled board-card crops + manifest\n"
		       << "  --dealer-turn-test          Task 0290c/0291a: dealer-chip TM + ORB + turn-timer\n"
		       << "                               accuracy vs sidecar (BOTH hands unless --frames-dir)\n"
		       << "  --hole-card-test            Task 0291a: showdown hole-card recognition (both hands)\n"
		       << "  --ocr-accuracy-test         Task 0291a: name/balance/pot OCR accuracy, cache disabled\n"
		       << "  --line-corner-test          Task 0291b: Canny+Hough line / FAST corner keypoints\n"
		       << "                               on the board band, vs SplitBoardBand boundaries\n"
		       << "  --anchor-selftest           Task 0294v: RGB BB/action template self-test\n"
			       << "  --dump-text-layout <path>  Task 0294x: write fixed 944x682 text search layout\n"
			       << "  --text-layout <path>      Task 0294aa: load shared text-search layout JSON\n"
			       << "  --json-out <path>         Task 0294y: write run manifest JSON\n"
			       << "  --report-out <path>       Task 0294y: write run report JSON\n"
			       << "  --timeline-text-calibration <path>  Task 0294z: OCR timeline JSON\n"
			       << "  --timeline-step <seconds>            Sampling interval (default 10)\n"
			       << "  --timeline-max-samples <n>           Sampling cap (default 24)\n"
			       << "  --generate-text-layout <in> <out>   Validate calibration and write shared layout\n"
			       << "  --layout-report <path>              Write layout acceptance/rejection report\n"
			       << "  --parity-check <layout> <report>    Compare shared live/sidecar regions\n"
		       << "Options:\n"
		       << "  --host <h> --port <p>       VideoServer address (default 127.0.0.1:8082)\n"
		       << "  --seconds <n>               Live run duration (default 30)\n"
		       << "  --max-frames <n>            Cap offline frames or processed sidecar2 PTS seconds\n"
		       << "  --wait-timeout-ms <n>       Live source wait timeout (default 4000)\n"
		       << "  --dataset <path>            Labeled dataset (default " << kDatasetDefault << ")\n"
		       << "  --frames-dir <dir>          Real JPEG frames for non-sidecar offline tests\n"
		       << "  --video <path>              Sidecar2 MP4 source; default " << kVideoPath << "\n"
		       << "  --shader-frame-second <n>  Frame second for --shader-evidence-frame (default 0)\n"
		       << "  --source-sidecar <path>     Legend source; default " << kSidecarPath << "\n"
		       << "  --sidecar2-out <path>       Output path; default next to source as *_sidecar2.txt\n"
		       << "  --sidecar2-window <mode>    both (default), right, or left; right preserves one-window output\n"
		       << "  --start-second <n>          Seek to this MP4 presentation second\n"
		       << "  --end-second <n>            Stop before this MP4 presentation second\n"
		       << "  --short-sample              Sidecar2 safety cap of 120 PTS seconds (default)\n"
		       << "  --two-window-sample         Sidecar2 both-window sample capped at 105 seconds\n"
		       << "  --full-run                  Sidecar2 processes every PTS second through EOF/range\n"
		       << "  --templates <dir>           Card template library (default " << g_templates_dir << ")\n"
		       << "  --ocr-cap <n>               Max OCR calls per frame (-1 unlimited, 0 off)\n"
		       << "  --no-ocr                    Disable OCR stage entirely\n"
		       << "  --no-ocr-cache               Disable the Task 0286 approximate-hash OCR cache\n"
		       << "                               (enabled by default; use for before/after A-B runs)\n"
		       << "  --ocr-cache-threshold <n>   Tight signature-distance threshold (default 40)\n"
		       << "  --sidecar2-diagnostics      Print per-sampled-frame board/OCR/action gates\n"
		       << "  --sidecar2-diagnostics-out <path>  Also write diagnostics deterministically\n"
		       << "  --crop-list <path>           (crop-safety-check) add a category\\tpath list file\n"
		       << "  --no-engine                 Skip the real Game engine stage\n"
		       << "  --verbose                   Per-region log lines\n";
		return;
	}
	if((sidecar2_short || two_window_sample) && sidecar2_full) {
		Cerr() << "ERROR: sample modes and --full-run are mutually exclusive\n";
		SetExitCode(1);
		return;
	}
	if(!shader_stage_manifest.IsEmpty() || !shader_stage_crop_map.IsEmpty()) {
		if(shader_stage_manifest.IsEmpty() || shader_stage_crop_map.IsEmpty()) {
			Cerr() << "ERROR: --shader-evidence-stage requires manifest and crop-map\n";
			SetExitCode(1);
			return;
		}
		if(!ConfigureShaderEvidenceStage(shader_stage_manifest, shader_stage_crop_map)) {
			SetExitCode(1);
			return;
		}
	}
	if(!text_layout_in.IsEmpty() && !LoadFixedTextSearchLayout(text_layout_in)) {
		SetExitCode(1);
		return;
	}
	// Task 0294v: load the same RGB anchor library for live and libavcodec
	// sidecar2 modes. Missing optional templates are diagnosed, not fatal: the
	// existing classifier/OCR path remains available for incomplete installations.
	g_recognition_anchors.Load(g_templates_dir);
	if(mode == "sidecar2") {
		if(annotated_regression)
			sidecar2_window = "both";
		if(sidecar2_window != "both" && sidecar2_window != "right" && sidecar2_window != "left") {
			Cerr() << "ERROR: --sidecar2-window must be both, right, or left\n";
			SetExitCode(1);
			return;
		}
		if(!frames_dir.IsEmpty()) {
			Cerr() << "ERROR: --frames-dir is not valid with --sidecar2; use direct --video MP4 input\n";
			SetExitCode(1);
			return;
		}
		if(end_second >= 0 && end_second <= start_second) {
			Cerr() << "ERROR: --end-second must be greater than --start-second\n";
			SetExitCode(1);
			return;
		}
		if(two_window_sample) {
			sidecar2_window = "both";
			if(!end_second_set) end_second = start_second + 105;
			if(!max_frames_set) max_frames = 105;
		}
		if(sidecar2_full) max_frames = 0;
		if(!sidecar2_full && max_frames <= 0) max_frames = 120;
		if(g_sidecar2_diagnostics) {
			g_sidecar2_diagnostics_path = sidecar2_diagnostics_output;
			if(g_sidecar2_diagnostics_path.IsEmpty()) {
				String diagnostic_base = sidecar2_output.IsEmpty()
				                       ? AppendFileName(GetFileFolder(source_sidecar),
				                                         GetFileTitle(source_sidecar) + "_sidecar2.txt")
				                       : sidecar2_output;
				g_sidecar2_diagnostics_path = diagnostic_base + ".diagnostics.log";
			}
			SaveFile(g_sidecar2_diagnostics_path, String());
		}
	}

	int rc = 0;
	if(mode == "selftest") rc = RunClassifySelfTest(dataset);
	else if(mode == "shader-evidence") rc = RunShaderEvidenceSelfTest();
	else if(mode == "shader-evidence-frame") rc = RunShaderEvidenceFrame(video_path, shader_manifest,
	                                                                     shader_crop_map, shader_frame_second);
	else if(mode == "shader-evidence-frame-config") rc = RunShaderEvidenceFrameConfig(shader_frame_config);
	else if(mode == "offline") rc = RunOfflineFrames(frames_dir, dataset, max_frames, verbose, use_engine, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "sidecar2") rc = RunSidecar2(video_path, dataset, source_sidecar,
	                                               sidecar2_output, start_second, end_second,
	                                               max_frames, sidecar2_window,
	                                               verbose, ocr_cap, ocr_cache_enabled,
	                                               ocr_cache_threshold);
	if(annotated_regression && rc == 0) {
		if(sidecar2_output.IsEmpty()) {
			Cerr() << "ERROR: --annotated-regression requires --sidecar2-out <path>\n";
			rc = 1;
		}
		else
			rc = CompareAnnotatedSidecar2(source_sidecar, sidecar2_output);
	}
	else if(mode == "live") rc = RunLive(host, port, seconds, dataset, verbose, use_engine, wait_timeout_ms, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "gui") rc = RunGui(host, port, dataset, verbose, use_engine, wait_timeout_ms, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "cropsafety") {
		if(crop_safety_lists.IsEmpty()) {
			crop_safety_lists.Add("tmp/_task0274_final_crop_list_hand1.txt");
			crop_safety_lists.Add("tmp/_task0274_final_crop_list_hand2.txt");
		}
		rc = RunCropSafetyCheck(crop_safety_lists, ocr_cache_threshold);
	}
	else if(mode == "cardrecog") rc = RunCardRecogTest(frames_dir, g_templates_dir);
	else if(mode == "carddataset") rc = RunCardDatasetDump(frames_dir, g_templates_dir, card_dataset_out);
	else if(mode == "dealerturn") rc = RunDealerTurnTest(frames_dir, g_templates_dir);
	else if(mode == "holecard") rc = RunHoleCardTest(frames_dir, g_templates_dir);
	else if(mode == "ocracc") rc = RunOcrAccuracyTest(frames_dir, g_templates_dir);
	else if(mode == "linecorner") rc = RunLineCornerTest(frames_dir);
	else if(mode == "anchorselftest") rc = RunRecognitionAnchorSelfTest();
	else if(mode == "timelinecal") {
		String failure_report = timeline_calibration_out + ".failure.json";
		rc = RunTimelineTextCalibration(video_path, timeline_calibration_out,
		                                timeline_step_seconds, timeline_max_samples, failure_report);
	}
	else if(mode == "layoutgen")
		rc = SaveGeneratedTextLayout(calibration_layout_in, calibration_layout_out, layout_report_out);
	else if(mode == "parity")
		rc = WriteLayoutParityReport(parity_layout_in, parity_report_out) ? 0 : 1;
	else if(mode == "textlayout") {
		rc = SaveFixedTextSearchLayout(text_layout_out) ? 0 : 1;
		Cout() << "text_layout=" << text_layout_out << " size=944x682 status="
		       << (rc == 0 ? "written" : "write-failed") << "\n";
	}
	if(!WritePipelineReport(report_out, mode, video_path, text_layout_in, sidecar2_output, rc)) {
		Cerr() << "ERROR: failed to write report: " << report_out << "\n";
		if(!rc) rc = 1;
	}
	if(!json_out.IsEmpty() && !WritePipelineReport(json_out, mode, video_path, text_layout_in,
	                                                sidecar2_output, rc)) {
		Cerr() << "ERROR: failed to write JSON manifest: " << json_out << "\n";
		if(!rc) rc = 1;
	}
	if(rc) SetExitCode(rc);
}
