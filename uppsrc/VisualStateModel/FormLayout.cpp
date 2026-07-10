#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// Reads every <property name="..." value="..."/> under `item`'s <properties>
// child (if any) into `el.properties`, in document order, and also mirrors
// the three well-known ones (Type/Variable/Parent) into their own fields for
// convenience. Any property this task doesn't have a named field for (Label,
// Frame, Font.Height, Text.Align, Anchor, ...) is still preserved in
// `properties` — nothing is silently dropped, and an unrecognized property
// name/value never causes a failure or warning (per task 0112's guardrail
// that the parser must be agnostic to whatever widget vocabulary a given
// `.form` file actually uses).
static void VsmReadItemProperties(const XmlNode& item, VsmFormElement& el)
{
	int pi = item.FindTag("properties");
	if(pi < 0)
		return;
	const XmlNode& props = item.Node(pi);
	for(int i = 0; i < props.GetCount(); i++) {
		const XmlNode& p = props.Node(i);
		if(!p.IsTag("property"))
			continue;
		String key = p.Attr("name");
		String val = p.Attr("value");
		if(key.IsEmpty())
			continue;
		el.properties.Add(key, val);
		if(key == "Type")
			el.type = val;
		else if(key == "Variable")
			el.variable = val;
		else if(key == "Parent")
			el.parent = val;
	}
}

// Parses one <content><item ...>...</item> element into a VsmFormElement.
static VsmFormElement VsmReadFormElement(const XmlNode& item)
{
	VsmFormElement el;
	el.x  = item.AttrInt("x", 0);
	el.y  = item.AttrInt("y", 0);
	el.cx = item.AttrInt("cx", 0);
	el.cy = item.AttrInt("cy", 0);

	int ni = item.FindTag("name");
	if(ni >= 0)
		el.name = item.Node(ni).GatherText();

	VsmReadItemProperties(item, el);
	return el;
}

// Parses one <layouts><item> (one designed component) into a VsmFormLayout:
// its <content> child holds the flat element list, its trailing
// <properties> block holds the Form.Width/Form.Height/Form.ScaleMode/
// Form.Name properties (a design-space size, NOT necessarily 1024x648 or any
// other fixed constant — read from the file, never assumed).
static VsmFormLayout VsmReadFormLayout(const XmlNode& layout_item)
{
	VsmFormLayout layout;

	int ci = layout_item.FindTag("content");
	if(ci >= 0) {
		const XmlNode& content = layout_item.Node(ci);
		for(int i = 0; i < content.GetCount(); i++) {
			const XmlNode& item = content.Node(i);
			if(!item.IsTag("item"))
				continue;
			layout.elements.Add(VsmReadFormElement(item));
		}
	}

	int pi = layout_item.FindTag("properties");
	if(pi >= 0) {
		const XmlNode& props = layout_item.Node(pi);
		for(int i = 0; i < props.GetCount(); i++) {
			const XmlNode& p = props.Node(i);
			if(!p.IsTag("property"))
				continue;
			String key = p.Attr("name");
			String val = p.Attr("value");
			if(key == "Form.Width")
				layout.width = StrInt(val);
			else if(key == "Form.Height")
				layout.height = StrInt(val);
			else if(key == "Form.ScaleMode")
				layout.scale_mode = val;
			else if(key == "Form.Name")
				layout.name = val;
		}
	}
	// Fall back to the layout item's own <name> tag if Form.Name wasn't set
	// (not observed in either existing form, but cheap to handle).
	if(layout.name.IsEmpty()) {
		int ni = layout_item.FindTag("name");
		if(ni >= 0)
			layout.name = layout_item.Node(ni).GatherText();
	}

	return layout;
}

Vector<VsmFormLayout> VsmParseFormLayouts(const XmlNode& form_root)
{
	Vector<VsmFormLayout> out;

	const XmlNode& form = form_root.IsTag("form") ? form_root : form_root["form"];
	if(form.IsVoid())
		return out;

	const XmlNode& layouts = form["layouts"];
	if(layouts.IsVoid())
		return out;

	for(int i = 0; i < layouts.GetCount(); i++) {
		const XmlNode& item = layouts.Node(i);
		if(!item.IsTag("item"))
			continue;
		out.Add(VsmReadFormLayout(item));
	}

	return out;
}

Vector<VsmFormLayout> VsmParseFormFile(const String& path)
{
	Vector<VsmFormLayout> out;
	if(!FileExists(path))
		return out;
	XmlNode root = ParseXMLFile(path);
	if(root.IsVoid())
		return out;
	return VsmParseFormLayouts(root);
}

