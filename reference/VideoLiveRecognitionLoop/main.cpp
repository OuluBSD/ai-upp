#include <CtrlCore/CtrlCore.h> // GUI_APP_MAIN (engine pulls no Ctrl header itself)
#include <VisualStateModel/VisualStateModel.h>
#include <ComputerVision/ComputerVision.h> // Task 0290a: native MatchTemplate + OrbSystem (no OpenCV)
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

// Task 0290a: real, already-existing PokerStars card template library (rank
// glyphs used by MatchTemplate). Overridable with --templates <dir>.
static String g_templates_dir = "C:/Users/sblo/Dev/PKR/datasets/pokerstars/templates";

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
//      slides the real PokerStars rank-glyph library (templates/ranks/*.png,
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

// Load the real PokerStars rank-glyph library from <dir>/ranks/{rank}.png. Each
// glyph is tight-cropped to its bright (white) pixels then pre-rescaled to every
// calibrated scale, so per-frame recognition is a fixed set of MatchTemplate calls.
static CardTemplates LoadCardTemplates(const String& dir)
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
		for(int s = 0; s < kNumRankScales; s++) {
			int nw = max(1, (int)(tight.GetWidth()  * kRankScales[s] + 0.5));
			int nh = max(1, (int)(tight.GetHeight() * kRankScales[s] + 0.5));
			Image scg = Rescale(tight, nw, nh);
			ByteMat bm; ImageToGrayByteMat(scg, bm);
			rt.scaled.Add() = pick(bm);
		}
	}
	ct.ok = ct.ranks.GetCount() == 13;
	if(!ct.ok) Cerr() << "WARN: card templates incomplete (" << ct.ranks.GetCount() << "/13)\n";
	return ct;
}

