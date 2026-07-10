#ifndef _VisualStateModel_FormLayout_h_
#define _VisualStateModel_FormLayout_h_

namespace Upp {

// ---------------------------------------------------------------------------
// M04-01: generic `.form`-file-driven layout element model.
//
// `.form` files (TheIDE's layout designer XML format, see e.g.
// game/TexasHoldem/GameTable_PS_6p.form) declare a flat list of widget
// `<item>`s inside `<layouts><item><content>`, each with a design-space rect
// (`x`/`y`/`cx`/`cy` attributes) and a `<properties>` block of name/value
// pairs. This model reads that list as-is — it does not hardcode any fixed
// widget-name schema (no `PlayerNameRect`-style struct fields) so the same
// code works across `.form` files that declare different widget sets. Callers
// that want to know "which element is the pot label" classify by the
// element's `type` (the `Type` property, e.g. "PlayerCtrl", "BoardCtrl",
// "ActionButton", "Label", ...) rather than by a fixed field name.
//
// Region-to-element assignment (mapping coarse detected regions onto these
// elements) is out of scope here — this is only the parse-into-a-model step
// (see task 0112 / a later task for that logic).

// ---------------------------------------------------------------------------
// One `<item>` from a `.form` file's `<content>` list.
//
// `parent` is the raw `Parent` property value (e.g. "humanButtons" for the
// action buttons nested under `HumanButtons`'s `ParentCtrl`), present only on
// child items. It is preserved as-is (not yet resolved into an absolute
// rect) — see the note on `VsmFormLayout::GetAbsoluteRect()` below for why
// full nested-rect resolution is deliberately deferred.
struct VsmFormElement : Moveable<VsmFormElement> {
	String name;                          // <name> tag text, e.g. "Player0"
	String type;                          // Type property, e.g. "PlayerCtrl"
	String variable;                      // Variable property, e.g. "player0"
	String parent;                        // Parent property, empty if none
	int    x = 0, y = 0, cx = 0, cy = 0;  // design-space rect (item's own x/y/cx/cy attrs)

	// Every <property name="..." value="..."/> found on this item, in
	// document order, including Type/Variable/Parent (also broken out above
	// for convenience) and anything else (Label, Frame, Font.Height,
	// Text.Align, Anchor, ...). Keeping the full raw set — rather than only
	// the fields this task happens to need — is what makes the model
	// agnostic to unrecognized properties/types: nothing is dropped just
	// because this task doesn't have a named field for it.
	VectorMap<String, String> properties;

	String GetProperty(const String& key, const String& def = String()) const {
		int q = properties.Find(key);
		return q >= 0 ? properties[q] : def;
	}

	// Design-space rect in this item's own coordinate frame. For a child item
	// (non-empty `parent`), this is relative to the parent's rect, NOT an
	// absolute/screen-space rect — see VsmFormLayout::GetAbsoluteRect().
	Rect GetRect() const { return Rect(x, y, x + cx, y + cy); }

	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("type", type)
			("variable", variable)
			("parent", parent)
			("x", x)("y", y)("cx", cx)("cy", cy)
		;
	}
};

// ---------------------------------------------------------------------------
// One `<layouts><item>` from a `.form` file — i.e. one designed component
// (today's `.form` files each declare exactly one, named e.g. "GameTable").
struct VsmFormLayout : Moveable<VsmFormLayout> {
	String name;                          // Form.Name property
	int    width = 0, height = 0;         // Form.Width / Form.Height (design-space size)
	String scale_mode;                    // Form.ScaleMode property, e.g. "Relative"

	Vector<VsmFormElement> elements;      // flat list, document order preserved

	// Look up an element by its <name>. Returns nullptr if not found. Deliberately
	// does not assume any particular name exists (callers must not hardcode
	// "there are exactly 6 players" etc. — see task 0112's guardrails).
	const VsmFormElement* Find(const String& element_name) const {
		for(const VsmFormElement& e : elements)
			if(e.name == element_name)
				return &e;
		return NULL;
	}

