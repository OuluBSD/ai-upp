#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// M04-03 (task 0114) role classification tables.
//
// Every mapping below was checked against BOTH `game/TexasHoldem/
// GameTable_PS_6p.form` and `game/TexasHoldem/GameTable.form` — both declare
// the exact same `Variable`/`Type` vocabulary (confirmed already by task
// 0112's evidence: same 38-element name/type/variable set, only rects
// differ), so one classification table serves both files. `file:line`
// citations below are given against `GameTable_PS_6p.form` as the canonical
// reference; the same `Variable` values appear at different line numbers in
// `GameTable.form` (structurally identical `<properties>` blocks, just a
// different rect).

// ---------------------------------------------------------------------------
// Variable-based overrides, tried FIRST (before any Type-based rule below).
// These exist because the following `Type`s are ambiguous — MULTIPLE
// elements share the same `Type` but mean different things:
//   - "Label": PotTitle/PotTotal/PotBets/TurnTitle/GameInfo/HandInfo/SpeedLabel
//     (7 Label-typed elements, 6 distinct meanings — PotTitle+PotTotal share
//     one, "pot_amount").
//   - "SliderCtrl": SliderBet (a human bet-amount input) vs. SpeedSlider (a
//     playback-speed control) — same Type, opposite intent.
//   - "StaticRect"/"ImageCtrl": HandsPane/HandsImage are the only instances
//     of each Type in these forms, but the Type itself (a generic decorative
//     rect / a generic image) doesn't imply "this is the hands pane" — only
//     the Variable does, so it's classified the same way as the genuinely
//     ambiguous ones for consistency (a future form could reuse StaticRect
//     for an unrelated background).
//
// Per task 0114's guardrail, classification keys on `Variable`, NOT the
// `<name>` tag (e.g. "lblPotTitle", not "PotTitle") — Variable is the more
// stable "intent" signal a differently-shaped future `.form` is more likely
// to preserve.
static String VsmClassifyByVariable(const String& variable)
{
	// GameTable_PS_6p.form:98 <property name="Variable" value="lblPotTitle"/>
	// (PotTitle = static "Pot" caption) and
	// GameTable_PS_6p.form:109 (PotTotal = the pot's numeric value label) —
	// both grouped under one role since they jointly render "the pot
	// amount" as one visual unit (caption + value), and the vocabulary has
	// no separate "caption vs. value" distinction for either pot or turn.
	if(variable == "lblPotTitle" || variable == "lblPotTotal")
		return "pot_amount";
	// GameTable_PS_6p.form:120 (PotBets = bets-into-pot-this-round label).
	if(variable == "lblPotBets")
		return "pot_bets";
	// GameTable_PS_6p.form:131 (TurnTitle = current street name, e.g.
	// "Preflop" — this one is itself the content, not just a caption for a
	// separate value label, so it maps directly to "turn_label").
	if(variable == "lblTurnTitle")
		return "turn_label";
	// GameTable_PS_6p.form:142 (GameInfo = free-text game-info label).
	if(variable == "lblGameInfo")
		return "game_info";
	// GameTable_PS_6p.form:153 (HandInfo = free-text hand-info label).
	if(variable == "lblHandInfo")
		return "hand_info";
	// GameTable_PS_6p.form:276 (SpeedLabel, "Speed: 4" caption) and :287
	// (SpeedSlider, the actual playback-speed SliderCtrl) — grouped under
	// one role since both are the same on-screen control (label + slider),
	// and the vocabulary has no separate "speed_label" role.
	if(variable == "lblSpeed" || variable == "sliderSpeed")
		return "speed_control";
	// GameTable_PS_6p.form:207 (SliderBet = the human bet-amount slider,
	// paired with EditBet's EditInt box — both are "the human enters a bet
	// amount here" controls).
	if(variable == "sliderBet")
		return "human_input";
	// GameTable_PS_6p.form:312 (HandsPane, the StaticRect background) and
	// :320 (HandsImage, the ImageCtrl child showing the actual hands image)
	// — grouped under one role since they're the same on-screen region
	// (container + its content).
	if(variable == "handsImgPane" || variable == "handsImg")
		return "hands_pane";
	// M05-06 (task 0124): the merged name+action control on the synthetic
	// "PlayerCtrlMerged" seat type (upptst/VisualStateModelTests/testdata/
	// GameTable_MergedNameAction.form) — a real, `.form`-declared, Parent-
	// nested `Label` element (Type alone is ambiguous, same reason every
	// other Label-typed variable above is keyed by Variable, not Type), keyed
	// off a `Variable` PREFIX (StartsWith, not exact-match, since this must
	// work for "nameaction0"/"nameaction1"/"nameaction2"/... any seat index —
	// mirrors VsmParseSeatIndex's own prefix-based approach below) rather than
	// a fixed exact string, so this rule generalizes to any seat count.
	if(variable.StartsWith("nameaction"))
		return "name_action";
	// M05-06 (task 0124): the stack/chip-count sibling child on the same
	// synthetic fixture — reuses the EXISTING "stack_text" role (same meaning
	// as real PlayerCtrl's `textLabel_Cash` sub-slot: a per-seat chip-count
	// display), not a new near-duplicate role, since the meaning is identical
	// even though this fixture reaches it via a top-level `.form` element
	// rather than a `VsmFormSubSlot` table entry.
	if(variable.StartsWith("stacktext"))
		return "stack_text";
	return String();
}

