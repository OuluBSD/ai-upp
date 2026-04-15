#include "Ctrl.h"

namespace Upp {

namespace Node {

// Draw a filled+outlined ellipse approximated as a 32-point polygon.
static void DrawEllipse(Draw& w, Rect r, Color fill, Color line, int lw)
{
	if(r.Width() <= 0 || r.Height() <= 0) return;
	double cx = r.left + r.Width()  / 2.0;
	double cy = r.top  + r.Height() / 2.0;
	double rx = r.Width()  / 2.0;
	double ry = r.Height() / 2.0;
	const int N = 32;
	Vector<Point> pts;
	pts.Reserve(N + 1);
	for(int i = 0; i <= N; i++) {
		double a = 2.0 * M_PI * i / N;
		pts.Add(Point((int)(cx + rx * cos(a)), (int)(cy + ry * sin(a))));
	}
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, line);
	else if(!IsNull(line))
		w.DrawPolyline(pts, lw, line);
}

// Draw a diamond (rotated square) fitting inside rect r.
static void DrawDiamond(Draw& w, Rect r, Color fill, Color line, int lw)
{
	int cx = r.left + r.Width()  / 2;
	int cy = r.top  + r.Height() / 2;
	Vector<Point> pts = {
		Point(cx,         r.top),
		Point(r.right - 1, cy),
		Point(cx,         r.bottom - 1),
		Point(r.left,     cy),
		Point(cx,         r.top),
	};
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, line);
	else if(!IsNull(line))
		w.DrawPolyline(pts, lw, line);
}

// Draw a filled arrowhead pointing from `from` toward `to`.
static void DrawArrow(Draw& w, Point from, Point to, int size, Color clr)
{
	double dx = to.x - from.x;
	double dy = to.y - from.y;
	double len = sqrt(dx * dx + dy * dy);
	if(len < 1) return;
	dx /= len; dy /= len;

	double px = -dy, py = dx; // perpendicular

	Point tip = to;
	Point b1((int)(tip.x - dx * size + px * size * 0.4),
	         (int)(tip.y - dy * size + py * size * 0.4));
	Point b2((int)(tip.x - dx * size - px * size * 0.4),
	         (int)(tip.y - dy * size - py * size * 0.4));

	Vector<Point> arrow = { tip, b1, b2, tip };
	w.DrawPolygon(arrow, clr, 1, clr);
}

// Draw text with optional multi-line support (\n splits), centered in rect r.
static void DrawCenteredText(Draw& w, Rect r, const String& text, Font f, Color clr)
{
	// Split on \n
	Vector<String> lines = Split(text, '\n');
	if(lines.IsEmpty()) return;

	int line_h = f.GetHeight() + 2;
	int total_h = line_h * lines.GetCount();
	int y = r.top + max(0, (r.Height() - total_h) / 2);

	for(const String& line : lines) {
		int tw = GetTextSize(line, f).cx;
		int x  = r.left + max(0, (r.Width() - tw) / 2);
		w.DrawText(x, y, line, f, clr);
		y += line_h;
	}
}

void PaintScene(Draw& w, const Scene& scene, const Viewport& vp, const EditorState* es)
{
	for(const auto& item : scene.items) {
		bool selected = es && es->IsSelected(item.entity_id);
		// For pin/label/widget items, check if parent node is selected
		bool parent_selected = false;
		if(!selected && es) {
			int sep = item.entity_id.Find(':');
			if(sep >= 0)
				parent_selected = es->IsSelected(item.entity_id.Left(sep));
		}
		bool hovered = es && es->hovered_entity == item.entity_id;

		Color fill  = item.fill_clr;
		Color line  = item.line_clr;
		int   width = item.line_width;

		if(selected || parent_selected) {
			line  = LtRed();
			width = item.line_width + 1;
		}
		if(hovered) {
			line  = Cyan();
			width = item.line_width + 1;
		}

		if(item.type == SceneItem::NODE || item.type == SceneItem::GROUP) {
			Rect r = vp.WorldToView(item.rect);
			switch(item.shape) {
			case 1: // Ellipse
				DrawEllipse(w, r, fill, line, width);
				break;
			case 2: // Diamond
				DrawDiamond(w, r, fill, line, width);
				break;
			default: // Rect (0)
				if(!IsNull(fill))  w.DrawRect(r, fill);
				if(!IsNull(line))  DrawFrame(w, r, line);
				break;
			}
		}
		else if(item.type == SceneItem::PIN) {
			Rect r = vp.WorldToView(item.rect);
			if(!IsNull(fill))  w.DrawRect(r, fill);
			if(!IsNull(line))  DrawFrame(w, r, line);
		}
		else if(item.type == SceneItem::EDGE) {
			if(item.path.GetCount() > 1) {
				Color edge_clr = (selected || hovered) ? line : item.line_clr;
				int   edge_w   = (selected || hovered) ? width : item.line_width;

				Vector<Point> pts;
				pts.Reserve(item.path.GetCount());
				for(const auto& p : item.path)
					pts.Add(vp.WorldToView(p));

				w.DrawPolyline(pts, edge_w, edge_clr);

				// Arrowhead at target end
				if(item.directed && pts.GetCount() >= 2) {
					int n = pts.GetCount();
					int sz = max(6, (int)(8 * vp.GetScale()));
					DrawArrow(w, pts[n - 2], pts[n - 1], sz, edge_clr);
				}

				// Edge label centered at path midpoint
				if(!item.text.IsEmpty()) {
					int mid = pts.GetCount() / 2;
					Point mp = pts[mid];
					Font f = StdFont().Height(max(6, (int)(11 * vp.GetScale())));
					Size ts = GetTextSize(item.text, f);
					w.DrawText(mp.x - ts.cx / 2, mp.y - ts.cy - 2, item.text, f, edge_clr);
				}
			}
		}
		else if(item.type == SceneItem::LABEL) {
			Rect r = vp.WorldToView(item.rect);
			Font f = StdFont();
			if(!item.font_face.IsEmpty()) f.FaceName(item.font_face);
			f.Height(max(6, (int)(item.font_height * vp.GetScale())));
			if(item.font_bold)   f.Bold();
			if(item.font_italic) f.Italic();
			DrawCenteredText(w, r, item.text, f, line);
		}
	}
}

} // namespace Node

} // namespace Upp