// Suit from dominant card background colour (4-colour deck). Samples the card
// interior, skipping near-white glyph pixels. Returns "c"/"d"/"h"/"s" or "".
static String ClassifySuitByColor(const VsmFrameImage& table, const Rect& card)
{
	Rect r = RectC(card.left + 6, kBoardCardTop + 8,
	               max(1, card.Width() - 12), kBoardCardBot - kBoardCardTop - 20)
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
	Rect band = RectC(card.left - 2, kBoardCardTop, card.Width() + 4, kBoardCardBot - kBoardCardTop);
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
// THIS PokerStars client the dealer button is NOT a "D" glyph -- it is a small
// white-rimmed disc bearing the red PokerStars spade logo. The already-existing
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
	double last_pot = -1;
	// Task 0287: per-seat OCR'd values, attributed via RegionToSeat(). -1 /
	// empty means "not resolved from video yet for this seat" -> BuildSnapshot
	// falls back to the engine-seed value and labels the field engine-seed.
	double seat_stack[6]; // last OCR'd balance-plate value per seat index
	double seat_bet[6];   // last OCR'd bet-label value per seat index
	String seat_name[6];  // last OCR'd name-plate text per seat index
	// Task 0289: per-seat latest action word (from seat_action_bubble OCR) and
	// per-seat round-bet-total chip figure (from chip_badge_stack OCR).
	String seat_action[6];       // normalized action word ("Call"/"Fold"/...)
	double seat_round_total[6];  // last OCR'd chip_badge_stack value per seat, -1 none
	int    last_action_seat = -1;    // seat of the most recent action-bubble read
	int64  last_action_ms = 0;       // wall time of that read (for "fresh"/turn highlight)
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
                              const std::shared_ptr<Game>& game, bool verbose, int ocr_cap,
                              const CardTemplates& ct, const DealerTemplate& dt)
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
				}
			}
		}
		else if(IsOcrCategory(cl.category)) {
			double val;
			if(ParseChipValue(ocr_text, val)) {
				if(cl.category == "pot_label") {
					rs.pot_reads++;
					if(rs.last_pot >= 0 && fabs(val - rs.last_pot) > 0.001) {
						rs.value_changes++;
						rs.LogEvent(Format("pot: %.4g", val), verbose);
					}
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
						if(cl.category == "seat_balance_plate") {
							if(rs.seat_stack[seat] < 0 || fabs(rs.seat_stack[seat] - val) > 0.001)
								rs.LogEvent(Format("seat %d stack: %.4g", seat, val), verbose);
							rs.seat_stack[seat] = val;
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
		double wf = CardBackWhiteFraction(table, cb.rect);
		if(wf < 0) continue;
		int raw = (wf >= kCardBackWhiteFrac) ? 1 : 0;
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
		int strength = 0;
		int ts = DetectTurnSeat(table, strength);
		rs.turn_bar_strength = strength;
		if(ts != rs.turn_seat_video) {
			rs.turn_seat_video = ts;
			if(ts >= 0) rs.LogEvent(Format("turn: seat %d (timer bar, strength %d)", ts, strength), verbose);
		}
	}
	st.resolve.Add(NowMs() - t);

	// --- Task 0290c item 1: video-grounded dealer chip (template match) ---
	// THROTTLED to once every kDealerEveryFrames frames -- the dealer button is
	// static within a hand, and MatchTemplate over the felt band is the one
	// non-trivial cost here, so running it every frame would be wasteful. Logged
	// as an INDEPENDENT signal; agreement with the engine's GBUTTON_DEALER
	// indicator is reported in BuildSnapshot/Paint, not asserted here.
	static const int kDealerEveryFrames = 8;
	if(dt.ok && (stats.frames % kDealerEveryFrames == 0)) {
		t = NowMs();
		double dsc = -2; Point dc(-1, -1);
		int dseat = DetectDealerChip(table, dt, dsc, dc);
		rs.dealer_chip_score = dsc;
		if(dseat != rs.dealer_seat_video) {
			rs.dealer_seat_video = dseat;
			if(dseat >= 0)
				rs.LogEvent(Format("dealer chip: seat %d (template score %.2f at %d,%d)",
				                    dseat, dsc, dc.x, dc.y), verbose);
		}
		st.engine.Add(NowMs() - t);
	}

	// Structural street events + engine application from board region count.
	t = NowMs();
	if(board_regions_this_frame > 0 && ct.ok) {
		// Task 0290a: a board_card region appeared this frame -> re-recognize the
		// WHOLE board from raw pixels (felt-gap split + suit-colour + rank template
		// match), bounded to the fixed board band. This resolves all 3/4/5 cards at
		// once regardless of how the change detector merged them.
		Vector<String> dbg;
		Vector<int> recog = RecognizeBoardCards(table, ct, verbose ? &dbg : nullptr);
		new_board_cards.Clear();
		int resolved_cards = 0;
		for(int ci : recog) { new_board_cards.Add(ci); if(ci >= 0) resolved_cards++; }
		if(verbose)
			for(const String& d : dbg) Cout() << "    [board-recog] " << d << "\n";
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
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
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

	StageSet st;
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
	LoopStats stats; ResolveState rs;
	CardTemplates card_templates = LoadCardTemplates(g_templates_dir); // Task 0290a
	DealerTemplate dealer_template = LoadDealerTemplate(g_templates_dir); // Task 0290c item 1
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
		ProcessTableFrame(table, prev, has_prev, clf, ocr, ocr_available, cdp, st, stats, rs, oc, game, verbose, ocr_cap, card_templates, dealer_template);
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
	st.acquire.name = "acquire"; st.crop.name = "crop+conv"; st.change.name = "change_detect";
	st.classify.name = "classify"; st.ocr.name = "ocr"; st.resolve.name = "resolve"; st.engine.name = "engine";
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
// Task 0290a: Mode 5 -- offline card-recognition benchmark against the real
// video + sidecar ground truth. Deterministic, no server. Loads real frames at
// the sidecar's known board-transition timestamps (frame index == video
// second, see FRAME_TIME_MAPPING.md), runs the felt-split + suit-colour + rank
// template pipeline, and scores against the ground-truth board identities. Also
// measures real per-card TIMING of template-match vs ORB on a real card crop --
// the "is ORB slower?" question the task requires answering with real numbers.
// ===========================================================================
struct BoardCase { int frame; const char* board; };

static int RunCardRecogTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: card-recog-test (real board frames vs sidecar ground truth) ===\n";
	CardTemplates ct = LoadCardTemplates(templates_dir);
	Cout() << "rank templates: " << ct.ranks.GetCount() << "/13 (" << (ct.ok ? "OK" : "INCOMPLETE")
	       << ") from " << templates_dir << "\n";
	if(!ct.ok) { Cerr() << "ERROR: incomplete rank template library\n"; return 1; }

	// Hand-1 ground truth (bin/video_record_25min_20260716_203356.txt):
	//   flop 3d3sKh @ 00:00:18, turn 5c @ 00:00:27, river Ad @ 00:00:38.
	// Board persists between events; frame index == video second.
	static const BoardCase cases[] = {
		{18,"3d3sKh"},   {21,"3d3sKh"},   {24,"3d3sKh"},   {26,"3d3sKh"},
		{27,"3d3sKh5c"}, {30,"3d3sKh5c"}, {34,"3d3sKh5c"}, {37,"3d3sKh5c"},
		{38,"3d3sKh5cAd"},{41,"3d3sKh5cAd"},{45,"3d3sKh5cAd"},{47,"3d3sKh5cAd"},
	};
	int card_tot = 0, card_ok = 0, suit_ok = 0, rank_ok = 0, count_ok = 0, frame_tot = 0;
	for(const BoardCase& bc : cases) {
		String path = AppendFileName(frames_dir, Format("frame_%06d.jpg", bc.frame));
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(path, buf)) { Cerr() << "skip (load failed): " << path << "\n"; continue; }
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		if(table.IsEmpty()) { Cerr() << "skip (empty crop): " << path << "\n"; continue; }
		Vector<String> dbg;
		Vector<int> recog = RecognizeBoardCards(table, ct, &dbg);
		String bs = bc.board;
		Vector<int> exp;
		for(int i = 0; i + 1 < bs.GetCount(); i += 2) exp.Add(CardIndex(bs.Mid(i, 1), bs.Mid(i + 1, 1)));
		frame_tot++;
		bool cnt = recog.GetCount() == exp.GetCount(); count_ok += cnt;
		String line;
		for(int i = 0; i < exp.GetCount(); i++) {
			card_tot++;
			int got = i < recog.GetCount() ? recog[i] : -1;
			bool ok = (got == exp[i]); card_ok += ok;
			if(got >= 0) { if(got / 13 == exp[i] / 13) suit_ok++; if(got % 13 == exp[i] % 13) rank_ok++; }
			line << Format("  %s%s", ~FormatCardStr(exp[i]),
			               ok ? "=OK" : Format("!=%s", ~FormatCardStr(got)));
		}
		Cout() << Format("frame %02d (%d cards, count %s):%s\n",
		                 bc.frame, recog.GetCount(), cnt ? "OK" : "MISMATCH", ~line);
		for(const String& d : dbg) Cout() << "        " << d << "\n";
	}
	auto pct = [](int a, int b) { return b ? 100.0 * a / b : 0.0; };
	Cout() << "\n=== Board recognition accuracy (real frames vs sidecar) ===\n";
	Cout() << Format("frames=%d  card-count-correct=%d/%d  cards=%d\n", frame_tot, count_ok, frame_tot, card_tot);
	Cout() << Format("full card (rank+suit) correct = %d/%d (%.1f%%)\n", card_ok, card_tot, pct(card_ok, card_tot));
	Cout() << Format("suit-only correct             = %d/%d (%.1f%%)\n", suit_ok, card_tot, pct(suit_ok, card_tot));
	Cout() << Format("rank-only correct             = %d/%d (%.1f%%)\n", rank_ok, card_tot, pct(rank_ok, card_tot));

	// --- timing: template-match vs ORB on a real card crop (frame 18, first card) ---
	Cout() << "\n=== Per-card matcher timing (real card crop, frame 18) ===\n";
	VsmImageBuffer buf;
	if(LoadJpgToBuffer(AppendFileName(frames_dir, "frame_000018.jpg"), buf)) {
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
// Task 0290c: Mode 7 -- dealer-chip + turn-timer detection accuracy against the
// real 0263 frames + sidecar ground truth. Deterministic, no server. Runs the
// two new detectors on every real frame and scores them:
//   * DEALER: ground truth (bin/video_record_25min_20260716_203356.txt) has
//     dealer=seat4 for BOTH hands in this 0-55s window; seat4 is the BOTTOM seat
//     = mirror seat 0. So the expected video-detected dealer seat is 0 for every
//     frame where the button is on screen.
//   * TURN: expected acting seat per frame, encoded from the ground-truth action
//     sequence (the player genuinely deliberating before their next logged
//     action). -1 = a setup/dealing/showdown transition frame, not scored.
// Also prints real per-frame TIMING for the dealer template match (its one
// non-trivial cost) so the throttle choice is grounded in real numbers.
// ===========================================================================
static int RunDealerTurnTest(const String& frames_dir, const String& templates_dir)
{
	Cout() << "=== Mode: dealer-turn-test (real 0263 frames vs sidecar ground truth) ===\n";
	DealerTemplate dt = LoadDealerTemplate(templates_dir);
	Cout() << "dealer template: " << (dt.ok ? "OK" : "MISSING") << " (" << dt.scaled.GetCount()
	       << " scales) from " << templates_dir << "\n";
	if(!dt.ok) { Cerr() << "ERROR: dealer template missing\n"; return 1; }

	// Expected acting seat (mirror index) per frame from GT; -1 = not scored.
	// Sidecar->mirror: bottom(seat4)=0, leftbot(seat5)=1, lefttop(seat6)=2,
	// top(seat1)=3, righttop(seat2)=4, rightbot(seat3)=5.
	int exp_turn[56];
	for(int i = 0; i < 56; i++) exp_turn[i] = -1;
	auto setrange = [&](int a, int b, int v){ for(int i = a; i <= b && i < 56; i++) exp_turn[i] = v; };
	setrange(6,11,4); setrange(13,13,0); setrange(14,16,2); setrange(19,20,2);
	setrange(22,22,0); setrange(24,25,2); setrange(29,29,0); setrange(31,35,2);
	setrange(36,36,0); setrange(39,39,2); setrange(41,45,0); setrange(47,47,2);
	setrange(55,55,4);

	int dealer_present = 0, dealer_ok = 0;
	int turn_scored = 0, turn_ok = 0, turn_detected = 0;
	double dealer_ms_sum = 0; int dealer_ms_n = 0;
	for(int f = 0; f < 56; f++) {
		String path = AppendFileName(frames_dir, Format("frame_%06d.jpg", f));
		VsmImageBuffer buf;
		if(!LoadJpgToBuffer(path, buf)) continue;
		VsmFrameImage table = CropBufferToFrame(buf, kTableRect);
		if(table.IsEmpty()) continue;

		double t0 = NowMs();
		double dsc = -2; Point dc(-1, -1);
		int dseat = DetectDealerChip(table, dt, dsc, dc);
		dealer_ms_sum += NowMs() - t0; dealer_ms_n++;

		int strength = 0;
		int tseat = DetectTurnSeat(table, strength);

		String dline;
		if(dseat >= 0) {
			dealer_present++;
			bool ok = (dseat == 0);  // GT: dealer=seat4=bottom=mirror 0 this window
			dealer_ok += ok;
			dline = Format("dealer=seat%d %s (score %.2f @%d,%d)", dseat, ok ? "OK" : "MISPLACED", dsc, dc.x, dc.y);
		}
		else dline = Format("dealer=none (best score %.2f)", dsc);

		if(tseat >= 0) turn_detected++;
		String tline;
		if(exp_turn[f] >= 0) {
			turn_scored++;
			bool ok = (tseat == exp_turn[f]);
			turn_ok += ok;
			tline = Format("turn=seat%d exp=seat%d %s (str %d)", tseat, exp_turn[f], ok ? "OK" : "MISS", strength);
		}
		else tline = Format("turn=%s (str %d, GT n/a)", tseat >= 0 ? ~Format("seat%d", tseat) : "none", strength);

		Cout() << Format("f%02d: %-46s | %s\n", f, ~dline, ~tline);
	}
	auto pct = [](int a, int b){ return b ? 100.0 * a / b : 0.0; };
	Cout() << "\n=== Dealer-chip accuracy (GT: dealer on bottom/mirror-seat-0) ===\n";
	Cout() << Format("frames with a chip detected = %d/56; correctly on seat 0 = %d/%d (%.1f%%)\n",
	                 dealer_present, dealer_ok, dealer_present, pct(dealer_ok, dealer_present));
	Cout() << Format("dealer template match timing = %.1f ms/frame (%d frames, %d scales, felt-band search)\n",
	                 dealer_ms_n ? dealer_ms_sum / dealer_ms_n : 0.0, dealer_ms_n, kNumDealerScales);
	Cout() << "\n=== Turn-timer accuracy (GT-scored frames only) ===\n";
	Cout() << Format("frames with a timer bar detected = %d/56\n", turn_detected);
	Cout() << Format("GT-scored frames = %d; detected seat == GT acting seat = %d (%.1f%%)\n",
	                 turn_scored, turn_ok, pct(turn_ok, turn_scored));
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
	String card_dataset_out; // Task 0290a --card-dataset-dump output dir

	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--classify-selftest") mode = "selftest";
		else if(args[i] == "--offline-frames" && i + 1 < args.GetCount()) { mode = "offline"; frames_dir = args[++i]; }
		else if(args[i] == "--live") mode = "live";
		else if(args[i] == "--gui") mode = "gui";
		else if(args[i] == "--crop-safety-check") mode = "cropsafety";
		else if(args[i] == "--card-recog-test") { mode = "cardrecog"; if(frames_dir.IsEmpty()) frames_dir = "tmp/real_recording_0263_frames"; }
		else if(args[i] == "--card-dataset-dump" && i + 1 < args.GetCount()) { mode = "carddataset"; card_dataset_out = args[++i]; if(frames_dir.IsEmpty()) frames_dir = "tmp/real_recording_0263_frames"; }
		else if(args[i] == "--dealer-turn-test") { mode = "dealerturn"; if(frames_dir.IsEmpty()) frames_dir = "tmp/real_recording_0263_frames"; }
		else if(args[i] == "--frames-dir" && i + 1 < args.GetCount()) frames_dir = args[++i];
		else if(args[i] == "--templates" && i + 1 < args.GetCount()) g_templates_dir = args[++i];
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
		       << "  --card-recog-test           Task 0290a: board card recognition (felt-split +\n"
		       << "                               suit-colour + rank template match) accuracy vs the\n"
		       << "                               sidecar ground truth + ORB-vs-template timing\n"
		       << "  --card-dataset-dump <dir>   Task 0290a: dump labeled board-card crops + manifest\n"
		       << "  --dealer-turn-test          Task 0290c: dealer-chip template match + turn-timer\n"
		       << "                               bar detection accuracy vs the sidecar ground truth\n"
		       << "Options:\n"
		       << "  --host <h> --port <p>       VideoServer address (default 127.0.0.1:8082)\n"
		       << "  --seconds <n>               Live run duration (default 30)\n"
		       << "  --max-frames <n>            Cap frames in offline mode\n"
		       << "  --wait-timeout-ms <n>       Live source wait timeout (default 4000)\n"
		       << "  --dataset <path>            Labeled dataset (default " << kDatasetDefault << ")\n"
		       << "  --frames-dir <dir>          Real frames for card-recog-test/dataset-dump\n"
		       << "  --templates <dir>           Card template library (default " << g_templates_dir << ")\n"
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
	else if(mode == "cardrecog") rc = RunCardRecogTest(frames_dir, g_templates_dir);
	else if(mode == "carddataset") rc = RunCardDatasetDump(frames_dir, g_templates_dir, card_dataset_out);
	else if(mode == "dealerturn") rc = RunDealerTurnTest(frames_dir, g_templates_dir);
	if(rc) SetExitCode(rc);
}
