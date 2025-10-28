#pragma once
#ifndef _CtrlCore_SystemDraw_h_
#define _CtrlCore_SystemDraw_h_

#include "Ctrl.h"
#include "Draw.h"
#include <memory>

namespace Upp {

// SystemDraw represents the platform-specific drawing context
// It's typically used for drawing directly to windows or screen
class SystemDraw : public Draw {
	Size   size;
	
public:
	virtual dword GetInfo() const override { return DRAWSYS; }
	virtual Size  GetPageSize() const override { return size; }
	virtual Rect  GetPaintRect() const override { return Rect(size); }

	virtual void  BeginOp() override {}
	virtual void  EndOp() override {}
	
	virtual void  OffsetOp(Point p) override {}
	virtual bool  ClipOp(const Rect& r) override { return true; }
	virtual bool  ClipoffOp(const Rect& r) override { return true; }
	virtual bool  ExcludeClipOp(const Rect& r) override { return true; }
	virtual bool  IntersectClipOp(const Rect& r) override { return true; }
	
	virtual bool  IsPaintingOp(const Rect& r) const override { return true; }

	virtual void  DrawRectOp(int x, int y, int cx, int cy, Color color) override {}
	virtual void  DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) override {}
	virtual void  DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color) override {}
	virtual void  DrawPolyPolylineOp(const Point *vertices, int vertex_count,
	                                 const int *counts, int count_count,
	                                 int width, Color color, Color doxor) override {}
	virtual void  DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
	                                    const int *subpolygon_counts, int subpolygon_count_count,
	                                    const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
	                                    Color color, int width, Color outline, uint64 pattern, Color doxor) override {}
	virtual void  DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor) override {}
	virtual void  DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color) override {}
	virtual void  DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
	                        int n, const int *dx) override {}
	virtual void  DrawDrawingOp(const Rect& target, const Drawing& w) override {}
	virtual void  DrawPaintingOp(const Rect& target, const Painting& w) override {}
	virtual void  DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id) override {}
	virtual void  Escape(const String& data) override {}

	// Platform-specific operations
	virtual bool   CanSetSurface() const { return false; }
	
	void SetSize(Size sz) { size = sz; }
	Size GetSize() const { return size; }
	
	SystemDraw() { size = Size(0, 0); }
	
	// Platform specific implementations would override these
#ifdef PLATFORM_WIN32
	// Windows-specific methods
	HDC GetHDC() const { return nullptr; } // Placeholder
	void ReleaseHDC(HDC hdc) const {} // Placeholder
#elif defined(PLATFORM_POSIX)
	// X11-specific methods
	void* GetX11Display() const { return nullptr; } // Placeholder
	void* GetX11Drawable() const { return nullptr; } // Placeholder
#endif
};

// BackDraw - used for buffering and double-buffering operations
class BackDraw : public Draw {
	SystemDraw *painting;
	Point       painting_offset;
	Size        size;
	
public:
	virtual dword GetInfo() const override;
	virtual Size  GetPageSize() const override { return size; }
	virtual Rect  GetPaintRect() const override { return Rect(size); }

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

	void  Set(SystemDraw& w, Point offset);
	void  Set(SystemDraw *w, Point offset);
	void  Set(SystemDraw& w)                    { Set(w, Point(0, 0)); }
	void  Set(SystemDraw *w)                    { Set(w, Point(0, 0)); }
	void  Clear()                               { Set((SystemDraw *)NULL, Point(0, 0)); }
	void  Destroy();
	bool  Is() const                            { return painting != NULL; }
	
	BackDraw();
	~BackDraw();
};

// Screen information and system drawing operations
SystemDraw& ScreenInfo();

// Set surface data directly to the drawing context
void SetSurface(Draw& w, const Rect& dest, const RGBA *pixels, Size srcsz, Point poff);
void SetSurface(Draw& w, int x, int y, int cx, int cy, const RGBA *pixels);

}

#endif