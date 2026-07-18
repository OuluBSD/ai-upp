#include <CtrlCore/CtrlCore.h> // GUI_APP_MAIN (engine pulls no Ctrl header itself)
#include <VisualStateModel/VisualStateModel.h>
#include <plugin/jpg/jpg.h>

#include <TexasHoldem/TexasHoldemLocalGame.h>
#include <Poker/LocalEngineFactory.h>
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
//   --live                Continuous loop against a running VideoServer (the
//                         real Task 0278/0279 live source).
// ===========================================================================

// Fixed PokerStars table window in the 1920x1080 recording. Confirmed static
// across all 56 tracking frames of the M12 recording (tracking.json: the sole
// distinct table-0 window rect is (8,1,944,682)). Using a fixed crop matches
// M12's single-static-table scope; a moving/multi-table feed would need
// VideoWindowTracker per frame (explicitly out of scope for this task).
static const Rect kTableRect = RectC(8, 1, 944, 682);

static const char* kDatasetDefault = "tmp/real_recording_combined_classified_dataset.json";

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
	double Avg() const { return calls ? total_ms / calls : 0; }
};

struct StageSet {
	Stage acquire, crop, change, classify, ocr, resolve, engine;
	void Print() {
		auto row = [](const Stage& s) {
			Cout() << Format("  %-14s calls=%-6d avg=", ~s.name, s.calls)
			       << Format("%.3f", s.Avg()) << "ms  max="
			       << Format("%.3f", s.max_ms) << "ms  total="
			       << Format("%.1f", s.total_ms) << "ms\n";
		};
		row(acquire); row(crop); row(change); row(classify); row(ocr); row(resolve); row(engine);
	}
};

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

