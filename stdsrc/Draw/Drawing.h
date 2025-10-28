#pragma once
#ifndef _Draw_Drawing_h_
#define _Draw_Drawing_h_

#include "Draw.h"
#include <vector>
#include <memory>

namespace Upp {

class DrawingDraw : public Draw {
	Size      size;
	bool      dots;
	Stream    drawing;
	ValueArray val;
	
	Stream& DrawingOp(int code);
	
	virtual dword GetInfo() const;
	virtual Size  GetPageSize() const;
	virtual Rect  GetPaintRect() const;

public:
	virtual void BeginOp();
	virtual void OffsetOp(Point p);
	virtual bool ClipOp(const Rect& r);
	virtual bool ClipoffOp(const Rect& r);
	virtual bool ExcludeClipOp(const Rect& r);
	virtual bool IntersectClipOp(const Rect& r);
	virtual bool IsPaintingOp(const Rect& r) const;
	virtual void EndOp();
	virtual void DrawRectOp(int x, int y, int cx, int cy, Color color);
	virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
	virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);
	virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
	                                const int *counts, int count_count,
	                                int width, Color color, Color doxor);
	virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
	                                   const int *subpolygon_counts, int subpolygon_count_count,
	                                   const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
	                                   Color color, int width, Color outline, uint64 pattern, Color doxor);
	virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
	virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color);
	virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
	                       int n, const int *dx);
	virtual void DrawDrawingOp(const Rect& target, const Drawing& w);
	virtual void DrawPaintingOp(const Rect& target, const Painting& w);
	virtual void DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id);
	virtual void Escape(const String& data);

public:
	void     Create(Size sz, bool dots_ = false);
	void     Create(int cx, int cy, bool dots_ = false);
	
	Drawing  GetResult();

	DrawingDraw();
	DrawingDraw(Size sz, bool dots_ = false);
	DrawingDraw(int cx, int cy, bool dots_ = false);
};

class Drawing {
	Size      size;
	String    data;
	ValueArray val;

public:
	Size GetSize() const              { return size; }
	Size GetRawSize() const           { return size; }
	void SetSize(Size sz)             { size = sz; }
	
	void Append(Drawing& dw);

	void Serialize(Stream& s);
	
	void Clear()                      { size = Size(0, 0); data.Clear(); val.Clear(); }
	
	Drawing()                         {}
	
#ifdef CPU_64
	template <class K>
	Drawing(const K& s)               { Xmlize(*this, s); }
#else
	template <class K>
	Drawing(const K& s)               { Xmlize(*this, s); }
#endif

};

inline void Draw::DrawDrawing(const Rect& target, const Drawing& w) {
	DrawDrawingOp(target, w);
}

void Draw::DrawDrawing(int x, int y, int cx, int cy, const Drawing& w);
void Draw::DrawDrawing(int x, int y, const Drawing& w);

Size Drawing::RatioSize(int cx, int cy) const;

}

#endif