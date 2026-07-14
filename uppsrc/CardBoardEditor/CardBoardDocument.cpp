#include "CardBoardEditor.h"

NAMESPACE_UPP

static CardBoardRect Rel(double x, double y, double w, double h)
{
	CardBoardRect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	r.relative = true;
	return r;
}

static String ColorToJsonString(Color color)
{
	if(IsNull(color))
		return "null";
	return Format("#%02x%02x%02x", color.GetR(), color.GetG(), color.GetB());
}

static Color JsonStringToColor(const String& text)
{
	if(text.IsEmpty() || text == "null")
		return Null;
	if(text.GetCount() == 7 && text[0] == '#') {
		int r = ScanInt("0x" + text.Mid(1, 2));
		int g = ScanInt("0x" + text.Mid(3, 2));
		int b = ScanInt("0x" + text.Mid(5, 2));
		return Color(r, g, b);
	}
	return Null;
}

static void JsonizeColor(JsonIO& json, const char *key, Color& color)
{
	String value = ColorToJsonString(color);
	json(key, value);
	if(json.IsLoading())
		color = JsonStringToColor(value);
}

Rect CardBoardRect::Realize(const Rect& parent) const
{
	if(!relative)
		return RectC(parent.left + fround(x), parent.top + fround(y), fround(w), fround(h));
	return RectC(parent.left + fround(parent.GetWidth() * x),
	             parent.top + fround(parent.GetHeight() * y),
	             fround(parent.GetWidth() * w),
	             fround(parent.GetHeight() * h));
}

String CardBoardRect::ToString() const
{
	return relative ? Format("%.4f %.4f %.4f %.4f rel", x, y, w, h)
	                : Format("%.1f %.1f %.1f %.1f px", x, y, w, h);
}

void CardBoardRect::Jsonize(JsonIO& json)
{
	json
		("x", x)
		("y", y)
		("w", w)
		("h", h)
		("relative", relative);
}

void CardBoardStyle::Jsonize(JsonIO& json)
{
	JsonizeColor(json, "fill", fill);
	JsonizeColor(json, "border", border);
	JsonizeColor(json, "text", text);
	json
		("asset", asset)
		("font_face", font_face)
		("align", align)
		("outline", outline)
		("shadow", shadow)
		("font_height", font_height)
		("pen", pen)
		("opacity", opacity)
		("rotation", rotation)
		("fan_start", fan_start)
		("fan_end", fan_end)
		("ellipse", ellipse)
		("rounded", rounded);
}

String CardBoardElementTypeName(CardBoardElementType type)
{
	switch(type) {
	case CARD_BOARD_WINDOW: return "Window";
	case CARD_BOARD_BOARD: return "Board";
	case CARD_BOARD_SEAT: return "Seat";
	case CARD_BOARD_PLAYER_PANEL: return "PlayerPanel";
	case CARD_BOARD_AVATAR: return "Avatar";
	case CARD_BOARD_AVATAR_RING: return "AvatarRing";
	case CARD_BOARD_HOLE_CARDS: return "HoleCards";
	case CARD_BOARD_NAMEPLATE: return "Nameplate";
	case CARD_BOARD_ACTION_BADGE: return "ActionBadge";
	case CARD_BOARD_BALANCE_TEXT: return "BalanceText";
	case CARD_BOARD_FLAG: return "Flag";
	case CARD_BOARD_PLAYER_NUMBER: return "PlayerNumber";
	case CARD_BOARD_DEALER_BUTTON: return "DealerButton";
	case CARD_BOARD_POT_LABEL: return "PotLabel";
	case CARD_BOARD_BOARD_CARDS: return "BoardCards";
	case CARD_BOARD_CHIP_STACK: return "ChipStack";
	case CARD_BOARD_ACTION_BUTTONS: return "ActionButtons";
	case CARD_BOARD_CHECKBOXES: return "Checkboxes";
	case CARD_BOARD_TOP_TOOLBAR: return "TopToolbar";
	case CARD_BOARD_TEXT: return "Text";
	case CARD_BOARD_BUTTON: return "Button";
	case CARD_BOARD_CARD: return "Card";
	default: return "Unknown";
	}
}