static bool IsOcrCategory(const String& cat)
{
	return cat == "pot_label" || cat == "seat_balance_plate" || cat == "seat_bet_label";
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
// PokerStars-style: rank in {2..9,T,J,Q,K,A}, suit in {c,d,h,s}. Engine card
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
// Shared per-frame pipeline state (stages 2..5) used by both offline + live.
// ===========================================================================
struct ResolveState {
	ResolveState() { for(int i = 0; i < 5; i++) board_cards.Add(-1); }
	// board tracking (structural tier)
	int  board_count = 0;
	Vector<int> board_cards;
	int  street_events = 0; // flop/turn/river dealt
	// value tracking (OCR tier): slot key "xxYY" -> last numeric value
	VectorMap<String, double> slot_value;
	int  value_changes = 0;
	int  pot_reads = 0;
	double last_pot = -1;
};

struct LoopStats {
	int frames = 0;
	int total_regions = 0;
	VectorMap<String, int> category_counts;
	int ocr_calls = 0, ocr_nonempty = 0;
	int classified = 0, unresolved = 0;
};

// Process one already-cropped table frame against prev; returns whether prev
// was updated (always true after first frame).
static void ProcessTableFrame(const VsmFrameImage& table, VsmFrameImage& prev, bool has_prev,
                              VsmLiveRegionClassifier& clf, VsmTesseractOcrEngine& ocr,
                              bool ocr_available, const VsmChangeDetectParams& cdp,
                              StageSet& st, LoopStats& stats, ResolveState& rs,
                              const std::shared_ptr<Game>& game, bool verbose, int ocr_cap)
{
	if(!has_prev) return;
	int ocr_used = 0; // OCR calls spent on THIS frame (budget throttle)

	// --- stage: change detection ---
	double t = NowMs();
	Vector<VsmChangedRect> rects = VsmDetectChanges(prev, table, cdp);
	st.change.Add(NowMs() - t);

	// Convert the table frame to an Image ONCE for classification crops.
	t = NowMs();
	Image table_img = VsmFrameImageToImage(table);
	double conv_ms = NowMs() - t;
	st.crop.Add(conv_ms);

	stats.frames++;
	stats.total_regions += rects.GetCount();

	int board_regions_this_frame = 0;
	Vector<int> new_board_cards;

	for(const VsmChangedRect& cr : rects) {
		Rect r = cr.GetRect() & RectC(0, 0, table.width, table.height);
		if(r.Width() <= 0 || r.Height() <= 0) continue;

		// --- stage: classify ---
		t = NowMs();
		Image crop = Crop(table_img, r);
		VsmLiveClassifyResult cl = clf.Classify(crop, r.left, r.top);
		st.classify.Add(NowMs() - t);

		stats.classified++;
		String cat = cl.category.IsEmpty() ? String("<unresolved>") : cl.category;
		if(cl.category.IsEmpty()) stats.unresolved++;
		stats.category_counts.GetAdd(cat, 0)++;

		// --- stage: OCR (only OCR-relevant categories) ---
		String ocr_text; double ocr_conf = 0;
		if(ocr_available && IsOcrCategory(cl.category) && (ocr_cap < 0 || ocr_used < ocr_cap)) {
			ocr_used++;
			t = NowMs();
			VsmFrameImage ocr_crop = CropFrameImage(table, r);
			VsmOcrRequest req;
			req.semantic = cl.category;     // Task 0277's semantic field, first real caller
			req.region_id = Format("%d,%d,%d,%d", r.left, r.top, r.Width(), r.Height());
			VsmOcrResult res = ocr.Execute(ocr_crop, req);
			st.ocr.Add(NowMs() - t);
			ocr_text = res.text; ocr_conf = res.confidence;
			stats.ocr_calls++;
			if(!TrimBoth(ocr_text).IsEmpty()) stats.ocr_nonempty++;
		}

		// --- stage: resolve ---
		t = NowMs();
		if(cl.category == "board_card") {
			board_regions_this_frame++;
			int ci = CardIndex(cl.rank, cl.suit);
			if(ci >= 0) new_board_cards.Add(ci);
		}
		else if(IsOcrCategory(cl.category)) {
			double val;
			if(ParseChipValue(ocr_text, val)) {
				if(cl.category == "pot_label") {
					rs.pot_reads++;
					if(rs.last_pot >= 0 && fabs(val - rs.last_pot) > 0.001) rs.value_changes++;
					rs.last_pot = val;
				} else {
					String slot = Format("%s@%d,%d", ~cl.category, r.left, r.top);
					int q = rs.slot_value.Find(slot);
					if(q >= 0 && fabs(rs.slot_value[q] - val) > 0.001) rs.value_changes++;
					rs.slot_value.GetAdd(slot) = val;
				}
			}
		}
		st.resolve.Add(NowMs() - t);

		if(verbose && !cl.category.IsEmpty())
			Cout() << Format("    region (%d,%d %d w x %d h) -> %-22s conf=%.2f tier=%s dist=%d%s\n",
			                 r.left, r.top, r.Width(), r.Height(), ~cat, cl.confidence,
			                 ~cl.tier, cl.distance,
			                 ocr_text.IsEmpty() ? "" : ~Format("  ocr=\"%s\"(%.2f)", ~TrimBoth(ocr_text), ocr_conf));
	}

	// Structural street events + engine application from board region count.
	t = NowMs();
	if(board_regions_this_frame > 0) {
		int new_count = rs.board_count;
		// The board grows to 3 (flop), 4 (turn), 5 (river). We infer growth
		// from how many distinct board-card regions we can currently resolve.
		int resolved_cards = new_board_cards.GetCount();
		if(resolved_cards > new_count) new_count = min(5, resolved_cards);
		if(new_count > rs.board_count) {
			// merge resolved cards into board_cards (positionally best-effort)
			for(int i = 0; i < new_board_cards.GetCount() && i < 5; i++)
				rs.board_cards[i] = new_board_cards[i];
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
			if(verbose)
				Cout() << Format("    [structural] board %d -> %d dealt; engine board forced\n", from, to);
		}
	}
	st.engine.Add(NowMs() - t);

	prev.Set(table.width, table.height, ~table.data);
}

// ===========================================================================
// Mode 2: offline pipeline over dataset source frames (timing, deterministic).
// ===========================================================================
static int RunOfflineFrames(const String& frames_dir, const String& dataset_path,
                            int max_frames, bool verbose, bool use_engine, int ocr_cap)
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
	}

	StageSet st;
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
	VsmChangeDetectParams cdp;

	VsmFrameImage prev; bool has_prev = false;
	for(int fi = 0; fi < files.GetCount(); fi++) {
		double t = NowMs();
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(files[fi], buf)) continue;
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		st.acquire.Add(NowMs() - t);
		if(table.IsEmpty()) continue;

		if(verbose) Cout() << Format("frame %d (%s):\n", fi, ~GetFileName(files[fi]));
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, game, verbose, ocr_cap);
		if(!has_prev) { prev.Set(table.width, table.height, ~table.data); has_prev = true; }
	}

	Cout() << "\n=== Per-stage timing (offline) ===\n";
	st.Print();
	Cout() << "\n=== Recognition summary ===\n";
	Cout() << "frames_processed=" << stats.frames << " total_changed_regions=" << stats.total_regions
	       << " (avg " << Format("%.1f", stats.frames ? (double)stats.total_regions / stats.frames : 0) << "/frame)\n";
	Cout() << "classified=" << stats.classified << " unresolved=" << stats.unresolved
	       << Format(" (%.1f%% resolved)\n", stats.classified ? 100.0 * (stats.classified - stats.unresolved) / stats.classified : 0);
	Cout() << "ocr_calls=" << stats.ocr_calls << " ocr_nonempty=" << stats.ocr_nonempty << "\n";
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
                   bool verbose, bool use_engine, int wait_timeout_ms, int ocr_cap)
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

	StageSet st;
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
	VsmChangeDetectParams cdp;

	VsmFrameImage prev; bool has_prev = false;
	int64 start = msecs();
	int64 deadline = start + (int64)seconds * 1000;
	int recoverable_timeouts = 0, unrecoverable = 0;
	Vector<double> per_frame_ms;      // full pipeline wall time / frame
	Vector<int64>  capture_gaps;      // receive-ts gaps
	int64 prev_ts = 0;
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
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		st.acquire.Add(acq_and_read);
		if(table.IsEmpty()) continue;
		if(prev_ts > 0) capture_gaps.Add(ts - prev_ts);
		prev_ts = ts;

		double f0 = NowMs();
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, game, verbose, ocr_cap);
		double f_done = NowMs();
		if(has_prev) {
			per_frame_ms.Add(f_done - f0);
			// capture-to-done latency: from TCP-receive wall-clock (ts) to now.
			// NOTE (Task 0279 finding): ts is the TCP-RECEIVE instant, NOT the
			// true glass/capture instant -- VideoServer does not expose the
			// latter, so this measures network+decode+process, not glass-to-glass.
			capture_to_done_sum += (double)(msecs() - ts); capture_to_done_n++;
		}
		if(!has_prev) { prev.Set(table.width, table.height, ~table.data); has_prev = true; }

		if(stats.frames > 0 && stats.frames % 25 == 0)
			Cout() << Format("progress: frames=%d elapsed=%d s regions=%d street_events=%d value_changes=%d ocr_calls=%d\n",
			                 stats.frames, (int)((msecs() - start) / 1000), stats.total_regions,
			                 rs.street_events, rs.value_changes, stats.ocr_calls);
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
	       << " unrecoverable=" << unrecoverable << "\n";
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
	Cout() << "ocr_calls=" << stats.ocr_calls << " ocr_nonempty=" << stats.ocr_nonempty << "\n";
	Cout() << "resolve: street_events=" << rs.street_events << " value_changes=" << rs.value_changes
	       << " pot_reads=" << rs.pot_reads << " final_board_count=" << rs.board_count << "\n";
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

