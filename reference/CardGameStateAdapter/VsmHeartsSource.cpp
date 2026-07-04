#include "CardGameStateAdapter.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Small PyValue attribute helpers, local to this file.
//
// Deliberately NOT reusing VsmCardGameStateExport.cpp's file-local GetAttr()/
// GetStr()/GetInt()/GetBool() (those stay file-local there) -- this is a
// handful of one-line wrappers around the *public* PyValue::GetItem()/
// GetType() API, not the significant lookup pattern the 0069 plan says to
// reuse rather than re-derive (that pattern is FindEntryModuleDict(), now
// exposed as VsmCardGameStateExport::FindEntryModuleDict() and used below).

static PyValue HGetAttr(const PyValue& obj, const char* name)
{
	if(obj.GetType() != PY_DICT)
		return PyValue::None();
	PyValue v = obj.GetItem(PyValue(String(name)));
	if(!v.IsNone())
		return v;
	PyValue cls = obj.GetItem(PyValue("__class__"));
	if(cls.GetType() == PY_DICT)
		return cls.GetItem(PyValue(String(name)));
	return PyValue::None();
}

static int HGetInt(const PyValue& obj, const char* name)
{
	PyValue v = HGetAttr(obj, name);
	return v.IsNone() ? 0 : v.AsInt();
}

static String HGetStr(const PyValue& obj, const char* name)
{
	PyValue v = HGetAttr(obj, name);
	return v.IsNone() ? String() : v.ToString();
}

static bool HGetBool(const PyValue& obj, const char* name)
{
	PyValue v = HGetAttr(obj, name);
	return !v.IsNone() && v.IsTrue();
}

// hearts/logic.py Card.id is "<suit>_<rank>" (e.g. "spades_queen"). The
// schema (CARD_GAME_STATE_SCHEMA.md) wants "<rank><suit>" as a two-character
// code (e.g. "QS"). Converts between the two; used nowhere else in the
// existing 0068 adapter (its main.cpp demo passes literal codes), so this is
// new, driver-local logic, not a duplicate of anything upstream.
static String HCardIdToCode(const String& card_id)
{
	int us = card_id.Find('_');
	if(us < 0)
		return card_id;
	String suit = card_id.Left(us);
	String rank = card_id.Mid(us + 1);

	String suit_c = "?";
	if(suit == "clubs")    suit_c = "C";
	if(suit == "diamonds") suit_c = "D";
	if(suit == "hearts")   suit_c = "H";
	if(suit == "spades")   suit_c = "S";

	String rank_c = rank;
	if(rank == "10")    rank_c = "T";
	if(rank == "jack")  rank_c = "J";
	if(rank == "queen") rank_c = "Q";
	if(rank == "king")  rank_c = "K";
	if(rank == "ace")   rank_c = "A";

	return rank_c + suit_c;
}

const int VsmHeartsSource::kMaxSteps; // odr-used (passed by reference into Format()) -- needs a definition

VsmHeartsSource::VsmHeartsSource() {}
VsmHeartsSource::~VsmHeartsSource() {}

PyValue VsmHeartsSource::ModuleDict()
{
	PyVM* vm = host.GetVM();
	if(!vm)
		return PyValue::None();
	return VsmCardGameStateExport::FindEntryModuleDict(host, *vm);
}

bool VsmHeartsSource::GetModuleBool(PyValue& mod, const char* name)
{
	if(mod.GetType() != PY_DICT)
		return false;
	PyValue v = mod.GetItem(PyValue(String(name)));
	return !v.IsNone() && v.IsTrue();
}

bool VsmHeartsSource::CallModuleFunc(PyValue& mod, const char* name)
{
	PyVM* vm = host.GetVM();
	if(!vm) {
		last_error = "VsmHeartsSource: host has no PyVM";
		return false;
	}
	if(mod.GetType() != PY_DICT) {
		last_error = String("VsmHeartsSource: no live module dict (calling ") + name + ")";
		return false;
	}
	PyValue fn = mod.GetItem(PyValue(String(name)));
	if(fn.IsNone() || !fn.IsFunction()) {
		last_error = String("VsmHeartsSource: module function not found: ") + name;
		return false;
	}
	vm->Call(fn, Vector<PyValue>());
	return true;
}

// ---------------------------------------------------------------------------
// VsmFrameSource