CardBoardElementType CardBoardElementTypeFromName(const String& name)
{
	for(int i = CARD_BOARD_WINDOW; i <= CARD_BOARD_CARD; i++) {
		CardBoardElementType type = (CardBoardElementType)i;
		if(CardBoardElementTypeName(type) == name)
			return type;
	}
	return CARD_BOARD_TEXT;
}

CardBoardElement MakeCardBoardElement(CardBoardElementType type, const String& id,
                                      const String& label, const CardBoardRect& rect)
{
	CardBoardElement element;
	element.type = type;
	element.id = id;
	element.label = label;
	element.rect = rect;
	return element;
}

CardBoardElement& CardBoardElement::Add(CardBoardElementType type, const String& id,
                                        const String& label, const CardBoardRect& rect)
{
	return children.Add(MakeCardBoardElement(type, id, label, rect));
}

void CardBoardElement::Jsonize(JsonIO& json)
{
	String type_name = CardBoardElementTypeName(type);
	json
		("id", id)
		("type", type_name)
		("label", label)
		("binding", binding)
		("template_id", template_id)
		("rect", rect)
		("style", style)
		("children", children)
		("z", z)
		("visible", visible);
	if(json.IsLoading())
		type = CardBoardElementTypeFromName(type_name);
}

void CardBoardDocument::Clear()
{
	provider_id.Clear();
	game_family.Clear();
	format_version = "1";
	asset_root.Clear();
	elements.Clear();
}

static void CopyStyle(CardBoardStyle& target, const CardBoardStyle& source)
{
	target.fill = source.fill;
	target.border = source.border;
	target.text = source.text;
	target.asset = source.asset;
	target.font_face = source.font_face;
	target.align = source.align;
	target.outline = source.outline;
	target.shadow = source.shadow;
	target.font_height = source.font_height;
	target.pen = source.pen;
	target.opacity = source.opacity;
	target.rotation = source.rotation;
	target.fan_start = source.fan_start;
	target.fan_end = source.fan_end;
	target.ellipse = source.ellipse;
	target.rounded = source.rounded;
}

static void CopyRect(CardBoardRect& target, const CardBoardRect& source)
{
	target.x = source.x;
	target.y = source.y;
	target.w = source.w;
	target.h = source.h;
	target.relative = source.relative;
}

static void CopyElement(CardBoardElement& target, const CardBoardElement& source)
{
	target.id = source.id;
	target.label = source.label;
	target.binding = source.binding;
	target.template_id = source.template_id;
	target.type = source.type;
	CopyRect(target.rect, source.rect);
	CopyStyle(target.style, source.style);
	target.z = source.z;
	target.visible = source.visible;
	target.children.Clear();
	for(const CardBoardElement& child : source.children)
		CopyElement(target.children.Add(), child);
}

void CardBoardDocument::CopyFrom(const CardBoardDocument& source)
{
	provider_id = source.provider_id;
	game_family = source.game_family;
	format_version = source.format_version;
	asset_root = source.asset_root;
	design_size = source.design_size;
	elements.Clear();
	for(const CardBoardElement& element : source.elements)
		CopyElement(elements.Add(), element);
}

static void StylePanel(CardBoardElement& element, Color fill, Color border, Color text = White())
{
	element.style.fill = fill;
	element.style.border = border;
	element.style.text = text;
}

