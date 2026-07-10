#ifndef _VisualStateModel_LayoutProfile_h_
#define _VisualStateModel_LayoutProfile_h_

namespace Upp {

// ---------------------------------------------------------------------------
// M04-03 (task 0114): semantic role classification + the `VsmLayoutProfile`
// artifact.
//
// Tasks 0112/0113 gave us a generic element list (`VsmFormLayout`) and
// generic per-Type sub-slot geometry (`VsmFormSubSlot`/`VsmGetSubSlots`/
// `VsmResolveSubSlot`) for a parsed `.form` file. Neither of those attach any
// MEANING to an element/sub-slot beyond its raw `Type`/`Variable` — this file
// adds that layer: a small, documented classification from
// (`Type`, `Variable`, sub-slot name) to a semantic ROLE string (`"seat"`,
// `"pot_amount"`, `"hole_card"`, ...), plus `VsmLayoutProfile`, the single
// artifact that holds every top-level element AND every resolved sub-slot for
// one parsed `.form` layout, each tagged with its role and (where applicable)
// its owning seat/card index.
//
// This is the milestone's "layout model artifact for PS_6p" deliverable —
// made generic across `.form` files (nothing here hardcodes "PS_6p" or any
// fixed widget count), per the M04 architecture direction already
// established in tasks 0112/0113.
//
// Region-to-element assignment (matching a *detected* screen region against
// this profile) is explicitly out of scope here — that is task 0115, which
// consumes this file's `VsmLayoutProfile` as input.

// ---------------------------------------------------------------------------
// Role vocabulary used by `VsmClassifyElementRole`/`VsmClassifySubSlotRole`
// below (plain `String`, not an enum, for the same "agnostic to an unknown
// future vocabulary" reason `VsmFormElement::type` is a `String` — a future
// `.form` with a widget this file doesn't recognize gets `"unknown"` rather
// than a compile error):
//
//   seat            - a PlayerCtrl's own top-level rect (one whole seat)
//   board           - the BoardCtrl's own top-level rect (whole board area)
//   board_card      - one BoardCtrl sub-slot (a single community card slot)
//   hole_card       - one PlayerCtrl sub-slot (a single hole-card slot)
//   dealer_button   - the PlayerCtrl "button_puck" sub-slot (dealer/SB/BB puck)
//   pot_amount      - the pot caption + pot total value labels
//   pot_bets        - the current-round bets-into-pot label
//   turn_label      - the current street/turn name label (e.g. "Preflop")
//   game_info       - the free-text game-info label
//   hand_info       - the free-text hand-info label
//   action_button   - a human action button (fold/check/raise/all-in/pot%)
//   human_input     - a human bet-amount input (EditInt or its bet slider)
//   status_pane     - a TabCtrl hosting one or more status sub-panes
//   player_name     - the PlayerCtrl "player_name" sub-slot
//   stack_text      - the PlayerCtrl "stack_text" sub-slot (chip stack)
//   bet_text        - the PlayerCtrl "bet_text" sub-slot (current bet/set)
//   avatar          - the PlayerCtrl "avatar" sub-slot
//   action_icon     - the PlayerCtrl "action_icon" sub-slot (check/call/...)
//   timeout_indicator - the PlayerCtrl "timeout" sub-slot (turn-timer bar) —
//                     NOTE: this role is an ADDITION to the vocabulary task
//                     0114 was given as an "e.g." starting list (see this
//                     task's evidence section for why: there is no listed
//                     role that fits this sub-slot, and the task file's own
//                     wording ("e.g.") frames that list as illustrative, not
//                     closed).
//   speed_control   - the playback-speed label + slider (paired, one role)
//   pause_button    - the single Pause button
//   chat_pane       - the poker chat control
//   hands_pane      - the shown-hands image + its containing pane
//   probability_pane - the win-probability control
//   log_pane        - a log/history text view (engine log or away log)
//   unknown         - classification found no rule for this element/sub-slot
//                     (see VsmClassifyElementRole's doc comment for the two
//                     specific cases task 0114 deliberately leaves this way)

// One top-level `.form` element, tagged with its classified role and
// resolved absolute rect (`VsmFormLayout::GetAbsoluteRect()`'s result, so
// `Parent`-nested elements like `HumanButtons`'s action buttons already carry
// their true on-screen rect here, not their parent-relative one).
struct VsmLayoutElementInfo : Moveable<VsmLayoutElementInfo> {
	String name;          // <name> tag, e.g. "Player0"
	String type;          // Type property, e.g. "PlayerCtrl"
	String variable;       // Variable property, e.g. "player0"
	String role;           // classified role, see vocabulary above
	int    seat_index = -1;  // for role=="seat": parsed from `variable` ("playerN" -> N); -1 if n/a
	int    x = 0, y = 0, cx = 0, cy = 0;  // ABSOLUTE rect (post GetAbsoluteRect)

	Rect GetRect() const { return Rect(x, y, x + cx, y + cy); }