bool VsmHeartsSource::Open(const String& uri_)
{
	uri = uri_;
	last_error.Clear();

	if(!FileExists(uri)) {
		last_error = "VsmHeartsSource::Open: file not found: " + uri;
		return false;
	}

	host.SetFixedArea(Size(1024, 768));
	if(!host.Load(uri)) {
		last_error = "VsmHeartsSource::Open: CardGameDocumentHost::Load failed for " + uri;
		return false;
	}
	host.ExecuteSync();

	Size sz = host.GetRecordFrameSize();
	width  = sz.cx;
	height = sz.cy;

	// Gotcha #1 (0069 plan, verified against main.py + PyVM::Call()/PY_MAKE_FUNCTION):
	// force full AI-vs-AI play. main.py's ai_step() gates player 0's own play on
	// `autoplay_enabled` (main.py:880-881), which start() only sets from
	// "--autoplay" in sys.argv (main.py:145) -- never true here, since
	// CardGamePlugin never populates sys.argv. Every module-level function
	// (ai_step, finish_trick_collect, finish_pass_animation, next_round) was
	// defined while executing main.py's own top-level code, so PY_MAKE_FUNCTION
	// (PyVM.cpp) captured `l2->globals = frame.globals` == this same mod_dict;
	// PyVM::Call() later restores `f.globals = l.globals` for that call, so a
	// LOAD_GLOBAL of "autoplay_enabled" inside ai_step() reads whatever we set
	// here, on the very next call -- confirmed by reading PY_MAKE_FUNCTION and
	// PyVM::Call() in full, not assumed.
	PyValue mod = ModuleDict();
	if(mod.GetType() != PY_DICT) {
		last_error = "VsmHeartsSource::Open: no live module dict after ExecuteSync() -- "
		             "has main.py's start() run?";
		return false;
	}
	mod.SetItem(PyValue("autoplay_enabled"), PyValue(true));
	PyValue check = mod.GetItem(PyValue("autoplay_enabled"));
	if(!check.IsTrue()) {
		last_error = "VsmHeartsSource::Open: forcing autoplay_enabled=True did not take effect";
		return false;
	}

	PyValue state = mod.GetItem(PyValue("state"));
	if(state.GetType() != PY_DICT) {
		last_error = "VsmHeartsSource::Open: no live GameState 'state' object";
		return false;
	}

	step_count      = 0;
	tricks_resolved = 0;
	last_step_events.Clear();
	ready = true;
	return true;
}

void VsmHeartsSource::Close()
{
	ready = false;
}

bool VsmHeartsSource::ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms)
{
	if(!ready) {
		last_error = "VsmHeartsSource::ReadFrame: not ready";
		return false;
	}
	Image img = host.CaptureRecordFrame();
	int w = img.GetWidth(), h = img.GetHeight();
	if(w <= 0 || h <= 0) {
		last_error = "VsmHeartsSource::ReadFrame: CaptureRecordFrame() returned an empty image";
		return false;
	}

	// Same RGB conversion pattern as reference/VisualStateWorkbench/
	// JpegSequenceImporter.cpp's ImageToVsmBuffer() (grayscale=false branch) --
	// reused rather than re-derived.
	out_frame.Create(w, h, 3);
	for(int y = 0; y < h; y++) {
		const RGBA* row = img[y];
		for(int x = 0; x < w; x++) {
			out_frame.Set(x, y, row[x].r, 0);
			out_frame.Set(x, y, row[x].g, 1);
			out_frame.Set(x, y, row[x].b, 2);
		}
	}
	out_ts_ms = (int64)step_count * 33; // no real wall-clock timeline under manual stepping
	return true;
}

String VsmHeartsSource::GetSourceInfo() const
{
	return "VsmHeartsSource:" + uri;
}

// ---------------------------------------------------------------------------
// VsmSteppedFrameSource

bool VsmHeartsSource::HasMoreSteps() const
{
	if(!ready)
		return false;
	// Per 0069's requirement 3: false once state.game_over is true. Note this
	// does NOT by itself stop driving at round end (ROUND_END without
	// game_over==true keeps HasMoreSteps() true, since begin_next_round()
	// would be a valid next Step() in a hypothetical multi-round driver) --
	// the one-round acceptance scenario stops itself by watching for a
	// "round" tier event instead, and the kMaxSteps cap is the hard backstop.
	PyValue mod = const_cast<VsmHeartsSource*>(this)->ModuleDict();
	PyValue state = mod.GetItem(PyValue("state"));
	return !HGetBool(state, "game_over");
}