// Type-based dispatch, tried only if VsmClassifyByVariable() above found no
// match (i.e. for every Type that maps 1:1 to a role in these forms).
String VsmClassifyElementRole(const VsmFormElement& element)
{
	String by_var = VsmClassifyByVariable(element.variable);
	if(!by_var.IsEmpty())
		return by_var;

	// GameTable_PS_6p.form:20 (Player0..9, Type="PlayerCtrl") — one whole
	// seat block. Seat index itself is derived separately, from `variable`
	// (see VsmParseSeatIndex), not here.
	if(element.type == "PlayerCtrl")
		return "seat";
	// M05-06 (task 0124): "PlayerCtrlMerged" — the synthetic, test-only seat
	// type used by upptst/VisualStateModelTests/testdata/
	// GameTable_MergedNameAction.form (a hypothetical platform where the
	// player-name and action-text controls are merged into one). It IS a
	// seat, just a differently-composed one than real PlayerCtrl — same role,
	// seat_index parsed the same `VsmParseSeatIndex` way below (confirmed
	// unchanged: that parser keys off the `Variable` "player<N>" pattern
	// only, never `Type`). Added as its own rule rather than folded into the
	// "PlayerCtrl" check above, per this file's "add rows, don't rewrite
	// dispatch logic" guardrail.
	if(element.type == "PlayerCtrlMerged")
		return "seat";
	// GameTable_PS_6p.form:11 (Board, Type="BoardCtrl") — whole board area.
	if(element.type == "BoardCtrl")
		return "board";
	// GameTable_PS_6p.form:190 etc. (BtnAllIn/BtnPot33/BtnPot50/BtnPot100/
	// BtnBetRaise/BtnCheckCall/BtnFold, Type="ActionButton") — all 7
	// instances are human action buttons, Type alone is unambiguous here.
	if(element.type == "ActionButton")
		return "action_button";
	// GameTable_PS_6p.form:200 (EditBet, Type="EditInt") — the bet-amount
	// number entry box, paired with SliderBet above (both "human_input").
	if(element.type == "EditInt")
		return "human_input";
	// GameTable_PS_6p.form:166/174 (TabLeft/TabRight, Type="TabCtrl") — tab
	// containers hosting the chat/hands/probability/log sub-panes below.
	// There's no more specific listed role for "a tab container that hosts
	// status panes", so this maps to the closest vocabulary entry,
	// "status_pane".
	if(element.type == "TabCtrl")
		return "status_pane";
	// GameTable_PS_6p.form:297 (PauseButton, Type="Button") — the only
	// "Button"-typed element in either form; Type alone is unambiguous here
	// (no other Button-typed element exists to confuse it with).
	if(element.type == "Button")
		return "pause_button";
	// GameTable_PS_6p.form:306 (ChatCtrl, Type="PokerChatCtrl") — only
	// instance of this Type, unambiguous.
	if(element.type == "PokerChatCtrl")
		return "chat_pane";
	// GameTable_PS_6p.form:331 (Probabilities, Type="ProbabilityCtrl") —
	// only instance of this Type, unambiguous.
	if(element.type == "ProbabilityCtrl")
		return "probability_pane";
	// GameTable_PS_6p.form:339/347 (EngineLog/AwayLog, Type="RichTextView")
	// — both instances are log/history views; Type alone is unambiguous
	// (both map to the same role, "log_pane" — the vocabulary has no
	// separate "engine log" vs. "away log" distinction, and none is needed
	// for 0115's region-matching purpose).
	if(element.type == "RichTextView")
		return "log_pane";
	// GameTable_PS_6p.form:182 (HumanButtons, Type="ParentCtrl") —
	// DELIBERATE "unknown": this element is purely a layout container (its
	// own rect has no on-screen content — the action buttons/slider/edit box
	// it parents are the actual meaningful sub-elements, each separately
	// classified above via their own `Parent`-nested `<item>`s). Explicitly
	// called out here (per task 0114's own example of "purely decorative
	// containers... if you judge that reasonable") rather than inventing a
	// role that doesn't fit.
	if(element.type == "ParentCtrl")
		return "unknown";

	// No rule matched (a future `.form`'s Type/Variable this table doesn't
	// yet know about) — "unknown" rather than a guess, per the "agnostic to
	// an unrecognized vocabulary" direction from 0112/0113.
	return "unknown";
}

