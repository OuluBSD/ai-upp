#include "Ctrl.h"

namespace Upp {

namespace Node {

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

static void DrawEllipse(Draw& w, Rect r, Color fill, Color line, int lw)
{
	if(r.Width() <= 0 || r.Height() <= 0) return;
	double cx = r.left + r.Width()  / 2.0;
	double cy = r.top  + r.Height() / 2.0;
	double rx = r.Width()  / 2.0;
	double ry = r.Height() / 2.0;
	const int N = 24;
	Vector<Point> pts;
	pts.Reserve(N + 1);
	for(int i = 0; i <= N; i++) {
		double a = 2.0 * M_PI * i / N;
		pts.Add(Point((int)(cx + rx * cos(a)), (int)(cy + ry * sin(a))));
	}
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, IsNull(line) ? fill : line);
	else if(!IsNull(line))
		w.DrawPolyline(pts, lw, line);
}

static void DrawDiamond(Draw& w, Rect r, Color fill, Color line, int lw)
{
	int cx = r.left + r.Width()  / 2;
	int cy = r.top  + r.Height() / 2;
	Vector<Point> pts = {
		Point(cx, r.top), Point(r.right-1, cy),
		Point(cx, r.bottom-1), Point(r.left, cy), Point(cx, r.top),
	};
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, IsNull(line) ? fill : line);
	else if(!IsNull(line))
		w.DrawPolyline(pts, lw, line);
}

// Rounded rectangle approximated as a polygon
static void DrawRoundRect(Draw& w, Rect r, int radius, Color fill, Color line, int lw)
{
	if(r.Width() <= 0 || r.Height() <= 0) return;
	radius = min(radius, min(r.Width(), r.Height()) / 2);
	if(radius <= 0) {
		if(!IsNull(fill))  w.DrawRect(r, fill);
		if(!IsNull(line))  DrawFrame(w, r, line);
		return;
	}
	const int N = 8; // points per corner arc
	Vector<Point> pts;
	pts.Reserve(N * 4 + 4);
	// corners: top-left, top-right, bottom-right, bottom-left
	static const double starts[] = { M_PI, -M_PI/2, 0, M_PI/2 };
	struct { int x, y; } corners[] = {
		{ r.left + radius,  r.top + radius    },
		{ r.right - radius, r.top + radius    },
		{ r.right - radius, r.bottom - radius },
		{ r.left + radius,  r.bottom - radius },
	};
	for(int c = 0; c < 4; c++) {
		for(int i = 0; i <= N; i++) {
			double a = starts[c] + (M_PI / 2.0) * i / N;
			pts.Add(Point(corners[c].x + (int)(radius * cos(a)),
			              corners[c].y + (int)(radius * sin(a))));
		}
	}
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, IsNull(line) ? fill : line);
	if(!IsNull(line)) {
		// Close the polyline back to start
		pts.Add(pts[0]);
		w.DrawPolyline(pts, lw, line);
	}
}

// Rounded rect only on top corners
static void DrawRoundRectTop(Draw& w, Rect r, int radius, Color fill, Color line, int lw)
{
	if(r.Width() <= 0 || r.Height() <= 0) return;
	radius = min(radius, min(r.Width(), r.Height()) / 2);
	if(radius <= 0) {
		if(!IsNull(fill))  w.DrawRect(r, fill);
		if(!IsNull(line))  DrawFrame(w, r, line);
		return;
	}
	const int N = 8;
	Vector<Point> pts;
	// top-left arc, top-right arc, then straight bottom corners
	for(int i = 0; i <= N; i++) {
		double a = M_PI + (M_PI / 2.0) * i / N;
		pts.Add(Point(r.left + radius + (int)(radius * cos(a)),
		              r.top + radius + (int)(radius * sin(a))));
	}
	for(int i = 0; i <= N; i++) {
		double a = -M_PI/2.0 + (M_PI / 2.0) * i / N;
		pts.Add(Point(r.right - radius + (int)(radius * cos(a)),
		              r.top + radius + (int)(radius * sin(a))));
	}
	pts.Add(Point(r.right, r.bottom));
	pts.Add(Point(r.left,  r.bottom));
	if(!IsNull(fill))
		w.DrawPolygon(pts, fill, lw, IsNull(line) ? fill : line);
	if(!IsNull(line))
		w.DrawPolyline(pts, lw, line);
}

