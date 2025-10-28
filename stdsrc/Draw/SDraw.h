#pragma once
#ifndef _Draw_SDraw_h_
#define _Draw_SDraw_h_

#include "Draw.h"
#include <memory>

namespace Upp {

// Soft Drawing - a software-based drawing implementation
class SDraw : public Draw {
	Size   size;
	byte  *data;
	int    pitch;  // bytes per scanline
	int    bpp;    // bytes per pixel
	
	bool   owns_memory;
	
	void   FreeMemory();
	void   AllocMemory();

public:
	virtual dword GetInfo() const override;
	virtual Size  GetPageSize() const override;
	virtual Rect  GetPaintRect() const override;

	virtual void  BeginOp() override;
	virtual void  EndOp() override;
	
	virtual void  OffsetOp(Point p) override;
	virtual bool  ClipOp(const Rect& r) override;
	virtual bool  ClipoffOp(const Rect& r) override;
	virtual bool  ExcludeClipOp(const Rect& r) override;
	virtual bool  IntersectClipOp(const Rect& r) override;
	virtual bool  IsPaintingOp(const Rect& r) const override;

	virtual void  DrawRectOp(int x, int y, int cx, int cy, Color color) override;
	virtual void  DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) override;
	virtual void  DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color) override;
	virtual void  DrawPolyPolylineOp(const Point *vertices, int vertex_count,
	                                 const int *counts, int count_count,
	                                 int width, Color color, Color doxor) override;
	virtual void  DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
	                                    const int *subpolygon_counts, int subpolygon_count_count,
	                                    const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
	                                    Color color, int width, Color outline, uint64 pattern, Color doxor) override;
	virtual void  DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor) override;
	virtual void  DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color) override;
	virtual void  DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
	                        int n, const int *dx) override;
	virtual void  DrawDrawingOp(const Rect& target, const Drawing& w) override;
	virtual void  DrawPaintingOp(const Rect& target, const Painting& w) override;
	virtual void  DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id) override;
	virtual void  Escape(const String& data) override;

public:
	void Create(Size sz);
	void Create(int cx, int cy) { Create(Size(cx, cy)); }
	
	Size GetSize() const { return size; }
	
	Image GetImage() const;  // Convert to Image
	
	byte* GetScanLine(int y) const { return data + y * pitch; }
	int   GetPitch() const { return pitch; }
	int   GetBpp() const { return bpp; }
	
	SDraw();
	SDraw(Size sz);
	SDraw(int cx, int cy);
	~SDraw();
	
private:
	SDraw(const SDraw&) = delete;
	void operator=(const SDraw&) = delete;
};

}

#endif