// ---------------------------------------------------------------------------
// Sub-slot role classification. Sub-slot names come from VsmGetSubSlots()
// (FormLayout.cpp) — this table must stay in sync with the names declared
// there (verified: every name below matches a name added in that file for
// task 0113/0114).
String VsmClassifySubSlotRole(const String& owner_type, const String& slot_name)
{
	if(owner_type == "PlayerCtrl") {
		// hole_card_0/1: pixmapLabel_carda/cardb, GameTable.cpp:332/333
		// (task 0113).
		if(slot_name == "hole_card_0" || slot_name == "hole_card_1")
			return "hole_card";
		// button_puck: textLabel_Button, GameTable.cpp:336 (task 0113).
		if(slot_name == "button_puck")
			return "dealer_button";
		// avatar: label_Avatar, GameTable.cpp:328 (task 0114 — see Context
		// section / FormLayout.cpp for the added sub-slot row).
		if(slot_name == "avatar")
			return "avatar";
		// player_name: label_PlayerName, GameTable.cpp:329 (task 0114) —
		// explicitly named in the milestone's deliverable list.
		if(slot_name == "player_name")
			return "player_name";
		// stack_text: textLabel_Cash, GameTable.cpp:330 (task 0114).
		if(slot_name == "stack_text")
			return "stack_text";
		// bet_text: textLabel_Set, GameTable.cpp:337 (task 0114).
		if(slot_name == "bet_text")
			return "bet_text";
		// action_icon: actionPic, GameTable.cpp:339 (task 0114).
		if(slot_name == "action_icon")
			return "action_icon";
		// timeout: label_Timeout, GameTable.cpp:340 (task 0114 — found
		// while completing the Context section's "find label_Timeout's
		// rect if it has one" instruction; it does). "timeout_indicator" is
		// an ADDITION to the role vocabulary task 0114 was given — no
		// listed role fits a per-seat turn-timer bar, and the vocabulary
		// was introduced with "e.g." (illustrative, not closed). Flagged
		// explicitly in this task's evidence section as a judgment call.
		if(slot_name == "timeout")
			return "timeout_indicator";
	}
	if(owner_type == "BoardCtrl") {
		// board_card_0..4: TableLayoutProfile.cpp's "ps-6p" branch (task
		// 0113); slot_name is "board_card_<i>" for i in 0..4.
		if(slot_name.StartsWith("board_card_"))
			return "board_card";
	}
	return "unknown";
}

// ---------------------------------------------------------------------------
// "player<N>" -> N. Returns -1 if `variable` doesn't match that pattern
// (empty suffix or a non-digit character in it) — never guesses.
int VsmParseSeatIndex(const String& variable)
{
	static const char prefix[] = "player";
	int plen = (int)strlen(prefix);
	if(variable.GetCount() <= plen || memcmp(~variable, prefix, plen) != 0)
		return -1;
	String suffix = variable.Mid(plen);
	for(int i = 0; i < suffix.GetCount(); i++)
		if(!IsDigit(suffix[i]))
			return -1;
	return StrInt(suffix);
}

// "<slot_name_prefix><N>" -> N, used for both "hole_card_<N>" and
// "board_card_<N>" sub-slot names. Returns -1 if `slot_name` doesn't start
// with `prefix` or the remainder isn't all digits.
static int VsmParseTrailingIndex(const String& slot_name, const String& prefix)
{
	int plen = prefix.GetCount();
	if(slot_name.GetCount() <= plen || memcmp(~slot_name, ~prefix, plen) != 0)
		return -1;
	String suffix = slot_name.Mid(plen);
	for(int i = 0; i < suffix.GetCount(); i++)
		if(!IsDigit(suffix[i]))
			return -1;
	return StrInt(suffix);
}

VsmLayoutProfile VsmBuildLayoutProfile(const VsmFormLayout& layout)
{
	VsmLayoutProfile profile;
	profile.name = layout.name;
	profile.width = layout.width;
	profile.height = layout.height;

	for(const VsmFormElement& el : layout.elements) {
		VsmLayoutElementInfo info;
		info.name = el.name;
		info.type = el.type;
		info.variable = el.variable;
		info.role = VsmClassifyElementRole(el);

		Rect abs_rect = layout.GetAbsoluteRect(el);
		info.x = abs_rect.left;
		info.y = abs_rect.top;
		info.cx = abs_rect.Width();
		info.cy = abs_rect.Height();

		int seat_index = -1;
		if(info.role == "seat") {
			seat_index = VsmParseSeatIndex(el.variable);
			info.seat_index = seat_index;
		}
		profile.elements.Add(info);

		Vector<VsmFormSubSlot> subslots = VsmGetSubSlots(el.type);
		for(const VsmFormSubSlot& s : subslots) {
			VsmLayoutSubSlotInfo si;
			si.owner_name = el.name;
			si.owner_type = el.type;
			si.slot_name = s.name;
			si.role = VsmClassifySubSlotRole(el.type, s.name);
			si.seat_index = seat_index;
			if(si.role == "hole_card")
				si.card_index = VsmParseTrailingIndex(s.name, "hole_card_");
			else if(si.role == "board_card")
				si.card_index = VsmParseTrailingIndex(s.name, "board_card_");

			Rect sr = VsmResolveSubSlot(s, abs_rect);
			si.x = sr.left;
			si.y = sr.top;
			si.cx = sr.Width();
			si.cy = sr.Height();

			profile.subslots.Add(si);
		}
	}

	return profile;
}

} // namespace Upp