	void Jsonize(JsonIO& json) {
		json
			("name", name)("type", type)("variable", variable)("role", role)
			("seat_index", seat_index)
			("x", x)("y", y)("cx", cx)("cy", cy)
		;
	}
};

// One resolved sub-slot (`VsmFormSubSlot` resolved via `VsmResolveSubSlot`
// against its owning element's absolute rect), tagged with its classified
// role and (where applicable) the seat/card index it belongs to.
struct VsmLayoutSubSlotInfo : Moveable<VsmLayoutSubSlotInfo> {
	String owner_name;     // owning element's <name>, e.g. "Player0"
	String owner_type;     // owning element's Type, e.g. "PlayerCtrl"
	String slot_name;       // sub-slot's own name, e.g. "hole_card_0"
	String role;            // classified role, see vocabulary above
	int    seat_index = -1;   // owning PlayerCtrl's seat index, if owner_type=="PlayerCtrl"; else -1
	int    card_index = -1;   // hole_card/board_card index parsed from slot_name; -1 if n/a
	int    x = 0, y = 0, cx = 0, cy = 0;  // ABSOLUTE rect

	Rect GetRect() const { return Rect(x, y, x + cx, y + cy); }

	void Jsonize(JsonIO& json) {
		json
			("owner_name", owner_name)("owner_type", owner_type)
			("slot_name", slot_name)("role", role)
			("seat_index", seat_index)("card_index", card_index)
			("x", x)("y", y)("cx", cx)("cy", cy)
		;
	}
};

// The M04 "layout model artifact" itself: every top-level element of one
// parsed `.form` layout (`VsmFormLayout`), plus every resolved sub-slot,
// each tagged with role/rect/owner reference. Deliberately generic — nothing
// here assumes "PS_6p", a fixed player count, or a fixed widget vocabulary;
// a future `.form` with a different widget set still produces a valid
// profile, just with more `role=="unknown"` entries until this file's
// classification rules are extended to cover it (by adding rows, per this
// task's guardrail, not by rewriting the dispatch logic).
struct VsmLayoutProfile : Moveable<VsmLayoutProfile> {
	String name;                            // layout's Form.Name, e.g. "GameTable"
	int    width = 0, height = 0;           // layout's design-space size

	Vector<VsmLayoutElementInfo> elements;   // one per top-level `.form` element
	Vector<VsmLayoutSubSlotInfo> subslots;   // one per resolved sub-slot (0 or more per element)

	void Jsonize(JsonIO& json) {
		json
			("name", name)("width", width)("height", height)
			("elements", elements)("subslots", subslots)
		;
	}
};

// Classifies one top-level `.form` element's semantic role. Primary signal is
// `element.type`; where `type` alone is ambiguous (several `Label` elements
// with different meanings, `SliderCtrl` used for both the bet slider and the
// speed slider, `StaticRect`/`ImageCtrl` used for the hands-pane background
// and its image), falls back to `element.variable` (checked FIRST — see
// FormLayout.cpp's implementation comment for the exact table and its
// file:line provenance for every mapping). Never fails/asserts on an
// unrecognized type or variable — returns `"unknown"` instead, per the
// "agnostic to a future `.form`'s widget vocabulary" direction already
// established in 0112/0113.
//
// Two specific elements are classified `"unknown"` on PURPOSE, not because
// classification was incomplete (see FormLayout.cpp for the reasoning):
// `HumanButtons` itself (a `ParentCtrl` — purely a layout container for the
// action buttons/slider/edit box, no on-screen content of its own), and any
// element whose `Type`/`Variable` this table genuinely has no rule for yet.
String VsmClassifyElementRole(const VsmFormElement& element);

// Classifies one resolved sub-slot's semantic role, given its owning
// element's `Type` and the sub-slot's own `name` (as returned by
// `VsmGetSubSlots`, e.g. "hole_card_0", "button_puck", "avatar",
// "board_card_3"). See FormLayout.cpp for the exact table.
String VsmClassifySubSlotRole(const String& owner_type, const String& slot_name);

// Parses a PlayerCtrl's seat index from its `Variable` property ("player0"
// -> 0, "player9" -> 9), NOT from its `<name>` tag ("Player0") — the
// `Variable` is the more stable "intent" signal a differently-named future
// `.form` is more likely to preserve (same reasoning as `Parent` resolution
// in FormLayout.h). Returns -1 if `variable` doesn't match the "player<N>"
// pattern.
int VsmParseSeatIndex(const String& variable);

// Builds the full `VsmLayoutProfile` for one already-parsed `VsmFormLayout`:
// classifies every element's role (and seat index, if role=="seat"), resolves
// every element's known sub-slots (`VsmGetSubSlots`/`VsmResolveSubSlot`) to
// absolute rects, and classifies each sub-slot's role (and seat/card index,
// if applicable).
VsmLayoutProfile VsmBuildLayoutProfile(const VsmFormLayout& layout);

} // namespace Upp

#endif