bool VsmHeartsSource::Step()
{
	last_step_events.Clear();

	if(!ready) {
		last_error = "VsmHeartsSource::Step: called before a successful Open()";
		return false;
	}
	if(step_count >= kMaxSteps) {
		last_error = Format(
			"VsmHeartsSource::Step: exceeded max step cap (%d) without reaching round end "
			"-- treating this as a stuck driver, not looping forever", kMaxSteps);
		return false;
	}
	step_count++;

	PyValue mod = ModuleDict();
	if(mod.GetType() != PY_DICT) {
		last_error = "VsmHeartsSource::Step: no live module dict";
		return false;
	}
	PyValue state = mod.GetItem(PyValue("state"));
	if(state.GetType() != PY_DICT) {
		last_error = "VsmHeartsSource::Step: no live GameState 'state' object";
		return false;
	}

	bool was_playing = HGetStr(state, "phase") == "PLAYING";
	PyValue trick_before = HGetAttr(state, "trick");
	int trick_len_before = trick_before.GetType() == PY_LIST ? trick_before.GetCount() : 0;

	// The one-step unit: exactly one ai_step() call, the driver-side stand-in
	// for the timer-driven schedule_ai_step() chain (0069 plan's core point:
	// no new Python is needed, this *is* the step mechanism).
	if(!CallModuleFunc(mod, "ai_step"))
		return false;

	// Gotcha #1 side effect already active (autoplay_enabled forced in Open());
	// re-fetch state (mod_dict entries are stable PyValue dict references, so
	// this is defensive, not strictly required).
	state = mod.GetItem(PyValue("state"));

	// Gotcha (pass path): start_pass_animation() runs *synchronously* inside
	// ai_step() itself (main.py:867) once the 4th player's pass completes the
	// exchange; only the matching finish_pass_animation() is deferred behind
	// set_timeout(). Finish it now so the round doesn't stall waiting for a
	// timer that will never fire under manual stepping.
	if(GetModuleBool(mod, "pass_animating")) {
		if(!CallModuleFunc(mod, "finish_pass_animation"))
			return false;
		state = mod.GetItem(PyValue("state"));
	}

	// Was a card actually played by this ai_step() call? Only meaningful once
	// past the passing phase (before that, ai_step() drives select_pass()
	// calls, which have no card_play/trick/round tier of their own in the
	// schema -- this Step() still counts and still advances the round, it
	// just emits no state_json, which callers must tolerate per GetLastStateJson()'s
	// doc comment).
	PyValue trick_after = HGetAttr(state, "trick");
	int trick_len_after = trick_after.GetType() == PY_LIST ? trick_after.GetCount() : 0;
	bool trick_pending_after = HGetBool(state, "trick_pending");

	if(was_playing && trick_len_after > trick_len_before) {
		// The just-played card is the last (player_index, Card) tuple in
		// state.trick -- PyValue::ToValue() does not recurse through
		// PY_TUPLE (0068's adapter doc notes this), so read it manually via
		// GetItem(0)/GetItem(1), exactly as that doc predicted a future tier
		// would need to.
		PyValue last_play = trick_after.GetItem(trick_len_after - 1);
		int played_by = last_play.GetItem(0).AsInt();
		PyValue card  = last_play.GetItem(1);
		String card_code = HCardIdToCode(HGetStr(card, "id"));
		String json = exporter.ExportCardPlayState(host, played_by, card_code);
		last_step_events.Add(json);
	}

	// Gotcha #2 (trick path): when the 4th card of a trick is played,
	// state.trick_pending becomes true but resolve_trick() has NOT run yet --
	// that only happens once start_trick_collect() (called on the *next*
	// ai_step() entry, main.py:840-843) has scheduled, and finish_trick_collect()
	// (main.py:793-814) actually fires. Both are timer-gated under normal
	// play; detect "still trick_pending after this ai_step() call" and drive
	// finish_trick_collect() ourselves, in this same logical Step().
	if(trick_pending_after) {
		if(!CallModuleFunc(mod, "finish_trick_collect"))
			return false;
		state = mod.GetItem(PyValue("state"));

		tricks_resolved++; // exact -- we drove this resolution ourselves, unlike
		                    // VsmCardGameStateExport's own best-effort
		                    // last_trick_winner-diff heuristic (which undercounts
		                    // on same-winner-twice-in-a-row; not a limitation here).
		int winner = HGetInt(state, "last_trick_winner");
		int points = HGetInt(state, "last_trick_points");
		String trick_json = exporter.ExportTrickState(host, tricks_resolved, winner, points);
		last_step_events.Add(trick_json);

		// finish_trick_collect() already called state.resolve_trick(), which
		// (hearts/logic.py:251-252) calls resolve_round() synchronously in
		// Python whenever all hands are empty -- so if this was the round's
		// 13th trick, `state.phase` is ROUND_END/GAME_OVER and every Tier-3
		// field is already populated *right now*, with no need to also drive
		// next_round() (that begins dealing round 2, out of this task's
		// one-round scope; finish_trick_collect()'s own set_timeout(...,
		// "next_round") is deliberately left unfired).
		String phase = HGetStr(state, "phase");
		if(phase == "ROUND_END" || phase == "GAME_OVER") {
			int round_number = HGetInt(state, "round_number");
			String round_json = exporter.ExportRoundState(host, round_number);
			last_step_events.Add(round_json);
		}
	}

	return true;
}

END_UPP_NAMESPACE
