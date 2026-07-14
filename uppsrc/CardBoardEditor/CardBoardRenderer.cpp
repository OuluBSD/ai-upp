#include "CardBoardEditor.h"

NAMESPACE_UPP

static Color EffectiveFill(const CardBoardElement& element)
{
	if(!IsNull(element.style.fill))
		return element.style.fill;
	switch(element.type) {
	case CARD_BOARD_WINDOW: return SColorFace();
	case CARD_BOARD_BOARD: return Color(24, 96, 42);
	case CARD_BOARD_CARD: return White();
	default: return Null;
	}
}

static Font ElementFont(const CardBoardElement& element, int fallback)
{
	Font font = StdFont(element.style.font_height > 0 ? element.style.font_height : fallback);
	if(!element.style.font_face.IsEmpty())
		font.FaceName(element.style.font_face);
	return font;
}

static void DrawCenteredText(Draw& draw, const Rect& rect, const String& text, Font font, Color ink)
{
	Size text_size = GetTextSize(text, font);
	draw.DrawText(rect.left + (rect.GetWidth() - text_size.cx) / 2,
	              rect.top + (rect.GetHeight() - text_size.cy) / 2,
	              text, font, ink);
}

void CardBoardRenderer::Render(Draw& draw, const Rect& area, const CardBoardDocument& document,
                               const CardBoardState&) const
{
	draw.DrawRect(area, SColorPaper());
	for(const CardBoardElement& element : document.elements)
		RenderElement(draw, area, element, 0);
}

void CardBoardRenderer::RenderElement(Draw& draw, const Rect& parent,
                                      const CardBoardElement& element, int depth) const
{
	if(!element.visible)
		return;

	Rect rect = element.rect.Realize(parent);
	Color fill = EffectiveFill(element);
	Color border = element.style.border;
	if(!IsNull(fill)) {
		if(element.style.ellipse)
			draw.DrawEllipse(rect, fill);
		else
			draw.DrawRect(rect, fill);
	}
	if(!IsNull(border)) {
		if(element.style.ellipse)
			draw.DrawEllipse(rect, Null, max(1, element.style.pen), border);
		else {
			draw.DrawRect(rect.left, rect.top, rect.GetWidth(), max(1, element.style.pen), border);
			draw.DrawRect(rect.left, rect.bottom - max(1, element.style.pen), rect.GetWidth(), max(1, element.style.pen), border);
			draw.DrawRect(rect.left, rect.top, max(1, element.style.pen), rect.GetHeight(), border);
			draw.DrawRect(rect.right - max(1, element.style.pen), rect.top, max(1, element.style.pen), rect.GetHeight(), border);
		}
	}

	if(!element.label.IsEmpty()) {
		if(element.type == CARD_BOARD_CHECKBOXES) {
			Vector<String> lines = Split(element.label, '\n');
			Font font = ElementFont(element, 10);
			int y = rect.top + 2;
			for(String line : lines) {
				draw.DrawEllipse(RectC(rect.left + 2, y + 4, 6, 6), Color(45, 45, 45));
				draw.DrawText(rect.left + 14, y, line, font, element.style.text);
				y += font.GetHeight() + 2;
			}
		}
		else
			DrawCenteredText(draw, rect, element.label, ElementFont(element, 12), element.style.text);
	}

	if(element.type == CARD_BOARD_HOLE_CARDS && element.children.IsEmpty()) {
		Rect left = RectC(rect.left, rect.top + rect.GetHeight() / 8, rect.GetWidth() / 2, rect.GetHeight() * 3 / 4);
		Rect right = RectC(rect.left + rect.GetWidth() / 2, rect.top + rect.GetHeight() / 8,
		                   rect.GetWidth() / 2, rect.GetHeight() * 3 / 4);
		draw.DrawRect(left, Color(130, 35, 35));
		draw.DrawRect(right, Color(140, 45, 45));
		draw.DrawRect(left.left, left.top, left.GetWidth(), 1, White());
		draw.DrawRect(left.left, left.bottom - 1, left.GetWidth(), 1, White());
		draw.DrawRect(left.left, left.top, 1, left.GetHeight(), White());
		draw.DrawRect(left.right - 1, left.top, 1, left.GetHeight(), White());
		draw.DrawRect(right.left, right.top, right.GetWidth(), 1, White());
		draw.DrawRect(right.left, right.bottom - 1, right.GetWidth(), 1, White());
		draw.DrawRect(right.left, right.top, 1, right.GetHeight(), White());
		draw.DrawRect(right.right - 1, right.top, 1, right.GetHeight(), White());
	}

	for(const CardBoardElement& child : element.children)
		RenderElement(draw, rect, child, depth + 1);
}

void CardBoardRenderer::DumpRects(String& out, const Rect& area, const CardBoardDocument& document) const
{
	for(const CardBoardElement& element : document.elements)
		DumpElement(out, area, element, 0);
}

void CardBoardRenderer::DumpElement(String& out, const Rect& parent,
                                    const CardBoardElement& element, int depth) const
{
	Rect rect = element.rect.Realize(parent);
	out.Cat(String(' ', depth * 2));
	out.Cat(Format("%s id=%s rect=(%d,%d %d`x%d) label=%s\n",
	               CardBoardElementTypeName(element.type), element.id,
	               rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), element.label));
	for(const CardBoardElement& child : element.children)
		DumpElement(out, rect, child, depth + 1);
}

END_UPP_NAMESPACE
