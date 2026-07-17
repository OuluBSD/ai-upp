#include "VideoConfidenceTieredCandidates.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// rgb8x8 crop signature - copied VERBATIM from VideoChangedRegionReview/
// main.cpp (Signature(), ~line 58) so the template-match tier compares
// candidate crops against the confirmed-crop library using the exact same
// deterministic fingerprint 0251's review groups were keyed on.
// ---------------------------------------------------------------------------
static String Signature(const Image& image)
{
	String result = "rgb8x8:";
	for(int y = 0; y < 8; y++)
		for(int x = 0; x < 8; x++) {
			int sx = min(image.GetWidth() - 1, x * image.GetWidth() / 8);
			int sy = min(image.GetHeight() - 1, y * image.GetHeight() / 8);
			Color c = image[sy][sx];
			result << Format("%02x%02x%02x", c.GetR() >> 4, c.GetG() >> 4, c.GetB() >> 4);
		}
	return result;
}

static String Resolve(const String& path)
{
	if(path.IsEmpty()) return String();
	String p = path;
	p.Replace("\\", DIR_SEPS);
	if(FileExists(p)) return NormalizePath(p);
	String candidate = AppendFileName(GetCurrentDirectory(), p);
	return FileExists(candidate) ? NormalizePath(candidate) : p;
}

// ---------------------------------------------------------------------------
// Task 0267: rgb8x8 near-match (fuzzy template match).
//
// The rgb8x8 signature (see Signature() above) is "rgb8x8:" followed by 64
// blocks (an 8x8 grid over the crop), each block 6 hex chars = 3 channels x
// 2 hex digits, where each channel is a 4-bit-quantized nibble (color >> 4,
// range 0-15) printed as "%02x" (so the first hex digit of each channel is
// always '0'). Distance below is the Manhattan (L1) distance between two
// signatures' per-block per-channel nibble values: max possible distance is
// 64 blocks * 3 channels * 15 (max nibble delta) = 2880.
//
// Threshold justification (real measurements, Task 0267, real Aludra capture
// tmp/real_recording_0263_tracked): comparing genuinely-identical-content
// crops of DIFFERENT frames at the SAME crop rect (verified by opening both
// images):
//   - table felt background strip, 3 frame pairs (frames 30/38/46):
//     distance 38, 66, 74 (out of 2880)
//   - facedown hole card back, 48 frames apart (frame 6 vs frame 54):
//     distance 30 (out of 2880)
// vs. genuinely-different-content crops at the same rect/dimensions:
//   - pot_label text, different pot amounts: distance 356, 454
//   - seat status badge, different text ("Bet 107.5 BB" vs "Raise All In" /
//     vs "wegohigh 103.2 BB"): distance 328, 285
// Same-content real pairs topped out at 74; different-content real pairs
// bottomed out at 285 - a clear ~4x gap. NEAR_MATCH_MAX_DISTANCE is set well
// inside that gap, biased toward the same-content side (conservative: a
// false "near match" is worse than an unresolved candidate).
//
// Caveat found during this investigation: one seat-status-badge pair that
// shares identical text ("Bet 107.5 BB" in both frames) still measured
// distance 330 because of a real pixel-level UI animation (an active-badge
// glow pulse + timer-bar depletion) between the two frames - i.e. "same
// logical state" is not always "same pixels" on this UI. That pair is
// excluded from the same-content evidence above precisely because visual
// inspection showed its pixels really do differ (confirmed via
// `magick compare -metric RMSE`, 12.3% vs ~1.6-1.8% for the accepted
// same-content pairs), not because it was cherry-picked to fit.
// ---------------------------------------------------------------------------
static int HexNibble(int c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}

static int HexByte(const String& s, int off)
{
	return HexNibble(s[off]) * 16 + HexNibble(s[off + 1]);
}

static const char *RGB8X8_PREFIX = "rgb8x8:";
static const int RGB8X8_BLOCK_COUNT = 64;
static const int RGB8X8_MAX_DISTANCE = RGB8X8_BLOCK_COUNT * 3 * 15; // 2880
static const int NEAR_MATCH_MAX_DISTANCE = 120; // see justification comment above

