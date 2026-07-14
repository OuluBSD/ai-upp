#ifndef _CardBoardEditor_CardBoardDocument_h_
#define _CardBoardEditor_CardBoardDocument_h_

struct CardBoardElement : Moveable<CardBoardElement> {
	String id;
	String label;
	String binding;
	String template_id;
	CardBoardElementType type = CARD_BOARD_TEXT;
	CardBoardRect rect;
	CardBoardStyle style;
	Vector<CardBoardElement> children;
	int z = 0;
	bool visible = true;

	CardBoardElement& Add(CardBoardElementType type, const String& id, const String& label,
	                      const CardBoardRect& rect);
};

struct CardBoardState : Moveable<CardBoardState> {
	ValueMap values;
};

struct CardBoardDocument : Moveable<CardBoardDocument> {
	String provider_id;
	String game_family;
	Size design_size = Size(610, 438);
	Vector<CardBoardElement> elements;

	void Clear();
	void CopyFrom(const CardBoardDocument& source);
	void MakePokerSample();
	String Validate() const;
	void DumpTree(String& out) const;
	void DumpRects(String& out, Size canvas_size) const;
};

CardBoardElement MakeCardBoardElement(CardBoardElementType type, const String& id,
                                      const String& label, const CardBoardRect& rect);

#endif