static void AddSeat(CardBoardElement& window, int seat, const String& name, const String& stack,
                    const CardBoardRect& rect, Color panel_border, bool hero = false,
                    const String& action = String())
{
	CardBoardElement& s = window.Add(CARD_BOARD_SEAT, Format("seat%d", seat), name, rect);
	s.template_id = hero ? "hero-seat" : "standard-seat";

	CardBoardElement& avatar = s.Add(CARD_BOARD_AVATAR, Format("seat%d.avatar", seat), "",
	                                 Rel(0.28, 0.00, 0.44, 0.44));
	StylePanel(avatar, Color(75, 75, 75), Color(215, 215, 215));
	avatar.style.ellipse = true;

	CardBoardElement& ring = s.Add(CARD_BOARD_AVATAR_RING, Format("seat%d.ring", seat), "",
	                               Rel(0.25, -0.03, 0.50, 0.50));
	StylePanel(ring, Null, panel_border);
	ring.style.ellipse = true;
	ring.style.pen = 3;

	CardBoardElement& cards = s.Add(CARD_BOARD_HOLE_CARDS, Format("seat%d.cards", seat), "",
	                                Rel(0.27, 0.02, 0.46, 0.28));
	StylePanel(cards, Color(150, 45, 38), White());
	cards.style.rotation = hero ? 4.0 : 0.0;
	cards.style.fan_start = -4.0;
	cards.style.fan_end = 4.0;
	cards.z = 20;

	CardBoardElement& panel = s.Add(CARD_BOARD_PLAYER_PANEL, Format("seat%d.panel", seat), "",
	                                Rel(0.12, 0.38, 0.76, 0.46));
	StylePanel(panel, Color(18, 22, 28), panel_border);
	panel.style.rounded = true;
	panel.z = 10;

	if(!action.IsEmpty()) {
		CardBoardElement& badge = s.Add(CARD_BOARD_ACTION_BADGE, Format("seat%d.action", seat), action,
		                                Rel(0.25, 0.32, 0.50, 0.16));
		StylePanel(badge, Color(45, 32, 8), Color(230, 185, 35), Color(255, 225, 70));
		badge.style.rounded = true;
		badge.style.font_height = 9;
		badge.binding = Format("seat%d_action", seat);
		badge.z = 30;
	}

	CardBoardElement& number = s.Add(CARD_BOARD_PLAYER_NUMBER, Format("seat%d.number", seat),
	                                 AsString(20 + seat), Rel(0.07, 0.43, 0.18, 0.18));
	StylePanel(number, Color(75, 30, 30), Color(130, 120, 110));
	number.style.font_height = 9;

	CardBoardElement& flag = s.Add(CARD_BOARD_FLAG, Format("seat%d.flag", seat), "FI",
	                               Rel(0.73, 0.43, 0.20, 0.14));
	StylePanel(flag, Color(230, 230, 245), Color(30, 60, 160), Blue());
	flag.style.font_height = 8;

	CardBoardElement& label = s.Add(CARD_BOARD_NAMEPLATE, Format("seat%d.name", seat), name,
	                                Rel(0.18, 0.46, 0.64, 0.17));
	label.style.text = White();
	label.style.font_height = hero ? 13 : 10;

	CardBoardElement& balance = s.Add(CARD_BOARD_BALANCE_TEXT, Format("seat%d.balance", seat), stack,
	                                  Rel(0.18, 0.64, 0.64, 0.18));
	balance.style.text = Color(90, 200, 255);
	balance.style.font_height = hero ? 14 : 11;
}