static void DrawArrow(Draw& w, Point from, Point to, int size, Color clr)
{
	double dx = to.x - from.x, dy = to.y - from.y;
	double len = sqrt(dx*dx + dy*dy);
	if(len < 1) return;
	dx /= len; dy /= len;
	double px = -dy, py = dx;
	Point tip = to;
	Point b1((int)(tip.x - dx*size + px*size*0.4), (int)(tip.y - dy*size + py*size*0.4));
	Point b2((int)(tip.x - dx*size - px*size*0.4), (int)(tip.y - dy*size - py*size*0.4));
	Vector<Point> arrow = { tip, b1, b2, tip };
	w.DrawPolygon(arrow, clr, 1, clr);
}

static void DrawTextLeft(Draw& w, Rect r, const String& text, Font f, Color clr)
{
	if(text.IsEmpty()) return;
	int y = r.top + max(0, (r.Height() - f.GetHeight()) / 2) + f.GetAscent() - f.GetHeight();
	int x = r.left + 4;
	w.DrawText(x, y, text, f, clr);
}

static void DrawTextRight(Draw& w, Rect r, const String& text, Font f, Color clr)
{
	if(text.IsEmpty()) return;
	int tw = GetTextSize(text, f).cx;
	int y = r.top + max(0, (r.Height() - f.GetHeight()) / 2) + f.GetAscent() - f.GetHeight();
	int x = r.right - tw - 4;
	w.DrawText(x, y, text, f, clr);
}

static void DrawTextCenter(Draw& w, Rect r, const String& text, Font f, Color clr)
{
	if(text.IsEmpty()) return;
	int tw = GetTextSize(text, f).cx;
	int y = r.top + max(0, (r.Height() - f.GetHeight()) / 2) + f.GetAscent() - f.GetHeight();
	int x = r.left + max(0, (r.Width() - tw) / 2);
	w.DrawText(x, y, text, f, clr);
}

// ---------------------------------------------------------------------------
// Background grid
// ---------------------------------------------------------------------------

void PaintBackground(Draw& w, Size sz, const Viewport& vp)
{
	w.DrawRect(sz, Color(20, 20, 20));

	const double SMALL_STEP = 10.0;   // world units per small cell (10x10 per large)
	const double LARGE_STEP = 100.0;  // world units per large cell
	double scale = vp.GetScale();
	if(scale < 0.02) return; // too zoomed out to draw grid

	Pointf world_tl = vp.ViewToWorld(Pointf(0, 0));
	Pointf world_br = vp.ViewToWorld(Pointf(sz.cx, sz.cy));

	// Small grid lines
	Color small_clr = Color(35, 35, 35);
	Color large_clr = Color(50, 50, 55);

	double x0 = floor(world_tl.x / SMALL_STEP) * SMALL_STEP;
	double y0 = floor(world_tl.y / SMALL_STEP) * SMALL_STEP;

	for(double wx = x0; wx <= world_br.x + SMALL_STEP; wx += SMALL_STEP) {
		int vx = (int)vp.WorldToView(Pointf(wx, 0)).x;
		bool is_large = fmod(fabs(wx), LARGE_STEP) < 0.5;
		w.DrawLine(vx, 0, vx, sz.cy, is_large ? 1 : 1,
		           is_large ? large_clr : small_clr);
	}
	for(double wy = y0; wy <= world_br.y + SMALL_STEP; wy += SMALL_STEP) {
		int vy = (int)vp.WorldToView(Pointf(0, wy)).y;
		bool is_large = fmod(fabs(wy), LARGE_STEP) < 0.5;
		w.DrawLine(0, vy, sz.cx, vy, is_large ? 1 : 1,
		           is_large ? large_clr : small_clr);
	}
}

// ---------------------------------------------------------------------------
// Per-item paint helper
// ---------------------------------------------------------------------------

