#ifndef _CardBoardEditor_CardBoardRenderer_h_
#define _CardBoardEditor_CardBoardRenderer_h_

class CardBoardRenderer {
public:
	void Render(Draw& draw, const Rect& area, const CardBoardDocument& document,
	            const CardBoardState& state = CardBoardState()) const;
	void DumpRects(String& out, const Rect& area, const CardBoardDocument& document) const;
	void RenderReport(String& out, const Rect& area, const CardBoardDocument& document,
	                  const CardBoardState& state = CardBoardState()) const;

private:
	void RenderElement(Draw& draw, const Rect& parent, const CardBoardElement& element,
	                   const CardBoardState& state, int depth) const;
	void RenderPrimitive(Draw& draw, const Rect& rect, const CardBoardElement& element,
	                     const String& label) const;
	void DumpElement(String& out, const Rect& parent, const CardBoardElement& element,
	                 int depth) const;
	void ReportElement(String& out, const Rect& parent, const CardBoardElement& element,
	                   const CardBoardState& state, int depth) const;
	String ResolveLabel(const CardBoardElement& element, const CardBoardState& state) const;
	String PrimitiveName(const CardBoardElement& element) const;
	void ForEachChildByZ(const Vector<CardBoardElement>& children,
	                     Function<void (const CardBoardElement&)> callback) const;
};

#endif
