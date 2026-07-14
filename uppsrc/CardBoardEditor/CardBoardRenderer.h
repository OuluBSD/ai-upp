#ifndef _CardBoardEditor_CardBoardRenderer_h_
#define _CardBoardEditor_CardBoardRenderer_h_

class CardBoardRenderer {
public:
	void Render(Draw& draw, const Rect& area, const CardBoardDocument& document,
	            const CardBoardState& state = CardBoardState()) const;
	void DumpRects(String& out, const Rect& area, const CardBoardDocument& document) const;

private:
	void RenderElement(Draw& draw, const Rect& parent, const CardBoardElement& element,
	                   int depth) const;
	void DumpElement(String& out, const Rect& parent, const CardBoardElement& element,
	                 int depth) const;
};

#endif
