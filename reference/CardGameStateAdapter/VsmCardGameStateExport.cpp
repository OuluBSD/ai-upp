#include "CardGameStateAdapter.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Locating the live `state` GameState instance.
//
// The task plan this adapter was built from (plan/VisualStateModel/0068_...)
// suggested `host.GetVM()->GetGlobals().GetItem(PyValue("state"))`. That does
// NOT work here: CardGamePlugin::Execute() (uppsrc/ScriptCommon/CardGamePlugin.cpp)
// loads the .gamestate's entry script (main.py) as its own module via
// PyVM::LoadModule(), which executes it with its OWN module dict as globals
// (PyVM.cpp: `f.func.GetLambdaRW().globals = mod_dict; f.globals = mod_dict;`).
// Only *after* that module-level execution finishes does CardGamePlugin copy
// that module dict's entries into PyVM::GetGlobals() -- once, as a snapshot,
// key by key (CardGamePlugin.cpp: `globals.GetAdd(items.GetKey(i)) = items[i];`)
// -- and only THEN does it call the entry_function ("start"). main.py's
// top-level code only sets `state = None`; the real `state = GameState()`
// assignment happens inside start(), which runs against main.py's own module
// dict (mod_dict), not PyVM::GetGlobals(). Because PyValue's copy of a scalar
// None is an independent value (not a shared reference the way a dict/list
// PyValue would be), PyVM::GetGlobals()["state"] keeps referring to that
// stale None forever afterwards. Confirmed by reading PyVM::LoadModule() and
// CardGamePlugin::Execute() in full, not assumed.
//
// The live, correct `state` lives in `sys.modules[<entry module name>]`,
// which is the same mod_dict object start() actually mutates. This function
// re-derives <entry module name> the same way CardGamePlugin::Execute() does
// (GetFileTitle() of the .gamestate's "entry_script" field), using
// CardGameDocumentHost::GetPath() (public) to find the .gamestate file again
// -- CardGameDocumentHost/CardGamePlugin do not expose entry_module_name
// directly (it is `protected` on CardGamePlugin, and CardGameDocumentHost's
// `plugin`/`runtime_plugin` members are private), so re-deriving from the
// public .gamestate contents is the non-invasive way to get it without
// modifying CardGameDocumentHost for something that is not actually missing
// -- the data needed is already public, just not pre-computed for us.
static PyValue FindEntryModuleDict(CardGameDocumentHost& host, PyVM& vm)
{
	String gamestate_path = host.GetPath();
	String module_name = "__game_main__";
	if(!gamestate_path.IsEmpty() && FileExists(gamestate_path)) {
		Value gs = ParseJSON(LoadFile(gamestate_path));
		String entry_script = gs["entry_script"];
		if(!entry_script.IsEmpty()) {
			String title = GetFileTitle(entry_script);
			if(!title.IsEmpty())
				module_name = title;
		}
	}

	PyValue sys = vm.GetGlobals().GetItem(PyValue("sys"));
	if(sys.GetType() != PY_DICT)
		return PyValue::None();
	PyValue modules = sys.GetItem(PyValue("modules"));
	if(modules.GetType() != PY_DICT)
		return PyValue::None();
	return modules.GetItem(PyValue(module_name));
}

// Instance-attribute lookup mirroring PyVM's own PY_LOAD_ATTR handling for
// dict-backed class instances (PyVM.cpp, PY_LOAD_ATTR case): check the
// instance dict first, then fall back to the class dict (for class-level
// defaults/methods). PyValue itself has no public GetAttr() -- that name only
// exists as a virtual on PyUserData (PyValue.h:204), which plain Python class
// instances here do not use (they are PY_DICT with a "__class__" entry, per
// PyVM.cpp's class-instantiation code around PY_CALL). GameState always
// initializes every field this adapter reads directly in __init__, so plain
// GetItem() would already suffice for GameState specifically, but this
// mirrors the VM's real semantics rather than assuming that.
static PyValue GetAttr(const PyValue& obj, const char* name)
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

static String GetStr(const PyValue& obj, const char* name)
{
	PyValue v = GetAttr(obj, name);
	return v.IsNone() ? String() : v.ToString();
}

static int GetInt(const PyValue& obj, const char* name)
{
	PyValue v = GetAttr(obj, name);
	return v.IsNone() ? 0 : v.AsInt();
}

