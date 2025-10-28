#pragma once
#ifndef _CtrlCore_LocalLoop_h_
#define _CtrlCore_LocalLoop_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

// LocalLoop - runs a local event loop, typically used for modal operations
class LocalLoop : public Ctrl {
	Ctrl *master;
	
public:
	virtual void Run();
	virtual void CancelMode();
	
	void SetMaster(Ctrl& m) { master = &m; }
	Ctrl& GetMaster() { return *master; }
	const Ctrl& GetMaster() const { return *master; }
	
	LocalLoop();
	virtual ~LocalLoop() {}
};

// RectTracker - tracks mouse movement and draws tracking rectangles
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
	Callback1<Rect&> sync;
	Callback1<Rect&> round;
	
	void RefreshRect(const Rect& old, const Rect& r);
	void DrawRect(Draw& w, Rect r);
	
public:
	virtual void Paint(Draw& w) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual void RightUp(Point p, dword keyflags) override;
	virtual Image CursorImage(Point p, dword keyflags) override;
	
	Rect Round(const Rect& r);
	void Pen(Point p, const PenInfo &pn, dword keyflags);
	
	Rect Track(const Rect& r, int tx = ALIGN_LEFT, int ty = ALIGN_TOP);
	int  TrackHorzLine(int x0, int y0, int cx, int line = 1);
	int  TrackVertLine(int x0, int y0, int cy, int line = 1);
	Point TrackLine(int x0, int y0);
	
	void SetCursorImage(const Image& img) { cursorimage = img; }
	void SetColor(Color c) { color = c; }
	void SetPattern(int p) { pattern = p; }
	void SetWidth(int w) { width = w; }
	void SetMinSize(Size sz) { minsize = sz; }
	void SetMaxSize(Size sz) { maxsize = sz; }
	void SetMaxRect(const Rect& r) { maxrect = r; }
	void SetKeepRatio(bool b) { keepratio = b; }
	void SetAnimation(int ms) { animation = ms; }
	void SetRounder(Ctrl *c) { rounder = c; }
	void SetSyncCallback(const Callback1<Rect&>& cb) { sync = cb; }
	void SetRoundCallback(const Callback1<Rect&>& cb) { round = cb; }
	
	RectTracker(Ctrl& master);
};

// PointLooper - a utility for point selection with animated cursor
class PointLooper : public LocalLoop {
	const std::vector<Image>& ani;
	int ani_ms;
	bool result;

public:
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual Image CursorImage(Point p, dword keyflags) override;
	virtual bool Key(dword key, int count) override;

	operator bool() const { return result; }

	PointLooper(Ctrl& ctrl, const std::vector<Image>& ani, int ani_ms);
};

// Utility function to run a point selection loop with animation
bool PointLoop(Ctrl& ctrl, const std::vector<Image>& ani, int ani_ms);
bool PointLoop(Ctrl& ctrl, const Image& img);

}

#endif