	// Look up an element by its Variable property. A `Parent` property value
	// (e.g. "humanButtons") names the parent's *Variable*, not its <name> tag
	// (e.g. "HumanButtons") — confirmed against both existing .form files,
	// where every parent/child pair differs in exactly that way. Used by
	// GetAbsoluteRect() below; exposed here too since any future caller
	// resolving `parent` must do the same variable-based lookup, not a
	// name-based one.
	const VsmFormElement* FindByVariable(const String& variable_name) const {
		for(const VsmFormElement& e : elements)
			if(e.variable == variable_name)
				return &e;
		return NULL;
	}

	// Resolves `element`'s rect to this layout's own design-space coordinate
	// frame by walking its `parent` chain (an element whose `parent` names
	// another element in this same layout has its rect declared relative to
	// that parent's rect, e.g. `BtnFold`'s x/y is relative to `HumanButtons`).
	// Deferred-scope note (see task 0112 evidence): this resolves the common
	// one-level-of-nesting case seen in both existing forms (action buttons
	// under `HumanButtons`, `HandsImage` under `handsImgPane`) and guards
	// against a cyclic/self-referential `Parent` chain, but does not attempt
	// to apply `align`/`valign`/`Anchor` semantics (anchoring/relative-scale
	// resize behavior) — only additive rect nesting. That's intentionally
	// left as a follow-up if a future `.form` needs it.
	Rect GetAbsoluteRect(const VsmFormElement& element) const {
		int ax = element.x, ay = element.y;
		String parent_variable = element.parent;
		int guard = 0;
		while(!parent_variable.IsEmpty() && guard++ < elements.GetCount()) {
			const VsmFormElement* p = FindByVariable(parent_variable);
			if(!p)
				break;
			ax += p->x;
			ay += p->y;
			parent_variable = p->parent;
		}
		return Rect(ax, ay, ax + element.cx, ay + element.cy);
	}

	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("width", width)("height", height)
			("scale_mode", scale_mode)
			("elements", elements)
		;
	}
};

// Parses the `<layouts>` list of an already-parsed `.form` document (the
// `<form>` root node as returned by ParseXML/ParseXMLFile). Returns one
// VsmFormLayout per `<layouts><item>` found (today's forms declare exactly
// one; the model does not assume that stays true).
Vector<VsmFormLayout> VsmParseFormLayouts(const XmlNode& form_root);

// Convenience: reads and parses a `.form` file from disk in one call.
// Returns an empty vector if the file can't be read/parsed as XML.
Vector<VsmFormLayout> VsmParseFormFile(const String& path);

// ---------------------------------------------------------------------------
// M04-02 (task 0113): generic per-Type sub-slot geometry.
//
// `.form` files only give COARSE element rects: `BoardCtrl` = the whole
// board area, `PlayerCtrl` = one player's whole block. The individual
// board-card slots (5), hole-card slots (2 per player), and the
// dealer/small-blind/big-blind puck position are not separate `.form`
// items — at render time they are computed as fixed FRACTIONS of the
// owning element's own rect (see GameTable.cpp:321-341's `PlayerCtrl::
// Layout()` for hole cards/puck, and TableLayoutProfile.cpp's
// `TexasTableLayout::BoardCardRect` for board cards). This struct captures
// that same fraction-of-own-rect relationship as DATA, keyed by the owning
// element's `type`, rather than new hardcoded C++ per platform — see
// VsmGetSubSlots() below for the concrete PS_6p-derived table and task
// 0113's evidence section (Manager repo,
// VisualStateModel/0113_m04_subslot_geometry_decision.md) for the full
// derivation/verification.
struct VsmFormSubSlot : Moveable<VsmFormSubSlot> {
	String type;   // owning element's Type this sub-slot applies to, e.g. "PlayerCtrl", "BoardCtrl"
	String name;   // this sub-slot's own name, e.g. "hole_card_0", "board_card_2", "button_puck"
	double fx = 0, fy = 0, fcx = 0, fcy = 0;  // fraction of the OWNING element's own rect (its cx/cy)