GUI_APP_MAIN
{
#ifdef PLATFORM_WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	SetVppLogName(AppendFileName(GetCurrentDirectory(), "VideoLiveRecognitionLoop.log"));

	const Vector<String>& args = CommandLine();
	String mode, host = "127.0.0.1", frames_dir, dataset = kDatasetDefault;
	int port = 8082, seconds = 30, max_frames = 0, wait_timeout_ms = 4000;
	int ocr_cap = -1; // -1 = unlimited, 0 = OCR off, N = up to N OCR calls/frame
	bool verbose = false, use_engine = true;

	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--classify-selftest") mode = "selftest";
		else if(args[i] == "--offline-frames" && i + 1 < args.GetCount()) { mode = "offline"; frames_dir = args[++i]; }
		else if(args[i] == "--live") mode = "live";
		else if(args[i] == "--host" && i + 1 < args.GetCount()) host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount()) port = StrInt(args[++i]);
		else if(args[i] == "--seconds" && i + 1 < args.GetCount()) seconds = max(1, StrInt(args[++i]));
		else if(args[i] == "--max-frames" && i + 1 < args.GetCount()) max_frames = StrInt(args[++i]);
		else if(args[i] == "--wait-timeout-ms" && i + 1 < args.GetCount()) wait_timeout_ms = StrInt(args[++i]);
		else if(args[i] == "--dataset" && i + 1 < args.GetCount()) dataset = args[++i];
		else if(args[i] == "--ocr-cap" && i + 1 < args.GetCount()) ocr_cap = StrInt(args[++i]);
		else if(args[i] == "--no-ocr") ocr_cap = 0;
		else if(args[i] == "--verbose") verbose = true;
		else if(args[i] == "--no-engine") use_engine = false;
		else if(args[i] == "--help" || args[i] == "-h") mode = "help";
	}

	if(mode.IsEmpty() || mode == "help") {
		Cout() << "VideoLiveRecognitionLoop (Task 0280)\n"
		       << "Modes:\n"
		       << "  --classify-selftest        Leave-one-out classifier accuracy over the dataset\n"
		       << "  --offline-frames <dir>     Full pipeline over dataset source frames (timing)\n"
		       << "  --live                     Continuous loop against a running VideoServer\n"
		       << "Options:\n"
		       << "  --host <h> --port <p>       VideoServer address (default 127.0.0.1:8082)\n"
		       << "  --seconds <n>               Live run duration (default 30)\n"
		       << "  --max-frames <n>            Cap frames in offline mode\n"
		       << "  --wait-timeout-ms <n>       Live source wait timeout (default 4000)\n"
		       << "  --dataset <path>            Labeled dataset (default " << kDatasetDefault << ")\n"
		       << "  --ocr-cap <n>               Max OCR calls per frame (-1 unlimited, 0 off)\n"
		       << "  --no-ocr                    Disable OCR stage entirely\n"
		       << "  --no-engine                 Skip the real Game engine stage\n"
		       << "  --verbose                   Per-region log lines\n";
		return;
	}

	int rc = 0;
	if(mode == "selftest") rc = RunClassifySelfTest(dataset);
	else if(mode == "offline") rc = RunOfflineFrames(frames_dir, dataset, max_frames, verbose, use_engine, ocr_cap);
	else if(mode == "live") rc = RunLive(host, port, seconds, dataset, verbose, use_engine, wait_timeout_ms, ocr_cap);
	if(rc) SetExitCode(rc);
}
