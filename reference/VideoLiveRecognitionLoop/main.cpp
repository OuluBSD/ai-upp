#include <CtrlCore/CtrlCore.h> // GUI_APP_MAIN (engine pulls no Ctrl header itself)
#include <VisualStateModel/VisualStateModel.h>
#include <plugin/jpg/jpg.h>

#include <TexasHoldem/TexasHoldemLocalGame.h>
#include <TexasHoldem/TexasHoldemLogicState.h> // card/board art helpers (reuse game/CardRender via GameTable's own encoding)
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
// across all 56 tracking frames of the M12 recording (tracking.json).
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
static const Rect kTableRect = RectC(968, 1, 944, 682);

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
	// Task 0287: seat_name_plate added -- the classifier already identifies name
	// plates, but no text extraction ran on them before, so the live mirror only
	// ever showed generic "SeatN". Balance/bet were already OCR'd; names are the
	// one new OCR category here.
	return cat == "pot_label" || cat == "seat_balance_plate"
	    || cat == "seat_bet_label" || cat == "seat_name_plate";
}

// ---------------------------------------------------------------------------
// Task 0287: map a classified region's table-space rect to a seat index (0-5).
//
// This is a fixed-layout 6-max PokerStars table (same premise as kTableRect
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
struct ResolveState {
	ResolveState() {
		for(int i = 0; i < 5; i++) board_cards.Add(-1);
		for(int i = 0; i < 6; i++) { seat_stack[i] = -1; seat_bet[i] = -1; }
	}
	// board tracking (structural tier)
	int  board_count = 0;
	Vector<int> board_cards;
	int  street_events = 0; // flop/turn/river dealt
	// value tracking (OCR tier): slot key "xxYY" -> last numeric value
	VectorMap<String, double> slot_value;
	int  value_changes = 0;
	int  pot_reads = 0;
	double last_pot = -1;
	// Task 0287: per-seat OCR'd values, attributed via RegionToSeat(). -1 /
	// empty means "not resolved from video yet for this seat" -> BuildSnapshot
	// falls back to the engine-seed value and labels the field engine-seed.
	double seat_stack[6]; // last OCR'd balance-plate value per seat index
	double seat_bet[6];   // last OCR'd bet-label value per seat index
	String seat_name[6];  // last OCR'd name-plate text per seat index
};

struct LoopStats {
	int frames = 0;
	int total_regions = 0;
	VectorMap<String, int> category_counts;
	int ocr_calls = 0, ocr_nonempty = 0;
	int classified = 0, unresolved = 0;
};

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