// Manhattan distance between two rgb8x8 signature strings of identical
// encoded length. Returns -1 if either string isn't a well-formed rgb8x8
// signature (wrong prefix/length) - callers must not force a comparison in
// that case. Does NOT itself check source-crop pixel dimensions; callers
// must only invoke this on signatures whose crops are already known to have
// matching width/height (see LibEntry::width/height, Candidate::sig_width/
// sig_height below) - two different-sized crops can produce equal-length
// signature strings (the 8x8 grid is always sampled at 64 points regardless
// of source size), so string length alone cannot detect a dimension mismatch.
static int SignatureDistance(const String& a, const String& b)
{
	int prefix_len = (int)strlen(RGB8X8_PREFIX);
	int expect_len = prefix_len + RGB8X8_BLOCK_COUNT * 6;
	if(a.GetCount() != expect_len || b.GetCount() != expect_len) return -1;
	if(!a.StartsWith(RGB8X8_PREFIX) || !b.StartsWith(RGB8X8_PREFIX)) return -1;
	int total = 0;
	for(int i = 0; i < RGB8X8_BLOCK_COUNT; i++) {
		int off = prefix_len + i * 6;
		int r1 = HexByte(a, off),     g1 = HexByte(a, off + 2), b1 = HexByte(a, off + 4);
		int r2 = HexByte(b, off),     g2 = HexByte(b, off + 2), b2 = HexByte(b, off + 4);
		total += abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
	}
	return total;
}

// Parse the "WIDTHxHEIGHT:hash" exact_fingerprint field (written by
// VideoChangedRegionReview, e.g. "168x96:ede4c708") into numeric dimensions,
// so the near-match tier can enforce same-dimension comparisons even though
// the rgb8x8 signature string itself doesn't encode crop dimensions.
static void ParseFingerprintDims(const String& fingerprint, int& w, int& h)
{
	w = 0; h = 0;
	int colon = fingerprint.Find(':');
	String dims = colon >= 0 ? fingerprint.Left(colon) : fingerprint;
	int xpos = dims.Find('x');
	if(xpos < 0) return;
	w = ScanInt(dims.Left(xpos));
	h = ScanInt(dims.Mid(xpos + 1));
}

// Join key that survives absolute-vs-relative and live-vs-replay path
// differences: the last 3 path segments (tracker_dir/changed_regions/basename).
// Basenames ALONE collide across the _live_tracked and _replay_tracked
// recordings the candidates are drawn from, so basename matching would
// wrongly conflate them.
static String PathKey3(const String& path)
{
	String p = UnixPath(path);
	Vector<String> seg = Split(p, '/', false);
	String key;
	int start = max(0, seg.GetCount() - 3);
	for(int i = start; i < seg.GetCount(); i++) {
		if(!key.IsEmpty()) key << "/";
		key << seg[i];
	}
	return ToLower(key);
}

static String Text(ValueMap map, const char *key)
{
	Value v = map.Get(key, Value());
	return IsVoid(v) || IsNull(v) ? String() : AsString(v);
}

static String StreetName(int street)
{
	switch(street) {
		case 1: return "flop";
		case 2: return "turn";
		case 3: return "river";
		default: return "preflop";
	}
}

// ---------------------------------------------------------------------------
// Structural tier: a thin sibling of reference/VideoGameEngineSyncer's
// GameEngineSyncer. It reuses the SAME tracked-state field pattern
// (tracked_board_cards / tracked_dealer_seat / tracked_hand_id per table) and
// the SAME board-empty / active-card-count logic (IsBoardEmpty /
// GetActiveCardCount), but instead of emitting *divergences* it emits positive
// *keypoint events*. Reimplemented here rather than #include-ing GameEngineSyncer
// because that class lives in its package's main.cpp (not its header) and pulls
// the heavy TexasHoldem+CtrlLib GUI dependency; this console tool only needs the
// board-count transition logic, which is small. (Documented deviation.)
// ---------------------------------------------------------------------------
struct TableTracker : Moveable<TableTracker> {
	bool board_known = false;
	int  board_count = 0;
	bool dealer_known = false;
	int  dealer_seat = -1;
	bool hand_known = false;
	int  hand_id = -1;
};

struct StructuralExtractor {
	VectorMap<int, TableTracker> trackers; // key: table_id
	Vector<KeypointEvent> events;