void CardBoardDocument::MakePokerSample()
{
	Clear();
	provider_id = "Original";
	game_family = "Poker";
	format_version = "1";
	asset_root = "share/cardboard/original";
	design_size = Size(610, 438);

	CardBoardElement& window = elements.Add(MakeCardBoardElement(CARD_BOARD_WINDOW, "window",
	                                                             "Original poker table",
	                                                             Rel(0, 0, 1, 1)));
	StylePanel(window, Color(18, 18, 20), Null);

	CardBoardElement& toolbar = window.Add(CARD_BOARD_TOP_TOOLBAR, "top.toolbar",
	                                       "NLH Original 02 - $5 / $10", Rel(0, 0, 1, 0.075));
	StylePanel(toolbar, Color(23, 24, 27), Color(45, 45, 48), Color(180, 180, 180));
	toolbar.style.font_height = 13;

	CardBoardElement& leaderboard = window.Add(CARD_BOARD_BUTTON, "top.leaderboard",
	                                           "DAILY $50,000 LEADERBOARD", Rel(0.01, 0.075, 0.27, 0.055));
	StylePanel(leaderboard, Color(28, 70, 38), Color(110, 135, 115), Color(230, 245, 160));
	leaderboard.style.rounded = true;
	leaderboard.style.font_height = 8;

	CardBoardElement& top_buttons = window.Add(CARD_BOARD_TOP_TOOLBAR, "top.buttons", "",
	                                           Rel(0.87, 0.075, 0.12, 0.055));
	StylePanel(top_buttons, Null, Null);
	for(int i = 0; i < 3; i++) {
		CardBoardElement& button = top_buttons.Add(CARD_BOARD_BUTTON, Format("top.button%d", i + 1),
		                                           i == 0 ? "⌨" : i == 1 ? "+" : "♠",
		                                           Rel(i * 0.34, 0.02, 0.28, 0.84));
		StylePanel(button, Color(85, 150, 60), Color(150, 220, 100), White());
		button.style.rounded = true;
		button.style.font_height = 12;
	}

	CardBoardElement& board = window.Add(CARD_BOARD_BOARD, "board", "", Rel(0.07, 0.20, 0.86, 0.57));
	StylePanel(board, Color(19, 112, 43), Color(28, 30, 28));
	board.style.rounded = true;

	CardBoardElement& pot = board.Add(CARD_BOARD_POT_LABEL, "board.pot", "Total Pot: $155",
	                                  Rel(0.39, 0.11, 0.22, 0.075));
	StylePanel(pot, Color(42, 78, 28), Null, Color(255, 214, 71));
	pot.style.rounded = true;
	pot.style.font_height = 12;
	pot.binding = "pot_label";
	pot.z = 20;

	CardBoardElement& cards = board.Add(CARD_BOARD_BOARD_CARDS, "board.cards", "",
	                                    Rel(0.31, 0.28, 0.36, 0.30));
	StylePanel(cards, Null, Null);
	const char *flop[] = { "K", "5", "3" };
	for(int i = 0; i < 5; i++) {
		CardBoardElement& card = cards.Add(CARD_BOARD_CARD, Format("board.card%d", i + 1),
		                                   i < 3 ? flop[i] : "",
		                                   Rel(i * 0.205, 0, 0.18, 1));
		StylePanel(card, i == 1 ? Color(35, 38, 43) : i == 2 ? Color(150, 28, 28) : Color(24, 145, 53),
		           Color(210, 210, 210));
		card.style.font_height = 32;
		card.binding = Format("board_card%d", i + 1);
	}

	CardBoardElement& chips = board.Add(CARD_BOARD_CHIP_STACK, "board.chips", "$155",
	                                    Rel(0.46, 0.56, 0.08, 0.12));
	StylePanel(chips, Color(210, 210, 210), Color(80, 80, 80), White());
	chips.style.font_height = 10;
	chips.binding = "pot_amount";
	chips.z = 30;

	AddSeat(window, 1, "Misha Inner", "$930", Rel(0.40, 0.70, 0.20, 0.25), Color(245, 120, 70), true, "LIVE");
	AddSeat(window, 2, "LlenoXX", "$2,934", Rel(0.42, 0.09, 0.16, 0.20), Color(130, 255, 70));
	AddSeat(window, 3, "cascconss", "$1,048", Rel(0.76, 0.20, 0.17, 0.22), Color(130, 70, 210));
	AddSeat(window, 4, "H Pesonen", "$1,190", Rel(0.78, 0.61, 0.17, 0.23), Color(130, 70, 210));
	AddSeat(window, 5, "Denis Chernous", "$1,013", Rel(0.05, 0.61, 0.18, 0.23), Color(130, 70, 210));
	AddSeat(window, 6, "Majiaz Vogrinec", "$1,974", Rel(0.08, 0.20, 0.17, 0.22), Color(130, 70, 210));

	CardBoardElement& dealer = window.Add(CARD_BOARD_DEALER_BUTTON, "dealer.button", "D",
	                                      Rel(0.44, 0.31, 0.025, 0.035));
	StylePanel(dealer, Color(235, 205, 38), Color(80, 75, 10), Color(70, 55, 5));
	dealer.style.ellipse = true;
	dealer.style.font_height = 9;

	CardBoardElement& checks = window.Add(CARD_BOARD_CHECKBOXES, "bottom.checks",
	                                      "Move Table\nStraddle\nGlobal Cash Game Sit-out",
	                                      Rel(0.025, 0.83, 0.26, 0.12));
	checks.style.text = Color(185, 185, 185);
	checks.style.font_height = 10;

	CardBoardElement& rank = window.Add(CARD_BOARD_TEXT, "bottom.rank", "43",
	                                    Rel(0.22, 0.925, 0.09, 0.075));
	StylePanel(rank, Color(50, 53, 58), Null, Color(130, 255, 190));
	rank.style.font_height = 20;

	CardBoardElement& chat = window.Add(CARD_BOARD_BUTTON, "bottom.chat", "●",
	                                    Rel(0.32, 0.93, 0.04, 0.05));
	StylePanel(chat, Color(205, 210, 218), Null, Color(80, 85, 95));
	chat.style.ellipse = true;
	chat.style.font_height = 12;

	CardBoardElement& emoji = window.Add(CARD_BOARD_BUTTON, "bottom.emoji", "☺",
	                                     Rel(0.37, 0.925, 0.05, 0.06));
	StylePanel(emoji, Color(230, 170, 50), Null, Color(95, 55, 10));
	emoji.style.ellipse = true;
	emoji.style.font_height = 14;

	CardBoardElement& buttons = window.Add(CARD_BOARD_ACTION_BUTTONS, "bottom.actions",
	                                       "", Rel(0.61, 0.84, 0.27, 0.11));
	StylePanel(buttons, Color(20, 20, 22), Color(45, 45, 50), Color(225, 225, 225));
	buttons.style.font_height = 11;
	for(int i = 0; i < 3; i++) {
		CardBoardElement& button = buttons.Add(CARD_BOARD_BUTTON, Format("bottom.action%d", i + 1),
		                                       i == 0 ? "Fold" : i == 1 ? "Call $10" : "Bet",
		                                       Rel(i * 0.34, 0.10, 0.30, 0.70));
		StylePanel(button, Color(32, 18, 20), Color(70, 35, 38), Color(230, 210, 210));
		button.style.rounded = true;
		button.style.font_height = 10;
	}
}