// ---------------------------------------------------------------------------
// M04-02 (task 0113) sub-slot data table.
//
// PlayerCtrl rows: transcribed VERBATIM from `PlayerCtrl::Layout()`,
// game/TexasHoldem/GameTable.cpp:321-341 — that function computes
// `fx = sz.cx/190.0`, `fy = sz.cy/142.0` (own PlayerCtrl size against a
// 190x142 reference), then:
//   pixmapLabel_carda.SetRect((int)(80*fx), (int)(10*fy), max(1,(int)(48*fx)), max(1,(int)(60*fy)));   // :332
//   pixmapLabel_cardb.SetRect((int)(110*fx),(int)(10*fy), max(1,(int)(48*fx)), max(1,(int)(60*fy)));   // :333
//   textLabel_Button.SetRect((int)(140*fx),(int)(75*fy), max(1,(int)(32*fx)), max(1,(int)(32*fy)));    // :336
// which is already a clean ratio-of-own-rect formula, so the fractions
// below (num/190 or num/142) are exactly those same numbers, just
// pre-divided. `button_puck`'s image (dealer/SB/BB) is chosen at
// GameTable.cpp:1570-1581 based on GBUTTON_DEALER/GBUTTON_SMALL_BLIND/
// GBUTTON_BIG_BLIND, but its RECT (this table's concern) is the single
// `textLabel_Button` rect above regardless of which puck image is shown.
//
// BoardCtrl rows: transcribed from the legacy "ps-6p" profile branch,
// game/Poker/TableLayoutProfile.cpp:43-61 (`board_x=641, board_y=349,
// board_step=131, board_w=122, board_h=143`), against the profile's
// `TexasTableLayout::BaseSize()` == 1920x1080 (TableLayoutProfile.cpp:115-
// 117), as used by `BoardCardRect()` (TableLayoutProfile.cpp:147-155):
// `x = (int)(board_x*Sx(sz)) + i*(int)(board_step*Sx(sz))`,
// `y = (int)(board_y*Sy(sz))`, `cw = (int)(board_w*Sx(sz))`,
// `ch = (int)(board_h*Sy(sz))`, where `Sx(sz)=sz.cx/1920`, `Sy(sz)=sz.cy/1080`.
// Note the base offset and the `i*step` term are each truncated to int
// SEPARATELY before being summed — this is why `fx` below holds only the
// base term and `fstep_x`/`index` hold the step term (see VsmFormSubSlot's
// comment): a first attempt that pre-combined `(board_x + i*board_step)`
// into one fraction before truncating was off by 1-4px per index (verified,
// then corrected, in task 0113's evidence section) because
// `(int)(a)+(int)(b) != (int)(a+b)` in general. With base/step kept
// separate, this reproduces `BoardCardRect`'s rects EXACTLY (bit-for-bit),
// verified across 8 board sizes (640x400 up to 3840x2160) x 5 indices = 40
// cases, not merely approximately. This is a PS_6p-specific transcription
// (same scope as the legacy profile it came from) — a future platform
// whose `.form`-driven board rendering differs would need its own
// BoardCtrl sub-slot data, same limitation the legacy per-platform
// `LayoutBaseCoords` struct already had, just relocated from C++ code to a
// data row.
Vector<VsmFormSubSlot> VsmGetSubSlots(const String& element_type)
{
	Vector<VsmFormSubSlot> out;
	if(element_type == "PlayerCtrl") {
		VsmFormSubSlot s;
		s.type = "PlayerCtrl";
		s.name = "hole_card_0";   // pixmapLabel_carda, GameTable.cpp:332
		s.fx = 80.0/190.0;  s.fy = 10.0/142.0;  s.fcx = 48.0/190.0;  s.fcy = 60.0/142.0;
		out.Add(s);

		s.name = "hole_card_1";   // pixmapLabel_cardb, GameTable.cpp:333
		s.fx = 110.0/190.0; s.fy = 10.0/142.0;  s.fcx = 48.0/190.0;  s.fcy = 60.0/142.0;
		out.Add(s);

		s.name = "button_puck";   // textLabel_Button, GameTable.cpp:336
		s.fx = 140.0/190.0; s.fy = 75.0/142.0;  s.fcx = 32.0/190.0;  s.fcy = 32.0/142.0;
		out.Add(s);
	}
	else if(element_type == "BoardCtrl") {
		const double board_x = 641.0, board_y = 349.0, board_step = 131.0;
		const double board_w = 122.0, board_h = 143.0;
		const double base_cx = 1920.0, base_cy = 1080.0;
		for(int i = 0; i < 5; i++) {
			VsmFormSubSlot s;
			s.type = "BoardCtrl";
			s.name = "board_card_" + AsString(i);
			s.fx  = board_x / base_cx;   // BASE only (index==0 position) — see step fields below
			s.fy  = board_y / base_cy;
			s.fcx = board_w / base_cx;
			s.fcy = board_h / base_cy;
			s.index = i;
			s.fstep_x = board_step / base_cx;
			out.Add(s);
		}
	}
	return out;
}

Rect VsmResolveSubSlot(const VsmFormSubSlot& slot, const Rect& owner_rect)
{
	int ow = owner_rect.GetWidth();
	int oh = owner_rect.GetHeight();
	// Base and step truncated SEPARATELY, then summed — matches the legacy
	// `(int)(board_x*Sx) + i*(int)(board_step*Sx)` order exactly (see
	// VsmFormSubSlot::index's comment for why this matters).
	int lx  = (int)(slot.fx * ow) + slot.index * (int)(slot.fstep_x * ow);
	int ly  = (int)(slot.fy * oh) + slot.index * (int)(slot.fstep_y * oh);
	int lcx = max(1, (int)(slot.fcx * ow));
	int lcy = max(1, (int)(slot.fcy * oh));
	return Rect(owner_rect.left + lx, owner_rect.top + ly,
	            owner_rect.left + lx + lcx, owner_rect.top + ly + lcy);
}

} // namespace Upp