	void Observe(int table_id, int frame, bool board_known, int board_count,
	             bool dealer_known, int dealer_seat, bool hand_known, int hand_id)
	{
		TableTracker& t = trackers.GetAdd(table_id);

		bool new_hand = false;
		if(hand_known && t.hand_known && hand_id != t.hand_id) {
			new_hand = true;
			KeypointEvent& e = events.Add();
			e.event_type = "new_hand"; e.table_id = table_id; e.frame = frame;
			e.description = Format("hand_id %d -> %d", t.hand_id, hand_id);
		}

		// board-empty -> board-populated (or growth) => flop/turn/river dealt
		if(board_known && t.board_known && !new_hand) {
			if(t.board_count == 0 && board_count >= 3) {
				KeypointEvent& e = events.Add();
				e.event_type = "flop_dealt"; e.table_id = table_id; e.frame = frame;
				e.from_count = t.board_count; e.to_count = board_count; e.street = 1;
				e.description = Format("board 0 -> %d (flop)", board_count);
			}
			else if(t.board_count == 3 && board_count == 4) {
				KeypointEvent& e = events.Add();
				e.event_type = "turn_dealt"; e.table_id = table_id; e.frame = frame;
				e.from_count = 3; e.to_count = 4; e.street = 2;
				e.description = "board 3 -> 4 (turn)";
			}
			else if(t.board_count == 4 && board_count == 5) {
				KeypointEvent& e = events.Add();
				e.event_type = "river_dealt"; e.table_id = table_id; e.frame = frame;
				e.from_count = 4; e.to_count = 5; e.street = 3;
				e.description = "board 4 -> 5 (river)";
			}
			else if(t.board_count > 0 && board_count == 0) {
				KeypointEvent& e = events.Add();
				e.event_type = "new_hand"; e.table_id = table_id; e.frame = frame;
				e.from_count = t.board_count; e.to_count = 0;
				e.description = Format("board %d -> 0 (community cards reset / new hand)", t.board_count);
			}
		}

		if(dealer_known && t.dealer_known && dealer_seat != t.dealer_seat && !new_hand) {
			KeypointEvent& e = events.Add();
			e.event_type = "dealer_rotation"; e.table_id = table_id; e.frame = frame;
			e.from_seat = t.dealer_seat; e.to_seat = dealer_seat;
			e.description = Format("dealer seat %d -> %d", t.dealer_seat, dealer_seat);
		}

		// update tracked state
		if(board_known) { t.board_known = true; t.board_count = board_count; }
		if(dealer_known) { t.dealer_known = true; t.dealer_seat = dealer_seat; }
		if(hand_known) { t.hand_known = true; t.hand_id = hand_id; }
	}
};

// Count active (dealt) cards in a board_cards array (value -1 == not dealt),
// mirroring GameEngineSyncer::GetActiveCardCount.
static int ActiveCardCount(const ValueArray& board)
{
	int n = 0;
	for(int i = 0; i < board.GetCount(); i++)
		if((int)board[i] != -1) n++;
	return n;
}

// Feed a native VsmProductionRecordOutWrapper / TexasHoldemLogicState JSONL
// (the format reference/VideoGameEngineSyncer consumes) into the extractor.
static void FeedLogicJsonl(StructuralExtractor& ex, const String& path)
{
	String content = LoadFile(path);
	Vector<String> lines = Split(content, '\n', false);
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if(line.IsEmpty()) continue;
		Value root = ParseJSON(line);
		if(IsError(root) || !IsValueMap(root)) continue;
		ValueMap m = root;
		ValueMap ls = m;
		if(m.Find("logic_state") >= 0 && IsValueMap(m["logic_state"]))
			ls = m["logic_state"];

		int frame = ls.Find("frame_id") >= 0 ? (int)ls["frame_id"] : i;

		bool board_known = ls.Find("board_cards_known") >= 0 ? (bool)ls["board_cards_known"] : false;
		int board_count = 0;
		if(ls.Find("board_cards") >= 0 && IsValueArray(ls["board_cards"]))
			board_count = ActiveCardCount(ls["board_cards"]);

		bool dealer_known = false; int dealer_seat = -1;
		if(ls.Find("dealer_seat_known") >= 0 && (bool)ls["dealer_seat_known"]) {
			dealer_known = true; dealer_seat = (int)ls["dealer_seat"];
		}
		else if(m.Find("derived_dealer_seat_known") >= 0 && (bool)m["derived_dealer_seat_known"]) {
			dealer_known = true; dealer_seat = (int)m["derived_dealer_seat"];
		}

		bool hand_known = ls.Find("hand_id_known") >= 0 ? (bool)ls["hand_id_known"] : false;
		int hand_id = ls.Find("hand_id") >= 0 ? (int)ls["hand_id"] : -1;

		ex.Observe(0, frame, board_known, board_count, dealer_known, dealer_seat, hand_known, hand_id);
	}
}

