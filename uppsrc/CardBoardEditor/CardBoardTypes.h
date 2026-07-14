#ifndef _CardBoardEditor_CardBoardTypes_h_
#define _CardBoardEditor_CardBoardTypes_h_

enum CardBoardElementType {
	CARD_BOARD_WINDOW,
	CARD_BOARD_BOARD,
	CARD_BOARD_SEAT,
	CARD_BOARD_PLAYER_PANEL,
	CARD_BOARD_AVATAR,
	CARD_BOARD_AVATAR_RING,
	CARD_BOARD_HOLE_CARDS,
	CARD_BOARD_NAMEPLATE,
	CARD_BOARD_ACTION_BADGE,
	CARD_BOARD_BALANCE_TEXT,
	CARD_BOARD_FLAG,
	CARD_BOARD_PLAYER_NUMBER,
	CARD_BOARD_DEALER_BUTTON,
	CARD_BOARD_POT_LABEL,
	CARD_BOARD_BOARD_CARDS,
	CARD_BOARD_CHIP_STACK,
	CARD_BOARD_ACTION_BUTTONS,
	CARD_BOARD_CHECKBOXES,
	CARD_BOARD_TOP_TOOLBAR,
	CARD_BOARD_TEXT,
	CARD_BOARD_BUTTON,
	CARD_BOARD_CARD
};

struct CardBoardRect : Moveable<CardBoardRect> {
	double x = 0;
	double y = 0;
	double w = 1;
	double h = 1;
	bool relative = true;

	Rect Realize(const Rect& parent) const;
	String ToString() const;
};

struct CardBoardStyle : Moveable<CardBoardStyle> {
	Color fill = Null;
	Color border = Null;
	Color text = White();
	String font_face;
	int font_height = 14;
	int pen = 1;
	bool ellipse = false;
	bool rounded = false;
};

String CardBoardElementTypeName(CardBoardElementType type);

#endif