static void PaintItem(Draw& w, const SceneItem& item, const Viewport& vp,
                      const EditorState* es, double scale,
                      VectorMap<String, Image>& img_cache)
{
	const int CORNER_R = 6;

	bool selected = es && es->IsSelected(item.entity_id);
	bool parent_selected = false;
	if(!selected && es) {
		int sep = item.entity_id.Find(':');
		if(sep >= 0)
			parent_selected = es->IsSelected(item.entity_id.Left(sep));
	}
	bool hovered = es && (es->hovered_entity == item.entity_id ||
	               (es->hovered_type == SceneItem::NODE && item.entity_id.StartsWith(
	                    item.entity_id.Left(item.entity_id.Find(':') < 0 ? item.entity_id.GetCount() : item.entity_id.Find(':')))));
	// Simpler hovered check:
	hovered = es && es->hovered_entity == item.entity_id;

	Color fill  = item.fill_clr;
	Color line  = item.line_clr;
	int   lw    = item.line_width;

	if(selected || parent_selected) { line = Color(255, 220, 80); lw = 2; }
	if(hovered)                     { line = Cyan(); lw = 2; }

	if(item.type == SceneItem::NODE) {
		Rect r = vp.WorldToView(item.rect);
		if(r.Width() <= 0 || r.Height() <= 0) return;
		int cr = max(1, (int)(CORNER_R * scale));
		DrawRoundRect(w, r, cr, fill, line, lw);
	}
	else if(item.type == SceneItem::GROUP) {
		Rect r = vp.WorldToView(item.rect);
		int cr = max(1, (int)(8 * scale));
		
		// Draw main area first (filled rect with rounded corners)
		DrawRoundRect(w, r, cr, fill, line, lw);
		
		// Draw title bar at top
		double title_h = 24 * scale;
		Rect title_bar(r.left + 4, r.top + 4, r.right - 4, r.top + title_h + 8);
		Color title_bg = Color(max(0, (int)fill.GetR() - 5), 
		                       max(0, (int)fill.GetG() - 5), 
		                       max(0, (int)fill.GetB() - 5));
		DrawRoundRect(w, title_bar, max(1,(int)(4*scale)), title_bg, line, 1);

		// Draw group label
		if(!item.text.IsEmpty()) {
			int font_h = clamp((int)(11 * scale), 9, 14);
			w.DrawText(title_bar.left + 8, title_bar.top + 2, item.text, StdFont(), White());
		}
		
		// Draw internal grid (10x10 pattern like background)
		double cell_w = 40 * scale;
		double cell_h = 40 * scale;
		Color grid_clr = Color(min(255, (int)fill.GetR() + 30),
		                       min(255, (int)fill.GetG() + 30),
		                       min(255, (int)fill.GetB() + 30));
		
		for(double x = r.left; x < r.right; x += cell_w) {
			w.DrawLine((int)x, (int)r.top, (int)x, (int)r.bottom, 1, grid_clr);
		}
		for(double y = r.top + title_h + 8; y < r.bottom; y += cell_h) {
			w.DrawLine((int)r.left, (int)y, (int)r.right, (int)y, 1, grid_clr);
		}
	}
	else if(item.type == SceneItem::PIN) {
		Rect r = vp.WorldToView(item.rect);
		if(item.shape == 1) {
			// Hover: draw larger bright ring + brightened fill
			Color pin_fill = fill;
			Color pin_line = Color(40, 40, 40);
			int   pin_lw   = 1;
			if(hovered) {
				pin_fill = Color(min(255, fill.GetR() + 60),
				                 min(255, fill.GetG() + 60),
				                 min(255, fill.GetB() + 60));
				pin_line = White();
				pin_lw   = 2;
				// Draw larger outer ring
				Rect big(r.left - 3, r.top - 3, r.right + 3, r.bottom + 3);
				DrawEllipse(w, big, Null, Color(200, 200, 200), 1);
			}
			DrawEllipse(w, r, pin_fill, pin_line, pin_lw);
		} else {
			if(!IsNull(fill))  w.DrawRect(r, fill);
			if(!IsNull(line))  DrawFrame(w, r, line);
		}
	}
	else if(item.type == SceneItem::EDGE) {
		if(item.path.GetCount() < 2) return;
		Color edge_clr = (selected || hovered) ? line : item.line_clr;
		if(IsNull(edge_clr)) edge_clr = Color(160, 160, 160);
		int   edge_w   = (selected || hovered) ? max(2, lw) : max(1, lw);

		Vector<Point> pts;
		pts.Reserve(item.path.GetCount());
		for(const auto& p : item.path)
			pts.Add(vp.WorldToView(p));

		bool is_pcb = !item.seg_layer.IsEmpty();
		if(is_pcb) {
			// PCB dual-layer rendering:
			// Layer 0 (front copper): warm green  #4A7C59 tinted
			// Layer 1 (back copper): cooler teal  #3A6070 tinted
			// When selected/hovered use bright highlight on all segments
			Color front_clr = selected || hovered ? line
			                : Color(
			                    max(0, min(255, edge_clr.GetR() / 2 + 40)),
			                    max(0, min(255, edge_clr.GetG() / 2 + 100)),
			                    max(0, min(255, edge_clr.GetB() / 2 + 50)));
			Color back_clr  = selected || hovered ? Color(line.GetR()/2+50, line.GetG()/2+80, line.GetB()/2+120)
			                : Color(
			                    max(0, min(255, edge_clr.GetR() / 2 + 20)),
			                    max(0, min(255, edge_clr.GetG() / 2 + 60)),
			                    max(0, min(255, edge_clr.GetB() / 2 + 90)));
			int trace_w = max(2, (int)(2.5 * scale));

			for(int i = 0; i + 1 < pts.GetCount(); i++) {
				int layer = (i < item.seg_layer.GetCount()) ? item.seg_layer[i] : 0;
				Color seg_clr = (layer == 0) ? front_clr : back_clr;
				w.DrawLine(pts[i].x, pts[i].y, pts[i+1].x, pts[i+1].y, trace_w, seg_clr);
			}

			// Through-hole pads at endpoints
			int pad_r = max(4, (int)(5 * scale));
			Color pad_ring = Color(200, 180, 100); // gold annular ring
			Color pad_hole = Color(20, 20, 20);
			for(int ep : {0, pts.GetCount() - 1}) {
				Rect pr(pts[ep].x - pad_r, pts[ep].y - pad_r,
				        pts[ep].x + pad_r, pts[ep].y + pad_r);
				DrawEllipse(w, pr, pad_ring, pad_ring, 1);
				int hr = max(2, pad_r / 2);
				Rect hr2(pts[ep].x - hr, pts[ep].y - hr,
				         pts[ep].x + hr, pts[ep].y + hr);
				DrawEllipse(w, hr2, pad_hole, pad_hole, 1);
			}

			// Via dots at bend points (layer transitions)
			int via_r = max(3, (int)(3.5 * scale));
			Color via_ring  = Color(180, 160, 80); // gold via ring
			Color via_fill  = Color(40, 40, 40);
			for(int vi : item.via_indices) {
				if(vi < 0 || vi >= pts.GetCount()) continue;
				Rect vr(pts[vi].x - via_r, pts[vi].y - via_r,
				        pts[vi].x + via_r, pts[vi].y + via_r);
				DrawEllipse(w, vr, via_fill, via_ring, 1);
			}
		} else {
			// Normal edge
			w.DrawPolyline(pts, edge_w, edge_clr);

			// Midpoint circle dot
			int mid = pts.GetCount() / 2;
			int dot_r = max(3, (int)(4 * scale));
			Rect dot_r2(pts[mid].x - dot_r, pts[mid].y - dot_r,
			            pts[mid].x + dot_r, pts[mid].y + dot_r);
			DrawEllipse(w, dot_r2, edge_clr, edge_clr, 1);

			// Arrow
			if(item.directed && pts.GetCount() >= 2) {
				int n = pts.GetCount();
				DrawArrow(w, pts[n-2], pts[n-1], max(6, (int)(8*scale)), edge_clr);
			}

			// Edge label
			if(!item.text.IsEmpty()) {
				int m = pts.GetCount() / 2;
				Font f = StdFont().Height(max(6, (int)(10*scale)));
				Size ts = GetTextSize(item.text, f);
				w.DrawText(pts[m].x - ts.cx/2, pts[m].y - ts.cy - dot_r - 2,
				           item.text, f, edge_clr);
			}
		}
	}
	else if(item.type == SceneItem::LABEL) {
		Rect r = vp.WorldToView(item.rect);
		if(r.Width() <= 0 || r.Height() <= 0) return;

		// Background — use selection/hover line color for outline when active
		if(!IsNull(item.fill_clr)) {
			int cr = max(1, (int)(CORNER_R * scale));
			Color bg_line = (selected || parent_selected || hovered) ? line : item.line_clr;
			int   bg_lw   = (selected || parent_selected || hovered) ? lw  : 1;
			if(item.shape == 3)
				DrawRoundRect(w, r, cr, item.fill_clr, bg_line, bg_lw);
			else
				w.DrawRect(r, item.fill_clr);
		}

		// Hamburger icon on title bars (font_bold + non-badge + has fill)
		if(item.font_bold && !item.badge && !IsNull(item.fill_clr)) {
			// Draw 3-line hamburger icon on the left side of title
			int icon_x = r.left + 6;
			int line_w = max(1, (int)(10 * scale));
			int bar_h  = max(1, (int)(1.5 * scale));
			int gap    = max(2, (int)(3 * scale));
			int total  = bar_h * 3 + gap * 2;
			int icon_y = r.top + (r.Height() - total) / 2;
			Color icon_clr = Color(160, 160, 160);
			for(int li = 0; li < 3; li++) {
				int y = icon_y + li * (bar_h + gap);
				w.DrawRect(icon_x, y, line_w, bar_h, icon_clr);
			}
			// Shift text rect to the right of the icon
			r.left += icon_x - r.left + line_w + 6;
		}

		if(item.text.IsEmpty()) return;
		Font f = StdFont();
		if(!item.font_face.IsEmpty()) f.FaceName(item.font_face);
		f.Height(max(5, (int)(item.font_height * scale)));
		if(item.font_bold) f.Bold();

		Color tc = !IsNull(item.text_clr) ? item.text_clr :
		           (!IsNull(line) ? line : Color(220, 220, 220));

		if(item.font_italic)
			DrawTextRight(w, r, item.text, f, tc);
		else if(item.badge)
			DrawTextCenter(w, r, item.text, f, tc);
		else
			DrawTextLeft(w, r, item.text, f, tc);
	}
	else if(item.type == SceneItem::WIDGET) {
		Rect r = vp.WorldToView(item.rect);
		if(r.Width() <= 0 || r.Height() <= 0) return;

		if(!item.image_path.IsEmpty()) {
			int q = img_cache.Find(item.image_path);
			if(q < 0) {
				Image img = StreamRaster::LoadFileAny(item.image_path);
				img_cache.Add(item.image_path, img);
				q = img_cache.GetCount() - 1;
			}
			const Image& img = img_cache[q];
			if(!IsNull(img)) {
				Size isz = img.GetSize();
				int iw = r.Width() - 4;
				int ih = r.Height() - 20;
				if(iw > 0 && ih > 0 && isz.cx > 0 && isz.cy > 0) {
					double aspect = (double)isz.cx / isz.cy;
					int dw = min(iw, (int)(ih * aspect));
					int dh = min(ih, (int)(iw / aspect));
					int ox = r.left + (r.Width() - dw) / 2;
					int oy = r.top + 2;
					w.DrawImage(ox, oy, dw, dh, img);
					String res_label = Format("%d×%d", isz.cx, isz.cy);
					Font f = StdFont().Height(max(5, (int)(9 * scale)));
					Size ts = GetTextSize(res_label, f);
					w.DrawText(r.left + (r.Width() - ts.cx)/2, oy + dh + 2,
					           res_label, f, Color(150, 150, 150));
				}
			} else {
				w.DrawRect(r, Color(35, 35, 35));
				DrawFrame(w, r, Color(60, 60, 60));
				Font f = StdFont().Height(max(5, (int)(9 * scale)));
				DrawTextCenter(w, r, item.image_path, f, Color(100, 100, 100));
			}
		} else {
			w.DrawRect(r, Color(30, 33, 40));
			DrawFrame(w, r, Color(45, 48, 58));
		}
	}
}