// Feed an observer table_state.json (VideoTableStateExtractor output, the
// per-frame board state that actually accompanies the changed-region
// candidates) into the extractor.
static void FeedTableState(StructuralExtractor& ex, const String& path)
{
	Value root = ParseJSON(LoadFile(path));
	if(IsError(root) || !IsValueMap(root)) return;
	ValueMap m = root;
	ValueArray tables = m.Get("tables", ValueArray());
	// process ordered by (frame_index, table_id) so per-table trackers see
	// frames in ascending order
	struct Row { int frame; int table; bool bk; int bc; bool dk; int ds; };
	Vector<Row> rows;
	for(int i = 0; i < tables.GetCount(); i++) {
		ValueMap t = tables[i];
		Row r;
		r.frame = t.Find("frame_index") >= 0 ? (int)t["frame_index"] : i;
		r.table = t.Find("table_id") >= 0 ? (int)t["table_id"] : 0;
		r.bk = t.Find("board_card_count") >= 0;
		r.bc = r.bk ? (int)t["board_card_count"] : 0;
		r.dk = t.Find("dealer_seat") >= 0 && !IsNull(t["dealer_seat"]);
		r.ds = r.dk ? (int)t["dealer_seat"] : -1;
		rows.Add(r);
	}
	Sort(rows, [](const Row& a, const Row& b){ return a.frame != b.frame ? a.frame < b.frame : a.table < b.table; });
	for(const Row& r : rows)
		ex.Observe(r.table, r.frame, r.bk, r.bc, r.dk, r.ds, false, -1);
}

static Value EventToValue(const KeypointEvent& e)
{
	ValueMap m;
	m("event_type", e.event_type);
	m("table_id", e.table_id);
	m("frame", e.frame);
	if(e.from_count >= 0) m("from_count", e.from_count);
	if(e.to_count >= 0) m("to_count", e.to_count);
	if(e.street >= 0) m("street", e.street);
	if(e.from_seat >= 0 || e.to_seat >= 0) { m("from_seat", e.from_seat); m("to_seat", e.to_seat); }
	m("description", e.description);
	return m;
}

// ---------------------------------------------------------------------------
// Template-match tier library: crops from changed_region_review.json that have
// a real (non-empty) reviewed label AND a review_status other than "unknown".
// ---------------------------------------------------------------------------
struct LibEntry : Moveable<LibEntry> {
	String signature, label, rank, suit, review_status, sample_path;
	int width = 0, height = 0; // parsed from exact_fingerprint, for near-match dimension gating
};

static VectorMap<String, LibEntry> LoadLibrary(const String& review_path, int& raw_group_count)
{
	VectorMap<String, LibEntry> lib; // key: normalized_signature
	raw_group_count = 0;
	if(review_path.IsEmpty() || !FileExists(review_path)) return lib;
	Value root = ParseJSON(LoadFile(review_path));
	if(IsError(root) || !IsValueMap(root)) return lib;
	ValueMap m = root;
	ValueArray groups = m.Get("groups", ValueArray());
	raw_group_count = groups.GetCount();
	for(int i = 0; i < groups.GetCount(); i++) {
		ValueMap g = groups[i];
		String label = Text(g, "label");
		String status = Text(g, "review_status");
		String sig = Text(g, "normalized_signature");
		if(label.IsEmpty()) continue;                 // no confirmed label => not in library
		if(status.IsEmpty() || status == "unknown") continue; // not human-confirmed
		if(sig.IsEmpty()) continue;
		LibEntry& e = lib.GetAdd(sig);
		e.signature = sig; e.label = label; e.rank = Text(g, "rank");
		e.suit = Text(g, "suit"); e.review_status = status;
		e.sample_path = Text(g, "sample_path");
		ParseFingerprintDims(Text(g, "exact_fingerprint"), e.width, e.height);
	}
	return lib;
}

