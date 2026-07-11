#include "TexasHoldemLogicState.h"
#include <EditorCommon/Tools.h>
#include <CardRender/CardRender.h>

NAMESPACE_UPP

void TexasHoldemLogicPlayerState::Jsonize(JsonIO& jio)
{
	jio
		("seat", seat)
		("uid_known", uid_known)("uid", uid)
		("name_known", name_known)("name", name)
		("hero_known", hero_known)("hero", hero)
		("active_known", active_known)("active", active)
		("stack_known", stack_known)("stack", stack)
		("bet_known", bet_known)("bet", bet)
		("action_known", action_known)("action", action)
		("button_known", button_known)("button", button)
		("hole_cards_known", hole_cards_known)("hole_cards", hole_cards)
	;
}

void TexasHoldemLogicState::Jsonize(JsonIO& jio)
{
	jio
		("schema", schema)
		("session_id_known", session_id_known)("session_id", session_id)
		("frame_id", frame_id)
		("render_step_known", render_step_known)("render_step", render_step)
		("timestamp_ms_known", timestamp_ms_known)("timestamp_ms", timestamp_ms)
		("provider_known", provider_known)("provider", provider)
		("table_size_known", table_size_known)
			("table_width", table_width)("table_height", table_height)
		("seed_known", seed_known)("seed", seed)
		("game_id_known", game_id_known)("game_id", game_id)
		("hand_id_known", hand_id_known)("hand_id", hand_id)
		("street_known", street_known)("street", street)
		("turn_uid_known", turn_uid_known)("turn_uid", turn_uid)
		("pot_known", pot_known)("pot", pot)
		("board_cards_known", board_cards_known)("board_cards", board_cards)
		("players_known", players_known)("players", players)
		("dealer_seat_known", dealer_seat_known)("dealer_seat", dealer_seat)
	;
}

// ---------------------------------------------------------------------------
// M05-03 (task 0121): see TexasHoldemLogicState.h for the full doc comment.
// Body is a verbatim merge of GameTable::LoadTheme's puckDealer/SB/BB themed-
// file-load tier, refreshGroupbox's legacy gfx/dealer.png-style fallback
// tier, and GameTable::GetPuckImage's procedural-fallback tier (all three in
// game/TexasHoldem/GameTable.cpp) - kept byte-for-byte identical to the
// drawing code so GameTable::GetPuckImage (which now just forwards to this
// function + applies its existing per-instance cache) renders pixel-
// identical output to before this extraction.
Image TexasHoldemGetPuckReferenceImage(int role, const String& theme)
{
	String dataDir = Tools::GetDataDir();

	static const char* themedNames[3] = { "dealerPuck.png", "smallblindPuck.png", "bigblindPuck.png" };
	static const char* legacyNames[3]  = { "gfx/dealer.png", "gfx/small_blind.png", "gfx/big_blind.png" };

	Image img;
	if(role >= 0 && role < 3) {
		// Tier 1: themed dir (GameTable::LoadTheme's puckDealer/puckSB/puckBB).
		String themedDir = AppendFileName(dataDir, "gfx/gui/table/" + theme);
		img = StreamRaster::LoadFileAny(AppendFileName(themedDir, themedNames[role]));
		if(img.IsEmpty()) {
			// Tier 2: legacy hardcoded path (GameTable::refreshGroupbox's
			// gfx/dealer.png / gfx/small_blind.png / gfx/big_blind.png).
			img = StreamRaster::LoadFileAny(AppendFileName(dataDir, legacyNames[role]));
		}
	}
	if(!img.IsEmpty())
		return img;

	// Tier 3: procedural fallback (GameTable::GetPuckImage's drawing, task
	// 0120). Enlarged to 40x40 relative to textLabel_Button's 32x32 base rect
	// (PlayerCtrl::Layout()) for legibility of the two-letter SB/BB labels --
	// ScaledImageCtrl centers/letterboxes it inside the actual rect at any
	// real on-screen scale.
	const int sz = 40;
	ImageDraw iw(sz, sz);
	iw.Alpha().DrawRect(0, 0, sz, sz, RGBAZero());

	Color fill, ink;
	String label;
	switch(role) {
	case 0: fill = Color(250, 240, 210); ink = Black(); label = "D";  break; // dealer: white/cream
	case 1: fill = Color(120, 190, 250); ink = Black(); label = "SB"; break; // small blind: light blue
	case 2: fill = Color(150, 20, 30);   ink = White(); label = "BB"; break; // big blind: darker red
	default: fill = White();             ink = Black(); label = "";  break;
	}
	// Mark the disc's area opaque on the SEPARATE alpha plane first -- see
	// GameTable::DrawChipDiscs's comment for why this is required in this
	// codebase's Win32 ImageDraw backend (RGB-only draws never implicitly
	// touch the alpha plane; skipping this leaves the whole image at
	// alpha==0, i.e. invisible, despite correct-looking RGB content).
	iw.Alpha().DrawEllipse(0, 0, sz, sz, White(), 2, White());
	iw.DrawEllipse(0, 0, sz, sz, fill, 2, Black());
	if(!label.IsEmpty()) {
		Font f = SansSerif(label.GetCount() > 1 ? 13 : 16).Bold();
		Size tsz = GetTextSize(label, f);
		iw.DrawText((sz - tsz.cx) / 2, (sz - tsz.cy) / 2, label, f, ink);
	}

	return Image(iw);
}

