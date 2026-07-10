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

} // namespace Upp

#endif