// ---------------------------------------------------------------------------
// Candidate + tiering
// ---------------------------------------------------------------------------
struct Candidate : Moveable<Candidate> {
	int index = -1;
	int frame_index = -1;
	int table_id = -1;
	String candidate_type;
	ValueMap rect;
	Vector<String> semantic;
	String crop_path;
	String signature;         // computed lazily
	int sig_width = 0, sig_height = 0; // crop pixel dimensions, for near-match dimension gating
	bool sig_ok = false;

	// resolution
	String tier = "unresolved";
	String label;
	double confidence = 0.0;
	ValueMap evidence;
};

static void LoadCandidates(const String& path, Vector<Candidate>& out)
{
	Value root = ParseJSON(LoadFile(path));
	if(IsError(root) || !IsValueMap(root)) return;
	ValueMap m = root;
	ValueArray arr = m.Get("candidates", ValueArray());
	for(int i = 0; i < arr.GetCount(); i++) {
		ValueMap c = arr[i];
		Candidate& cd = out.Add();
		cd.index = i;
		cd.frame_index = c.Find("frame_index") >= 0 ? (int)c["frame_index"] : -1;
		cd.table_id = c.Find("table_id") >= 0 ? (int)c["table_id"] : -1;
		cd.candidate_type = Text(c, "candidate_type");
		if(c.Find("rect") >= 0 && IsValueMap(c["rect"])) cd.rect = c["rect"];
		cd.crop_path = Text(c, "crop_path");
		if(c.Find("semantic_hits") >= 0 && IsValueArray(c["semantic_hits"])) {
			ValueArray hits = c["semantic_hits"];
			for(int j = 0; j < hits.GetCount(); j++) {
				ValueMap h = hits[j];
				String n = Text(h, "name");
				if(!n.IsEmpty()) cd.semantic.Add(n);
			}
		}
	}
}

static bool HasSemantic(const Candidate& c, const char* name)
{
	for(const String& s : c.semantic) if(s == name) return true;
	return false;
}

static void ComputeSignature(Candidate& c)
{
	if(c.sig_ok) return;
	String p = Resolve(c.crop_path);
	Image img = StreamRaster::LoadFileAny(p);
	if(img.IsEmpty()) return;
	c.signature = Signature(img);
	c.sig_width = img.GetWidth();
	c.sig_height = img.GetHeight();
	c.sig_ok = true;
}

// Load autoencoder cluster assignments keyed by crop basename.
static VectorMap<String, Value> LoadClusters(const String& path)
{
	VectorMap<String, Value> byname; // basename -> {cluster, features}
	if(path.IsEmpty() || !FileExists(path)) return byname;
	Value root = ParseJSON(LoadFile(path));
	if(IsError(root) || !IsValueMap(root)) return byname;
	ValueMap m = root;
	ValueArray samples = m.Get("samples", ValueArray());
	for(int i = 0; i < samples.GetCount(); i++) {
		ValueMap s = samples[i];
		String cp = Text(s, "crop_path");
		if(cp.IsEmpty()) continue;
		String base = PathKey3(cp);
		ValueMap v;
		if(s.Find("cluster") >= 0) v("cluster", s["cluster"]);
		if(s.Find("features") >= 0) v("features", s["features"]);
		byname.GetAdd(base) = v;
	}
	return byname;
}

// Build a dataset-manifest subset (only the given candidate indices) so the
// autoencoder tier processes ONLY unresolved candidates. Returns count written.
static int WriteRemainingDataset(const String& full_manifest_path,
                                 const Vector<Candidate>& cands,
                                 const Index<int>& remaining_indices,
                                 const String& out_path)
{
	if(!FileExists(full_manifest_path)) return 0;
	Value root = ParseJSON(LoadFile(full_manifest_path));
	if(IsError(root) || !IsValueMap(root)) return 0;
	ValueMap m = root;
	ValueArray samples = m.Get("samples", ValueArray());

	// map basename -> sample
	VectorMap<String, Value> by_base;
	for(int i = 0; i < samples.GetCount(); i++) {
		ValueMap s = samples[i];
		String cp;
		if(s.Find("crop_path") >= 0) cp = Text(s, "crop_path");
		else if(s.Find("image_variants") >= 0 && IsValueArray(s["image_variants"])) {
			ValueArray v = s["image_variants"];
			if(v.GetCount() > 0) cp = AsString(v[0]);
		}
		if(!cp.IsEmpty()) by_base.GetAdd(PathKey3(cp)) = samples[i];
	}

	ValueArray kept;
	for(int idx : remaining_indices) {
		String base = PathKey3(cands[idx].crop_path);
		int q = by_base.Find(base);
		if(q >= 0) kept.Add(by_base[q]);
	}

	ValueMap out;
	if(m.Find("schema") >= 0) out("schema", m["schema"]);
	if(m.Find("model") >= 0) out("model", m["model"]);
	out("samples", kept);
	RealizeDirectory(GetFileFolder(out_path));
	SaveFile(out_path, AsJSON(out, true));
	return kept.GetCount();
}