// ---------------------------------------------------------------------------
// Main scene painter — 4-pass Z-order:
//   Pass 1: groups (background)
//   Pass 2: edges  (behind nodes)
//   Pass 3: nodes + pins + labels + non-overlay widgets
//   Pass 4: overlay items (badges above node box)
// ---------------------------------------------------------------------------

void PaintScene(Draw& w, const Scene& scene, const Viewport& vp, const EditorState* es)
{
	double scale = vp.GetScale();
	static VectorMap<String, Image> img_cache;

	// Pass 1: groups
	for(const auto& item : scene.items)
		if(item.type == SceneItem::GROUP)
			PaintItem(w, item, vp, es, scale, img_cache);

	// Pass 2: edges (behind nodes)
	for(const auto& item : scene.items)
		if(item.type == SceneItem::EDGE)
			PaintItem(w, item, vp, es, scale, img_cache);

	// Pass 3: everything else except overlays
	for(const auto& item : scene.items)
		if(item.type != SceneItem::GROUP && item.type != SceneItem::EDGE && !item.overlay)
			PaintItem(w, item, vp, es, scale, img_cache);

	// Pass 4: overlay items (badges positioned above node box)
	for(const auto& item : scene.items)
		if(item.overlay)
			PaintItem(w, item, vp, es, scale, img_cache);
}

} // namespace Node

} // namespace Upp
