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
// M04-02 (task 0113) / M04-03 (task 0114) sub-slot data table.
//
// PlayerCtrl rows: transcribed VERBATIM from `PlayerCtrl::Layout()`,
// game/TexasHoldem/GameTable.cpp:321-341 — that function computes
// `fx = sz.cx/190.0`, `fy = sz.cy/142.0` (own PlayerCtrl size against a
// 190x142 reference), then:
//   label_Avatar.SetRect((int)(10*fx), (int)(10*fy), max(1,(int)(60*fx)), max(1,(int)(60*fy)));        // :328
//   label_PlayerName.SetRect((int)(10*fx),(int)(75*fy), max(1,(int)(170*fx)), max(1,(int)(15*fy)));    // :329
//   textLabel_Cash.SetRect((int)(10*fx),(int)(90*fy), max(1,(int)(170*fx)), max(1,(int)(15*fy)));      // :330
//   pixmapLabel_carda.SetRect((int)(80*fx), (int)(10*fy), max(1,(int)(48*fx)), max(1,(int)(60*fy)));   // :332
//   pixmapLabel_cardb.SetRect((int)(110*fx),(int)(10*fy), max(1,(int)(48*fx)), max(1,(int)(60*fy)));   // :333
//   textLabel_Button.SetRect((int)(140*fx),(int)(75*fy), max(1,(int)(32*fx)), max(1,(int)(32*fy)));    // :336
//   textLabel_Set.SetRect((int)(10*fx),(int)(105*fy), max(1,(int)(170*fx)), max(1,(int)(15*fy)));      // :337
//   actionPic.SetRect((int)(10*fx),(int)(120*fy), max(1,(int)(170*fx)), max(1,(int)(20*fy)));          // :339
//   label_Timeout.SetRect((int)(10*fx),(int)(70*fy), max(1,(int)(60*fx)), max(1,(int)(5*fy)));         // :340
// which is already a clean ratio-of-own-rect formula, so the fractions
// below (num/190 or num/142) are exactly those same numbers, just
// pre-divided. `button_puck`'s image (dealer/SB/BB) is chosen at
// GameTable.cpp:1570-1581 based on GBUTTON_DEALER/GBUTTON_SMALL_BLIND/
// GBUTTON_BIG_BLIND, but its RECT (this table's concern) is the single
// `textLabel_Button` rect above regardless of which puck image is shown.
//
// Task 0114 re-read the FULL `PlayerCtrl` constructor (GameTable.cpp:293-319)
// and `Layout()` (GameTable.cpp:321-341) to confirm this list is complete.
// Every child `Add()`ed in the constructor (:296-306: `bg`, `label_Avatar`,
// `pixmapLabel_carda`, `pixmapLabel_cardb`, `pixmapLabel_cards`,
// `label_PlayerName`, `textLabel_Cash`, `textLabel_Set`, `textLabel_Button`,
// `actionPic`, `label_Timeout` — 11 total) has a corresponding `SetRect()` in
// `Layout()` — none are left to parent/anchor-only auto-sizing. Two of the
// 11 are DELIBERATELY not added as sub-slots here, judgment calls made and
// documented in task 0114's evidence section rather than silently omitted:
//   - `bg` (:327, `SetRect(0,0,sz.cx,sz.cy)`) — identical to the owning
//     PlayerCtrl's own rect (fraction 1,1 of itself), so it carries no
//     information beyond what the top-level `PlayerCtrl` element's own
//     resolved rect already gives; adding it as a sub-slot would be a
//     no-op duplicate.
//   - `pixmapLabel_cards` (:334, `SetRect((int)(80*fx),(int)(10*fy),
//     max(1,(int)(78*fx)),max(1,(int)(60*fy)))`) — the combined face-down
//     card-back image shown INSTEAD of `pixmapLabel_carda`/`cardb` when a
//     hand is hidden (GameTable.cpp:1526-1552 toggles `.Show()`/`.Hide()`
//     between this and the two individual card labels). Its rect is exactly
//     the union of `hole_card_0`'s and `hole_card_1`'s rects (80..158 wide,
//     matching 80+78=158 = 110+48), so it introduces no NEW geometry beyond
//     those two slots — it is a rendering-state detail (which image is drawn
//     into the same on-screen area), not a distinct region.
//
// BoardCtrl rows: transcribed from the "texas-holdem-legacy-pokertable"
// profile branch, game/Poker/TableLayoutProfile.cpp:63-83 (`board_x=680,
// board_y=293, board_step=113, board_w=94, board_h=133`), against the
// profile's `TexasTableLayout::BaseSize()` == 1920x1080 (TableLayoutProfile.cpp:
// 115-117), as used by `BoardCardRect()` (TableLayoutProfile.cpp:147-155):
// `x = (int)(board_x*Sx(sz)) + i*(int)(board_step*Sx(sz))`,
// `y = (int)(board_y*Sy(sz))`, `cw = (int)(board_w*Sx(sz))`,
// `ch = (int)(board_h*Sy(sz))`, where `Sx(sz)=sz.cx/1920`, `Sy(sz)=sz.cy/1080`.
//
// *** ROOT-CAUSE NOTE (task 0129, M05-11) — why legacy-pokertable and NOT
// ps-6p: ***
// Tasks 0113-0128 believed the PS_6p recording rendered board cards with the
// "ps-6p" profile (`board_x=641, board_y=349, board_step=131, board_w=122,
// board_h=143`), matching the provider name and the .form file. Task 0126
// then measured — via real GameTable::PaintBoard instrumentation — that the
// true on-screen board_card_0 rect at a 1024x625 frame is (362,169)-(412,245)
// [50x76], NOT the (362 vs 341, 50 vs 65...) the ps-6p constants predict, and
// worked around it locally (VsmEmpiricalBoardCardRect in
// reference/VisualStateLogicCompare/main.cpp) while attributing the gap to a
// theorized Form scale_mode=2 "letterbox" shrinking Board().GetSize().
//
// Task 0129 re-instrumented PaintBoard fresh (temporary Cout, added+removed;
// see the evidence section of Manager task
// 0129_m05_board_card_geometry_root_fix.md) and found that theory was WRONG:
// Board().GetSize() IS the full (1024,625) (board.GetRect()==[0,0]-[1024,625],
// no letterbox), but the ACTIVE PROFILE at PaintBoard time is
// "texas-holdem-legacy-pokertable", not "ps-6p". The legacy-pokertable
// constants above reproduce the instrumented render (362,169,50,76 for card 0,
// step 60) BIT-FOR-BIT at (1024,625); the ps-6p constants never could for any
// board size (a real inconsistency the task spec itself flagged). Why the
// profile flips despite `--provider PS_6p`: GameTable's constructor sets
// "ps-6p" (GameTable.cpp:576), but StartTexasHoldemLocalGame later calls
// SetProjectContext("default","ps-6p") (TexasHoldemLocalGame.cpp:58); no
// "ps-6p" platform script exists, so SetProjectContext falls back to
// texas-holdem.py and RESETS m_scriptPlatform to "texas-holdem"
// (GameTable.cpp:888-892); ApplyProjectThemeMetadata then calls
// ResolveThemeLayoutProfile(emptyTheme,"texas-holdem"), which returns
// "texas-holdem-legacy-pokertable" (GameTheme.cpp:195-196). That last
// SetProfile wins, so every board card the recorder captures is drawn with
// legacy-pokertable geometry. This VSM table must therefore replicate
// legacy-pokertable to match what is actually on screen (per this task's
// guardrail: fix the model's replica, not the real renderer).
//
// Note the base offset and the `i*step` term are each truncated to int
// SEPARATELY before being summed — this is why `fx` below holds only the
// base term and `fstep_x`/`index` hold the step term (see VsmFormSubSlot's
// comment): a first attempt that pre-combined `(board_x + i*board_step)`
// into one fraction before truncating was off by 1-4px per index (verified,
// then corrected, in task 0113's evidence section) because
// `(int)(a)+(int)(b) != (int)(a+b)` in general. With base/step kept
// separate, this reproduces `BoardCardRect`'s rects EXACTLY (bit-for-bit)
// when resolved against the ACTUAL board size the renderer uses. (The
// downstream VsmBuildCandidates path resolves sub-slots against the .form's
// design-space (1024x648) Board rect and only THEN scales to the decoded
// frame size (1024x625) — a resolve-then-scale that costs ~1px vs. the
// renderer's resolve-directly-at-625; that residual is inherent to the whole
// sub-slot pipeline, affects every sub-slot equally, and is why the compare
// tool's bit-exact VsmEmpiricalBoardCardRect override is kept — see task 0129
// §3.) This is a real-render-specific transcription (legacy-pokertable is the
// profile the recorder actually renders with) — a future platform whose
// `.form`-driven board rendering resolves to a different active profile would
// need its own BoardCtrl sub-slot data, same limitation the legacy
// per-platform `LayoutBaseCoords` struct already had, just relocated from C++
// code to a data row.
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
		// M05-05 (task 0123): GameTable.cpp's PlayerCtrl::Layout() now locks
		// this SIZE to a single uniform scale factor (`f_puck = fmin(fx,
		// fy)`, both SetRect() size args using it) instead of independent
		// fx/fy, so the rect is always square regardless of the owning
		// PlayerCtrl box's own aspect ratio. fcx/fcy below are UNCHANGED
		// (still the verbatim 32/190, 32/142 transcription) -- aspect_lock
		// tells VsmResolveSubSlot() to derive the locked size from them at
		// resolution time (see VsmFormSubSlot::aspect_lock's comment in
		// FormLayout.h) rather than giving fcx/fcy new meaning here.
		s.fx = 140.0/190.0; s.fy = 75.0/142.0;  s.fcx = 32.0/190.0;  s.fcy = 32.0/142.0;
		s.aspect_lock = true;
		out.Add(s);
		// `s` is reused (not reset) between rows below (same pattern this
		// table already relied on for `index`/`fstep_x`/`fstep_y` staying
		// at their zero defaults) -- `aspect_lock` MUST be explicitly reset
		// to false here, or every row after button_puck would silently
		// inherit its aspect-locked SIZE resolution too (caught by
		// VisualStateModelTests: Player0.avatar/player_name would otherwise
		// resolve to the wrong, unintentionally-square-clamped size).
		s.aspect_lock = false;

		// --- task 0114 additions (see the header comment above for the
		// full-constructor re-verification these come from) ---

		s.name = "avatar";        // label_Avatar, GameTable.cpp:328
		s.fx = 10.0/190.0;  s.fy = 10.0/142.0;  s.fcx = 60.0/190.0;  s.fcy = 60.0/142.0;
		out.Add(s);

		s.name = "player_name";   // label_PlayerName, GameTable.cpp:329
		s.fx = 10.0/190.0;  s.fy = 75.0/142.0;  s.fcx = 170.0/190.0; s.fcy = 15.0/142.0;
		out.Add(s);

		s.name = "stack_text";    // textLabel_Cash, GameTable.cpp:330
		s.fx = 10.0/190.0;  s.fy = 90.0/142.0;  s.fcx = 170.0/190.0; s.fcy = 15.0/142.0;
		out.Add(s);

		s.name = "bet_text";      // textLabel_Set, GameTable.cpp:337
		s.fx = 10.0/190.0;  s.fy = 105.0/142.0; s.fcx = 170.0/190.0; s.fcy = 15.0/142.0;
		out.Add(s);

		s.name = "action_icon";   // actionPic, GameTable.cpp:339
		s.fx = 10.0/190.0;  s.fy = 120.0/142.0; s.fcx = 170.0/190.0; s.fcy = 20.0/142.0;
		out.Add(s);

		s.name = "timeout";       // label_Timeout, GameTable.cpp:340
		s.fx = 10.0/190.0;  s.fy = 70.0/142.0;  s.fcx = 60.0/190.0;  s.fcy = 5.0/142.0;
		out.Add(s);
	}
	else if(element_type == "BoardCtrl") {
		// "texas-holdem-legacy-pokertable" profile constants — the profile the
		// PS_6p recorder actually renders board cards with (see the header
		// comment's ROOT-CAUSE NOTE above; NOT the "ps-6p" profile's
		// 641/349/131/122/143, which never matched the real on-screen rect).
		const double board_x = 680.0, board_y = 293.0, board_step = 113.0;
		const double board_w = 94.0, board_h = 133.0;
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
	int lcx, lcy;
	if (slot.aspect_lock) {
		// Mirrors GameTable.cpp's `f_puck = fmin(fx, fy); ... (int)(32 *
		// f_puck)` for both axes. `slot.fcx*ow == 32*fx` and
		// `slot.fcy*oh == 32*fy` exactly (fcx==32/190, fx==ow/190, so
		// fcx*ow == 32*(ow/190) == 32*fx; symmetrically for fy/oh), so
		// `min(slot.fcx*ow, slot.fcy*oh) == min(32*fx, 32*fy) ==
		// 32*fmin(fx,fy)` exactly (min picks the smaller of two already-
		// computed values -- no reordering of any truncation happens here,
		// unlike the base/step case above). A single shared int truncation
		// of that one value, applied to both axes, is therefore bit-for-bit
		// identical to GameTable.cpp's single `(int)(32*f_puck)` (which uses
		// std::fmin over doubles; U++'s min() template gives the same
		// result for this comparison).
		int l = max(1, (int)min(slot.fcx * ow, slot.fcy * oh));
		lcx = lcy = l;
	}
	else {
		lcx = max(1, (int)(slot.fcx * ow));
		lcy = max(1, (int)(slot.fcy * oh));
	}
	return Rect(owner_rect.left + lx, owner_rect.top + ly,
	            owner_rect.left + lx + lcx, owner_rect.top + ly + lcy);
}

} // namespace Upp
