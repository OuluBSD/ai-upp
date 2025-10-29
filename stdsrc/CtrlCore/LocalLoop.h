#pragma once
#ifndef _CtrlCore_LocalLoop_h_
#define _CtrlCore_LocalLoop_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

class Ctrl;

class LocalLoop : public Ctrl {
	Ctrl *master;

public:
	virtual void Run();
	virtual void CancelMode();
	
	void SetMaster(Ctrl& m) { master = &m; }
	Ctrl& GetMaster() { return *master; }
	const Ctrl& GetMaster() const { return *master; }
	
	LocalLoop() { master = NULL; }
	virtual ~LocalLoop() {}
};

class RectTracker : public LocalLoop {
	Rect   rect;
	Rect   org;
	Rect   o;
	Rect   clip;
	Point  op;
	Image  master_image;
	int    tx, ty; // tracking x, y (-1 = no tracking)
	Size   minsize, maxsize;
	Rect   maxrect;
	bool   keepratio;
	Image  cursorimage;
	Color  color;
	int    pattern;
	int    animation;
	int    width;
	Event<Rect>  sync;
	Event<Rect&> round;
	
	Ctrl *rounder;
	
	void RefreshRect(const Rect& old, const Rect& r);
	void DrawRect(Draw& w, Rect r);
	Rect Round(const Rect& r);

public:
	virtual void Paint(Draw& w) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual void RightUp(Point p, dword keyflags) override;
	virtual Image CursorImage(Point p, dword keyflags) override;
	void Pen(Point p, const PenInfo &pn, dword keyflags);
	
	Rect Track(const Rect& r, int tx = ALIGN_LEFT, int ty = ALIGN_TOP);
	int  TrackHorzLine(int x0, int y0, int cx, int line = 1);
	int  TrackVertLine(int x0, int y0, int cy, int line = 1);
	Point TrackLine(int x0, int y0);
	
	RectTracker& SetCursorImage(const Image& img) { cursorimage = img; return *this; }
	RectTracker& SetColor(Color c) { color = c; return *this; }
	RectTracker& SetPattern(int p) { pattern = p; return *this; }
	RectTracker& Dashed() { return SetPattern(DRAWDRAGRECT_DASHED); }
	RectTracker& Solid() { return SetPattern(DRAWDRAGRECT_SOLID); }
	RectTracker& Normal() { return SetPattern(DRAWDRAGRECT_NORMAL); }
	RectTracker& SetWidth(int w) { width = w; return *this; }
	RectTracker& MinSize(Size sz) { minsize = sz; return *this; }
	RectTracker& MaxSize(Size sz) { maxsize = sz; return *this; }
	RectTracker& MaxRect(const Rect& r) { maxrect = r; return *this; }
	RectTracker& KeepRatio(bool b) { keepratio = b; return *this; }
	RectTracker& Animation(int ms = 40) { animation = ms; return *this; }
	RectTracker& Round(Ctrl& r) { rounder = &r; return *this; }

	Rect Get() { return rect; }
	
	RectTracker(Ctrl& master);
};

bool PointLoop(Ctrl& ctrl, const Vector<Image>& ani, int ani_ms);
bool PointLoop(Ctrl& ctrl, const Image& img);

}

#endif