CardBoardState CardBoardDocument::MakePokerSampleState() const
{
	CardBoardState state;
	state.values.Add("pot_label", "Total Pot: $155");
	state.values.Add("pot_amount", "$155");
	state.values.Add("board_card1", "K");
	state.values.Add("board_card2", "5");
	state.values.Add("board_card3", "3");
	state.values.Add("board_card4", "");
	state.values.Add("board_card5", "");
	state.values.Add("seat1_action", "LIVE");
	return state;
}

String CardBoardDocument::Validate() const
{
	if(provider_id.IsEmpty())
		return "provider_id is empty";
	if(game_family.IsEmpty())
		return "game_family is empty";
	if(design_size.cx <= 0 || design_size.cy <= 0)
		return "design_size is invalid";
	if(elements.IsEmpty())
		return "document has no elements";
	return String();
}

static void DumpElementTree(String& out, const CardBoardElement& element, int depth)
{
	out.Cat(String(' ', depth * 2));
	out.Cat(Format("%s id=%s label=%s rect=%s\n",
	               CardBoardElementTypeName(element.type), element.id, element.label,
	               element.rect.ToString()));
	for(const CardBoardElement& child : element.children)
		DumpElementTree(out, child, depth + 1);
}

void CardBoardDocument::DumpTree(String& out) const
{
	out.Cat(Format("CardBoardDocument provider=%s game=%s design=%d`x%d\n",
	               provider_id, game_family, design_size.cx, design_size.cy));
	for(const CardBoardElement& element : elements)
		DumpElementTree(out, element, 0);
}

void CardBoardDocument::DumpRects(String& out, Size canvas_size) const
{
	CardBoardRenderer renderer;
	renderer.DumpRects(out, RectC(0, 0, canvas_size.cx, canvas_size.cy), *this);
}

void CardBoardDocument::RenderReport(String& out, Size canvas_size, const CardBoardState& state) const
{
	CardBoardRenderer renderer;
	renderer.RenderReport(out, RectC(0, 0, canvas_size.cx, canvas_size.cy), *this, state);
}

String CardBoardDocument::StoreJson() const
{
	CardBoardDocument copy;
	copy.CopyFrom(*this);
	return StoreAsJson(copy, true);
}

bool CardBoardDocument::LoadJson(const String& json, String& error)
{
	CardBoardDocument loaded;
	if(!LoadFromJson(loaded, json)) {
		error = "failed to parse CardBoard JSON";
		return false;
	}
	error = loaded.Validate();
	if(!error.IsEmpty())
		return false;
	CopyFrom(loaded);
	return true;
}

void Jsonize(JsonIO& json, Size& size)
{
	json
		("cx", size.cx)
		("cy", size.cy);
}

void Jsonize(JsonIO& json, CardBoardDocument& document)
{
	json
		("format_version", document.format_version)
		("provider_id", document.provider_id)
		("game_family", document.game_family)
		("asset_root", document.asset_root)
		("design_size", document.design_size)
		("elements", document.elements);
}

END_UPP_NAMESPACE