	// Optional repeated-slot step, for a family of slots laid out as
	// `base + index*step` along an axis (BoardCtrl's 5 board-card slots).
	// The legacy renderer (TableLayoutProfile.cpp's BoardCardRect) truncates
	// the base offset and the `index*step` term to integer pixels
	// SEPARATELY before summing (`(int)(board_x*Sx) + i*(int)(board_step*Sx)`),
	// NOT the combined `(base+index*step)` truncated once — the two give
	// different (off-by-a-few-pixels) results at most sizes. To reproduce
	// the legacy rects exactly, `fx`/`fy` above hold ONLY the base (index==0)
	// fraction here, and `fstep_x`/`fstep_y` hold the per-index step
	// fraction; VsmResolveSubSlot() truncates base and step separately and
	// sums, exactly mirroring the legacy order. Slots with no stepping
	// (index always 0, e.g. every PlayerCtrl sub-slot) leave these at 0,
	// which is a no-op — see task 0113's evidence section for the numeric
	// verification that this exact-truncation-order approach is required
	// (an earlier, simpler single-fraction-per-slot attempt was off by up
	// to several pixels and was corrected before this was checked in).
	int index = 0;
	double fstep_x = 0, fstep_y = 0;

	// M05-05 (task 0123): true for a sub-slot whose SIZE (not position) is
	// locked to a single uniform scale factor of the owning element's own
	// rect, mirroring GameTable.cpp's `PlayerCtrl::Layout()` change to
	// `textLabel_Button` (`button_puck`): `f_puck = fmin(fx, fy)`, both
	// SetRect() size args using that same single factor, rather than each
	// axis independently scaling by its own fx/fy. `fcx`/`fcy` are NOT
	// reinterpreted or given new meaning for this case -- they still hold
	// exactly the same per-axis fractions they always would (32/190, 32/142
	// for button_puck, transcribed straight from the two independent
	// SetRect() size args as if unlocked). VsmResolveSubSlot() derives the
	// locked size from those SAME two fractions at resolution time
	// (`min(fcx*ow, fcy*oh)`, applied to both axes), which is arithmetically
	// identical to `GameTable.cpp`'s `(int)(32 * fmin(fx, fy))` -- see
	// VsmResolveSubSlot()'s own comment for the equivalence proof. This
	// keeps VsmFormSubSlot's existing fraction-of-own-rect representation
	// unchanged for the position fields and every non-aspect-locked slot;
	// only resolution gets a small, explicit accommodation for this one
	// case, rather than inventing a new "uniform scale fraction" field that
	// would duplicate fcx/fcy's information.
	bool aspect_lock = false;
};

// Returns every known sub-slot declared for `element_type` (empty if none
// known for that type — callers must not assume every type has sub-slots).
// See FormLayout.cpp for the concrete data table and its file:line
// provenance for every number.
Vector<VsmFormSubSlot> VsmGetSubSlots(const String& element_type);

// Resolves `slot` to an absolute rect given the owning element's own
// absolute rect `owner_rect` (e.g. a top-level element's own design-space
// rect, or the result of VsmFormLayout::GetAbsoluteRect() for a nested
// element). Mirrors both the integer-truncation pattern AND its ORDER
// (base and `index*step` truncated separately, then summed — see
// VsmFormSubSlot::index's comment above) used by the live renderer
// (GameTable.cpp:321-341, TableLayoutProfile.cpp:147-155), so results
// reproduce the live numbers exactly rather than merely approximately
// (verified numerically across 8 board sizes in task 0113's evidence
// section).
// Note (task 0123): if `slot.aspect_lock` is set, BOTH resulting axes' size
// use `min(fcx*ow, fcy*oh)` (see VsmFormSubSlot::aspect_lock's comment) --
// the "base and step truncated separately" note above still applies
// independently to each axis's position term, unaffected by aspect_lock.
Rect VsmResolveSubSlot(const VsmFormSubSlot& slot, const Rect& owner_rect);

} // namespace Upp

#endif