static void Help()
{
	Cout() << "VideoConfidenceTieredCandidates - Task 0261 tiered recognition pipeline\n"
	       << "Modes:\n"
	       << "  --emit-events            Structural tier only: read state source, write keypoint events.\n"
	       << "  (default)                Run full tiering over candidates.\n"
	       << "Options:\n"
	       << "  --candidates <path>      video_card_candidates.json\n"
	       << "  --table-state <path>     observer table_state.json (structural source)\n"
	       << "  --logic-jsonl <path>     native logic-state JSONL (alt structural source)\n"
	       << "  --review <path>          changed_region_review.json (template library source)\n"
	       << "  --dataset <path>         convnet dataset_manifest.json (for autoencoder routing)\n"
	       << "  --autoencoder-clusters <path>  precomputed autoencoder_clusters.json\n"
	       << "  --out-dir <dir>          output directory\n";
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	String candidates_path, table_state_path, logic_jsonl_path, review_path;
	String dataset_path, clusters_path, out_dir;
	bool emit_events = false, help = false;

	const Vector<String>& args = CommandLine();
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--candidates" && i + 1 < args.GetCount()) candidates_path = args[++i];
		else if(args[i] == "--table-state" && i + 1 < args.GetCount()) table_state_path = args[++i];
		else if(args[i] == "--logic-jsonl" && i + 1 < args.GetCount()) logic_jsonl_path = args[++i];
		else if(args[i] == "--review" && i + 1 < args.GetCount()) review_path = args[++i];
		else if(args[i] == "--dataset" && i + 1 < args.GetCount()) dataset_path = args[++i];
		else if(args[i] == "--autoencoder-clusters" && i + 1 < args.GetCount()) clusters_path = args[++i];
		else if(args[i] == "--out-dir" && i + 1 < args.GetCount()) out_dir = args[++i];
		else if(args[i] == "--emit-events") emit_events = true;
		else if(args[i] == "--help" || args[i] == "-h") help = true;
	}

	if(help) { Help(); return; }

	// ---- Structural extraction (always, if a source is given) ----
	StructuralExtractor ex;
	if(!table_state_path.IsEmpty()) FeedTableState(ex, table_state_path);
	if(!logic_jsonl_path.IsEmpty()) FeedLogicJsonl(ex, logic_jsonl_path);

	if(emit_events) {
		ValueArray evs;
		for(const KeypointEvent& e : ex.events) evs.Add(EventToValue(e));
		ValueMap out;
		out("event_count", ex.events.GetCount());
		out("events", evs);
		String path = out_dir.IsEmpty() ? String("keypoint_events.json")
		                                 : AppendFileName(out_dir, "keypoint_events.json");
		RealizeDirectory(GetFileFolder(path));
		SaveFile(path, AsJSON(out, true));
		Cout() << "Structural keypoint events: " << ex.events.GetCount() << "\n";
		for(const KeypointEvent& e : ex.events)
			Cout() << "  [" << e.event_type << "] table " << e.table_id
			       << " frame " << e.frame << " : " << e.description << "\n";
		Cout() << "Wrote " << path << "\n";
		return;
	}

	if(candidates_path.IsEmpty() || out_dir.IsEmpty()) {
		Cerr() << "ERROR: --candidates and --out-dir are required (unless --emit-events).\n";
		Help();
		SetExitCode(1);
		return;
	}
	RealizeDirectory(out_dir);

	Vector<Candidate> cands;
	LoadCandidates(candidates_path, cands);
	Cout() << "Loaded " << cands.GetCount() << " candidates.\n";

	// persist keypoint events alongside
	{
		ValueArray evs;
		for(const KeypointEvent& e : ex.events) evs.Add(EventToValue(e));
		ValueMap out; out("event_count", ex.events.GetCount()); out("events", evs);
		SaveFile(AppendFileName(out_dir, "keypoint_events.json"), AsJSON(out, true));
	}
	Cout() << "Structural keypoint events detected: " << ex.events.GetCount() << "\n";

	// ---- Tier 1: structural ----
	int structural_count = 0;
	for(Candidate& c : cands) {
		for(const KeypointEvent& e : ex.events) {
			bool board_ev = (e.event_type == "flop_dealt" || e.event_type == "turn_dealt"
			               || e.event_type == "river_dealt");
			if(board_ev && c.table_id == e.table_id
			   && (c.frame_index == e.frame || c.frame_index == e.frame - 1)
			   && HasSemantic(c, "board_cards")) {
				c.tier = "structural";
				c.label = Format("board_card street=%s count=%d", StreetName(e.street), e.to_count);
				c.confidence = 0.95;
				c.evidence("event_type", e.event_type);
				c.evidence("event_frame", e.frame);
				c.evidence("from_count", e.from_count);
				c.evidence("to_count", e.to_count);
				c.evidence("matched_region", "board_cards");
				structural_count++;
				break;
			}
			if(e.event_type == "dealer_rotation" && c.table_id == e.table_id
			   && (c.frame_index == e.frame || c.frame_index == e.frame - 1)
			   && HasSemantic(c, "bottom_button")) {
				c.tier = "structural";
				c.label = Format("dealer_button seat %d->%d", e.from_seat, e.to_seat);
				c.confidence = 0.95;
				c.evidence("event_type", e.event_type);
				c.evidence("event_frame", e.frame);
				c.evidence("from_seat", e.from_seat);
				c.evidence("to_seat", e.to_seat);
				structural_count++;
				break;
			}
		}
	}
	Cout() << "Tier 1 (structural) resolved: " << structural_count << "\n";

	// ---- Tier 2: template match (rgb8x8 exact against confirmed library, ----
	// ---- then a near-match fallback when exact match fails - Task 0267) ----
	int raw_groups = 0;
	VectorMap<String, LibEntry> lib = LoadLibrary(review_path, raw_groups);
	Cout() << "Template library: " << lib.GetCount() << " confirmed entries (from "
	       << raw_groups << " review groups).\n";
	int template_exact_count = 0, template_near_count = 0;
	for(Candidate& c : cands) {
		if(c.tier != "unresolved") continue;
		ComputeSignature(c);
		if(!c.sig_ok) continue;

		int q = lib.Find(c.signature);
		if(q >= 0) {
			const LibEntry& e = lib[q];
			c.tier = "template_match";
			c.label = e.label;
			c.confidence = 0.90;
			c.evidence("match_kind", "exact_rgb8x8");
			c.evidence("matched_signature", c.signature);
			c.evidence("matched_label", e.label);
			if(!e.rank.IsEmpty()) c.evidence("rank", e.rank);
			if(!e.suit.IsEmpty()) c.evidence("suit", e.suit);
			c.evidence("library_source", e.sample_path);
			c.evidence("score", 1.0);
			template_exact_count++;
			continue;
		}

		// Fallback (only reached when exact match failed): scan the library for
		// the closest same-dimension signature by rgb8x8 Manhattan distance and
		// accept it only if within NEAR_MATCH_MAX_DISTANCE (see the real-evidence
		// justification comment above SignatureDistance()). Different-dimension
		// library entries are skipped outright, never force-compared.
		int best_idx = -1, best_dist = RGB8X8_MAX_DISTANCE + 1;
		for(int i = 0; i < lib.GetCount(); i++) {
			const LibEntry& e = lib[i];
			if(e.width == 0 || e.height == 0) continue;
			if(e.width != c.sig_width || e.height != c.sig_height) continue;
			int d = SignatureDistance(c.signature, e.signature);
			if(d >= 0 && d < best_dist) { best_dist = d; best_idx = i; }
		}
		if(best_idx >= 0 && best_dist <= NEAR_MATCH_MAX_DISTANCE) {
			const LibEntry& e = lib[best_idx];
			c.tier = "template_match";
			c.label = e.label;
			c.confidence = 0.75; // lower than exact (0.90): fuzzy, not byte-identical
			c.evidence("match_kind", "near_rgb8x8");
			c.evidence("matched_signature", e.signature);
			c.evidence("matched_label", e.label);
			if(!e.rank.IsEmpty()) c.evidence("rank", e.rank);
			if(!e.suit.IsEmpty()) c.evidence("suit", e.suit);
			c.evidence("library_source", e.sample_path);
			c.evidence("distance", best_dist);
			c.evidence("max_distance", RGB8X8_MAX_DISTANCE);
			c.evidence("score", 1.0 - (double)best_dist / RGB8X8_MAX_DISTANCE);
			template_near_count++;
		}
	}
	int template_count = template_exact_count + template_near_count;
	Cout() << "Tier 2 (template_match) resolved: " << template_count
	       << " (exact_rgb8x8=" << template_exact_count
	       << " near_rgb8x8=" << template_near_count << ")\n";

	// ---- Route remaining to autoencoder tier ----
	Index<int> remaining;
	for(Candidate& c : cands) if(c.tier == "unresolved") remaining.Add(c.index);
	int subset = WriteRemainingDataset(dataset_path, cands, remaining,
	                                   AppendFileName(out_dir, "remaining_dataset.json"));
	Cout() << "Routed " << remaining.GetCount() << " unresolved candidate(s) to autoencoder tier"
	       << " (remaining_dataset.json samples: " << subset << ").\n";

	// ---- Tier 3: autoencoder clusters (if provided) ----
	int autoencoder_count = 0, unresolved_count = 0;
	VectorMap<String, Value> clusters = LoadClusters(clusters_path);
	for(Candidate& c : cands) {
		if(c.tier != "unresolved") continue;
		String base = PathKey3(c.crop_path);
		int q = clusters.Find(base);
		if(q >= 0) {
			ValueMap cv = clusters[q];
			c.tier = "autoencoder_cluster";
			int cid = cv.Find("cluster") >= 0 ? (int)cv["cluster"] : -1;
			c.label = Format("cluster_%d", cid);
			c.confidence = 0.30;
			c.evidence("cluster_id", cid);
			if(cv.Find("features") >= 0) c.evidence("features", cv["features"]);
			autoencoder_count++;
		}
		else {
			c.evidence("reason", clusters_path.IsEmpty()
			           ? "no_autoencoder_clusters_provided" : "not_present_in_clusters");
			unresolved_count++;
		}
	}
	Cout() << "Tier 3 (autoencoder_cluster) resolved: " << autoencoder_count << "\n";
	Cout() << "Unresolved: " << unresolved_count << "\n";

	// ---- Combined output ----
	ValueArray out_arr;
	VectorMap<String, int> tier_counts;
	for(const Candidate& c : cands) {
		ValueMap m;
		m("candidate_index", c.index);
		m("frame_index", c.frame_index);
		m("table_id", c.table_id);
		m("candidate_type", c.candidate_type);
		m("rect", c.rect);
		m("crop_path", c.crop_path);
		ValueArray sem; for(const String& s : c.semantic) sem.Add(s);
		m("semantic_regions", sem);
		m("resolved_tier", c.tier);
		m("label", c.label);
		m("confidence", c.confidence);
		m("evidence", c.evidence);
		out_arr.Add(m);
		tier_counts.GetAdd(c.tier, 0)++;
	}
	ValueMap summary;
	summary("structural", tier_counts.Get("structural", 0));
	summary("template_match", tier_counts.Get("template_match", 0));
	summary("template_match_exact_rgb8x8", template_exact_count);
	summary("template_match_near_rgb8x8", template_near_count);
	summary("autoencoder_cluster", tier_counts.Get("autoencoder_cluster", 0));
	summary("unresolved", tier_counts.Get("unresolved", 0));

	ValueMap root_out;
	root_out("candidate_count", cands.GetCount());
	root_out("tier_summary", summary);
	root_out("keypoint_event_count", ex.events.GetCount());
	root_out("candidates", out_arr);

	String out_json = AppendFileName(out_dir, "candidates_tiered.json");
	SaveFile(out_json, AsJSON(root_out, true));
	Cout() << "Wrote " << out_json << "\n";
	Cout() << "Tier summary: structural=" << tier_counts.Get("structural", 0)
	       << " template_match=" << tier_counts.Get("template_match", 0)
	       << " autoencoder_cluster=" << tier_counts.Get("autoencoder_cluster", 0)
	       << " unresolved=" << tier_counts.Get("unresolved", 0) << "\n";
}