// ---------------------------------------------------------------------------
// M05-08 (task 0126): see TexasHoldemLogicState.h for the full doc comment.
// Filename encoding copied verbatim from GameTable::GetCardImage
// (game/TexasHoldem/GameTable.cpp:246-265) - suits/ranks arrays and the
// card = suit_index*13 + rank_index formula must stay bit-for-bit identical
// to that function or this reference no longer represents what GetCardImage
// actually draws for the same `card` value.
Image TexasHoldemGetCardReferenceImage(int card, Size target_size, const String& theme)
{
	if(card < 0 || card >= 52)
		return Image();

	static const char* suits[] = { "clubs", "diamonds", "hearts", "spades" };
	static const char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
	String filename = String(suits[card / 13]) + "_" + ranks[card % 13] + ".png";

	return LoadCardArt(filename, target_size, theme);
}

// M05-08 (task 0126): see TexasHoldemLogicState.h for the full doc comment.
// Holder-tier selection copied verbatim from GameTable::PaintBoard
// (game/TexasHoldem/GameTable.cpp:1280-1283); themed-dir load path copied
// verbatim from GameTable::LoadTheme's cardHolderFlop/Turn/River assignment
// (GameTable.cpp:1211-1213); procedural fallback color copied verbatim from
// PaintBoard's own `w.DrawRect(x, y, w1, h1, Color(0, 80, 0))` fallback
// (GameTable.cpp:1287).
Image TexasHoldemGetBoardHolderReferenceImage(int card_index, Size target_size, const String& theme)
{
	static const char* holderNames[3] = { "cardholder_flop.png", "cardholder_turn.png", "cardholder_river.png" };
	int tier = (card_index < 3) ? 0 : (card_index == 3 ? 1 : 2);

	String dataDir = Tools::GetDataDir();
	String themedDir = AppendFileName(dataDir, "gfx/gui/table/" + theme);
	Image img = StreamRaster::LoadFileAny(AppendFileName(themedDir, holderNames[tier]));
	if(!img.IsEmpty()) {
		if(target_size.cx > 0 && target_size.cy > 0 && img.GetSize() != target_size)
			img = Rescale(img, target_size);
		return img;
	}

	// Procedural fallback: byte-for-byte the same solid fill PaintBoard
	// itself draws when the themed holder asset is missing (see this
	// function's doc comment). No alpha-plane subtlety to worry about here
	// (unlike the puck fallback above's rounded disc) - this is a plain
	// opaque rect, and callers only ever compare RGB channels
	// (VsmMeanAbsPixelDiff, reference/VisualStateLogicCompare/main.cpp) -
	// but the alpha plane is still filled in for a well-formed, fully-opaque
	// Image, same defensive style as the puck fallback.
	int cx = max(1, target_size.cx), cy = max(1, target_size.cy);
	ImageDraw iw(cx, cy);
	iw.Alpha().DrawRect(0, 0, cx, cy, White());
	iw.DrawRect(0, 0, cx, cy, Color(0, 80, 0));
	return Image(iw);
}

END_UPP_NAMESPACE