static bool GetBool(const PyValue& obj, const char* name)
{
	PyValue v = GetAttr(obj, name);
	return !v.IsNone() && v.IsTrue();
}

// list[int] -> Value (ValueArray). PyValue::ToValue() (PyValue.cpp) recurses
// correctly for PY_LIST (each element re-dispatched through ToValue()), so
// for a flat list[int] like round_scores/scores this already produces a
// ValueArray of plain ints directly -- verified by reading ToValue()'s
// switch, not assumed. (It does NOT recurse correctly through PY_TUPLE,
// which falls into ToValue()'s default case as an empty Value -- irrelevant
// here since none of this schema's fields are tuples, but noted because
// `state.trick` is a list of (player_index, card) tuples and would need a
// manual per-tuple GetItem(0)/GetItem(1) loop if a future tier ever needed it.)
static Value GetIntList(const PyValue& obj, const char* name)
{
	PyValue v = GetAttr(obj, name);
	if(v.GetType() != PY_LIST)
		return ValueArray();
	return v.ToValue();
}

void VsmCardGameStateExport::TrackTrickNumber(const PyValue& state)
{
	PyValue winner_pv = GetAttr(state, "last_trick_winner");
	int winner = winner_pv.IsNone() ? -1 : winner_pv.AsInt();
	if(winner != -1 && winner != last_seen_trick_winner)
		resolved_trick_count++;
	last_seen_trick_winner = winner;
}

String VsmCardGameStateExport::ExportCardPlayState(CardGameDocumentHost& host, int player, const String& card_played)
{
	PyVM* vm = host.GetVM();
	ASSERT_(vm, "VsmCardGameStateExport::ExportCardPlayState: host has no PyVM");
	PyValue state = FindEntryModuleDict(host, *vm).GetItem(PyValue("state"));
	ASSERT_(state.GetType() == PY_DICT, "VsmCardGameStateExport::ExportCardPlayState: "
	        "no live GameState 'state' object found -- has ExecuteSync() been run yet?");

	TrackTrickNumber(state);

	ValueMap v;
	v.Add("tier", "card_play");
	v.Add("round_number", GetInt(state, "round_number"));
	v.Add("phase", GetStr(state, "phase"));
	v.Add("turn", GetInt(state, "turn"));
	v.Add("trick_number", resolved_trick_count + 1);
	v.Add("leading_suit", GetStr(state, "leading_suit"));
	v.Add("hearts_broken", GetBool(state, "hearts_broken"));
	v.Add("player", player);
	v.Add("card_played", card_played);
	return AsJSON(v);
}

String VsmCardGameStateExport::ExportTrickState(CardGameDocumentHost& host, int trick_number, int trick_winner, int trick_points)
{
	PyVM* vm = host.GetVM();
	ASSERT_(vm, "VsmCardGameStateExport::ExportTrickState: host has no PyVM");
	PyValue state = FindEntryModuleDict(host, *vm).GetItem(PyValue("state"));
	ASSERT_(state.GetType() == PY_DICT, "VsmCardGameStateExport::ExportTrickState: "
	        "no live GameState 'state' object found -- has ExecuteSync() been run yet?");

	ValueMap v;
	v.Add("tier", "trick");
	v.Add("round_number", GetInt(state, "round_number"));
	v.Add("trick_number", trick_number);
	v.Add("trick_winner", trick_winner);
	v.Add("trick_points", trick_points);
	v.Add("round_scores", GetIntList(state, "round_scores"));
	return AsJSON(v);
}

String VsmCardGameStateExport::ExportRoundState(CardGameDocumentHost& host, int round_number)
{
	PyVM* vm = host.GetVM();
	ASSERT_(vm, "VsmCardGameStateExport::ExportRoundState: host has no PyVM");
	PyValue state = FindEntryModuleDict(host, *vm).GetItem(PyValue("state"));
	ASSERT_(state.GetType() == PY_DICT, "VsmCardGameStateExport::ExportRoundState: "
	        "no live GameState 'state' object found -- has ExecuteSync() been run yet?");

	ValueMap v;
	v.Add("tier", "round");
	v.Add("round_number", round_number);
	v.Add("round_scores", GetIntList(state, "last_round_scores"));
	v.Add("scores", GetIntList(state, "scores"));
	v.Add("moon_shooter", GetInt(state, "last_round_moon_shooter"));
	v.Add("game_over", GetBool(state, "game_over"));
	return AsJSON(v);
}

END_UPP_NAMESPACE