// Process one already-cropped table frame against prev; returns whether prev
// was updated (always true after first frame).
static void ProcessTableFrame(const VsmFrameImage& table, VsmFrameImage& prev, bool has_prev,
                              VsmLiveRegionClassifier& clf, VsmTesseractOcrEngine& ocr,
                              bool ocr_available, const VsmChangeDetectParams& cdp,
                              StageSet& st, LoopStats& stats, ResolveState& rs, OcrCacheState& oc,
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
		int cache_dist = -1; bool cache_hit = false;
		if(ocr_available && IsOcrCategory(cl.category) && (ocr_cap < 0 || ocr_used < ocr_cap)) {
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
				VsmFrameImage ocr_crop = CropFrameImage(table, r);
				VsmOcrRequest req;
				req.semantic = cl.category;     // Task 0277's semantic field, first real caller
				req.region_id = Format("%d,%d,%d,%d", r.left, r.top, r.Width(), r.Height());
				VsmOcrResult res = ocr.Execute(ocr_crop, req);
				st.ocr.Add(NowMs() - t);
				ocr_text = res.text; ocr_conf = res.confidence;
				stats.ocr_calls++;
				if(!TrimBoth(ocr_text).IsEmpty()) stats.ocr_nonempty++;
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

		// --- stage: resolve ---
		t = NowMs();
		if(cl.category == "board_card") {
			board_regions_this_frame++;
			int ci = CardIndex(cl.rank, cl.suit);
			if(ci >= 0) new_board_cards.Add(ci);
		}
		else if(cl.category == "seat_name_plate") {
			// Task 0287: name plates carry text, not a chip value -- store the
			// trimmed OCR string, attributed to a seat by rect position.
			String nm = TrimBoth(ocr_text);
			if(!nm.IsEmpty()) {
				int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
				if(seat >= 0 && seat < 6) {
					if(rs.seat_name[seat] != nm) rs.value_changes++;
					rs.seat_name[seat] = nm;
				}
			}
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
					// Task 0287: also attribute to a seat index so BuildSnapshot
					// can render it on the right seat (previously the position
					// key was stored but the seat identity was discarded).
					int seat = RegionToSeat(r.left, r.top, r.Width(), r.Height());
					if(seat >= 0 && seat < 6) {
						if(cl.category == "seat_balance_plate") rs.seat_stack[seat] = val;
						else if(cl.category == "seat_bet_label")  rs.seat_bet[seat]   = val;
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
				ss.is_dealer = ((int)p->getMyUniqueID() == s.dealer);
				// Task 0287: override name/stack/bet with the real per-seat OCR
				// values WHEN AVAILABLE (RegionToSeat-attributed), keeping the
				// engine-seed value only where nothing has been recognized yet.
				// The render index n matches RegionToSeat's seat convention (both
				// 0=bottom..5=right-bottom), so rs.*[n] lines up with seats[n].
				if(n < 6) {
					if(!rs.seat_name[n].IsEmpty()) { ss.name = rs.seat_name[n]; ss.name_live = true; }
					if(rs.seat_stack[n] >= 0)      { ss.stack = (int)(rs.seat_stack[n] + 0.5); ss.stack_live = true; }
					if(rs.seat_bet[n] >= 0)        { ss.bet = (int)(rs.seat_bet[n] + 0.5); ss.bet_live = true; }
				}
				n++;
			}
			s.seat_count = n;
		}
	}
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
	}

	StageSet st;
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
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
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap);
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

	StageSet st;
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
	VsmChangeDetectParams cdp;
	OcrCacheState oc; oc.enabled = ocr_cache_enabled; oc.threshold = ocr_cache_threshold;

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
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap);
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
	Cout() << "ocr_calls=" << stats.ocr_calls << " ocr_nonempty=" << stats.ocr_nonempty
	       << " ocr_cache_hits=" << oc.hits << " ocr_cache_misses=" << oc.misses
	       << Format(" (cache %s, threshold=%d)\n", oc.enabled ? "ENABLED" : "disabled", oc.threshold);
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
			// seat plate
			Color plate = ss.turn ? Color(70, 70, 30) : Color(28, 34, 40);
			w.DrawRect(bxx, byy, boxw, boxh, plate);
			w.DrawRect(bxx, byy, boxw, 1, Color(80,90,100));
			w.DrawRect(bxx, byy+boxh-1, boxw, 1, Color(80,90,100));
			w.DrawRect(bxx, byy, 1, boxh, Color(80,90,100));
			w.DrawRect(bxx+boxw-1, byy, 1, boxh, Color(80,90,100));
			// Task 0287: colour per-seat fields by provenance -- LIVE-DRIVEN
			// green (Color(150,230,160)) when the value came from real video
			// OCR this run, ENGINE-SEED amber (Color(235,180,120)) otherwise,
			// matching the right-hand legend's colour convention.
			const Color kLive = Color(150, 230, 160), kSeed = Color(235, 180, 120);
			String nm = ss.name.IsEmpty() ? Format("Seat%d", ss.seat) : ss.name;
			if(ss.is_dealer) nm << " (D)";
			Color namec = ss.name_live ? kLive : (ss.active ? White() : Color(150,150,150));
			w.DrawText(bxx + 6, byy + 4, nm, sf, namec);
			w.DrawText(bxx + 6, byy + 22, Format("stack %d", ss.stack), sf,
			           ss.stack_live ? kLive : kSeed);
			String betstr = ss.bet > 0 ? Format("bet %d", ss.bet) : String();
			if(!betstr.IsEmpty())
				w.DrawText(bxx + 6, byy + 40, betstr, sf, ss.bet_live ? kLive : kSeed);
			String ac = ActionName(ss.action);         // action stays engine-seed
			if(!ac.IsEmpty()) {
				int axoff = betstr.IsEmpty() ? 0 : GetTextSize(betstr, sf).cx + 10;
				w.DrawText(bxx + 6 + axoff, byy + 40, ac, sf, kSeed);
			}
			// two face-down hole cards (per-seat hole-card RECOGNITION not built --
			// show realistic backs, never fabricate/reveal engine-dealt faces)
			Image back = TexasHoldemGetCardBackReferenceImage(Size(scw, sch), "default");
			DrawCard(w, bxx + boxw - 2*scw - 8, byy + boxh - sch - 4, scw, sch, back);
			DrawCard(w, bxx + boxw - scw - 4,   byy + boxh - sch - 4, scw, sch, back);
		}

		// ---- right-hand status / honesty panel ----
		int panelx = sz.cx - 250, panely = 88;
		w.DrawRect(panelx, panely, 236, sz.cy - panely - 16, Color(16, 22, 28));
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
		line("LIVE-DRIVEN:", Color(150, 230, 160));
		line("  board cards, street, pot(OCR),", Color(150, 230, 160));
		line("  per-seat name/stack/bet (OCR),", Color(150, 230, 160));
		line("  green when read (Task 0287).", Color(150, 230, 160));
		ty += 4;
		line("ENGINE-SEED (not video-driven):", Color(235, 180, 120));
		line("  per-seat action; hole cards", Color(235, 180, 120));
		line("  (face-down). Amber seat fields", Color(235, 180, 120));
		line("  = not yet OCR'd this run.", Color(235, 180, 120));
		line("  Per-seat action resolution was", Color(180, 180, 180));
		line("  left unbuilt in Task 0280.", Color(180, 180, 180));
		if(!status.IsEmpty()) { ty += 6; line(status, Color(200, 200, 210)); }
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
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
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

			ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap);
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
	// Task 0286 Part B: approximate-hash OCR result cache, ON by default (this
	// IS the optimization being delivered) -- see OcrCacheState's comment for
	// the design AND the "SAFETY OVERRIDE" note explaining why 40 (not the
	// higher number the live-loop calibration alone suggested) is used.
	bool ocr_cache_enabled = true;
	int  ocr_cache_threshold = 40; // real-evidence calibrated, see OcrCacheState's comment
	Vector<String> crop_safety_lists;

	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--classify-selftest") mode = "selftest";
		else if(args[i] == "--offline-frames" && i + 1 < args.GetCount()) { mode = "offline"; frames_dir = args[++i]; }
		else if(args[i] == "--live") mode = "live";
		else if(args[i] == "--gui") mode = "gui";
		else if(args[i] == "--crop-safety-check") mode = "cropsafety";
		else if(args[i] == "--crop-list" && i + 1 < args.GetCount()) crop_safety_lists.Add(args[++i]);
		else if(args[i] == "--host" && i + 1 < args.GetCount()) host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount()) port = StrInt(args[++i]);
		else if(args[i] == "--seconds" && i + 1 < args.GetCount()) seconds = max(1, StrInt(args[++i]));
		else if(args[i] == "--max-frames" && i + 1 < args.GetCount()) max_frames = StrInt(args[++i]);
		else if(args[i] == "--wait-timeout-ms" && i + 1 < args.GetCount()) wait_timeout_ms = StrInt(args[++i]);
		else if(args[i] == "--dataset" && i + 1 < args.GetCount()) dataset = args[++i];
		else if(args[i] == "--ocr-cap" && i + 1 < args.GetCount()) ocr_cap = StrInt(args[++i]);
		else if(args[i] == "--no-ocr") ocr_cap = 0;
		else if(args[i] == "--no-ocr-cache") ocr_cache_enabled = false;
		else if(args[i] == "--ocr-cache-threshold" && i + 1 < args.GetCount()) ocr_cache_threshold = StrInt(args[++i]);
		else if(args[i] == "--verbose") verbose = true;
		else if(args[i] == "--no-engine") use_engine = false;
		else if(args[i] == "--help" || args[i] == "-h") mode = "help";
	}

	if(mode.IsEmpty() || mode == "help") {
		Cout() << "VideoLiveRecognitionLoop (Task 0280/0286)\n"
		       << "Modes:\n"
		       << "  --classify-selftest        Leave-one-out classifier accuracy over the dataset\n"
		       << "  --offline-frames <dir>     Full pipeline over dataset source frames (timing)\n"
		       << "  --live                     Continuous loop against a running VideoServer\n"
		       << "  --gui                      Live mirror window of the driven Game (Task 0281);\n"
		       << "                               background recognition loop + main-thread TopWindow,\n"
		       << "                               runs until the window is closed\n"
		       << "  --crop-safety-check         Task 0286: false-hit risk of the OCR cache over the\n"
		       << "                               31-crop Task 0274 validation set (--crop-list to\n"
		       << "                               override; defaults to both hand1/hand2 lists)\n"
		       << "Options:\n"
		       << "  --host <h> --port <p>       VideoServer address (default 127.0.0.1:8082)\n"
		       << "  --seconds <n>               Live run duration (default 30)\n"
		       << "  --max-frames <n>            Cap frames in offline mode\n"
		       << "  --wait-timeout-ms <n>       Live source wait timeout (default 4000)\n"
		       << "  --dataset <path>            Labeled dataset (default " << kDatasetDefault << ")\n"
		       << "  --ocr-cap <n>               Max OCR calls per frame (-1 unlimited, 0 off)\n"
		       << "  --no-ocr                    Disable OCR stage entirely\n"
		       << "  --no-ocr-cache               Disable the Task 0286 approximate-hash OCR cache\n"
		       << "                               (enabled by default; use for before/after A-B runs)\n"
		       << "  --ocr-cache-threshold <n>   Tight signature-distance threshold (default 40)\n"
		       << "  --crop-list <path>           (crop-safety-check) add a category\\tpath list file\n"
		       << "  --no-engine                 Skip the real Game engine stage\n"
		       << "  --verbose                   Per-region log lines\n";
		return;
	}

	int rc = 0;
	if(mode == "selftest") rc = RunClassifySelfTest(dataset);
	else if(mode == "offline") rc = RunOfflineFrames(frames_dir, dataset, max_frames, verbose, use_engine, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "live") rc = RunLive(host, port, seconds, dataset, verbose, use_engine, wait_timeout_ms, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "gui") rc = RunGui(host, port, dataset, verbose, use_engine, wait_timeout_ms, ocr_cap, ocr_cache_enabled, ocr_cache_threshold);
	else if(mode == "cropsafety") {
		if(crop_safety_lists.IsEmpty()) {
			crop_safety_lists.Add("tmp/_task0274_final_crop_list_hand1.txt");
			crop_safety_lists.Add("tmp/_task0274_final_crop_list_hand2.txt");
		}
		rc = RunCropSafetyCheck(crop_safety_lists, ocr_cache_threshold);
	}
	if(rc) SetExitCode(rc);
}
