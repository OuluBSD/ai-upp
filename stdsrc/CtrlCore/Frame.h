#pragma once
#ifndef _CtrlCore_Frame_h_
#define _CtrlCore_Frame_h_

#include <CtrlCore/CtrlCore.h>

namespace Upp {

class Frame : public Moveable<Frame> {
public:
	virtual void FrameLayout(Rect& r) = 0;
	virtual void FrameAddSize(Size& sz) = 0;
	virtual void FramePaint(Draw& w, const Rect& r) = 0;
	virtual int  OverPaint() const { return 0; }

	virtual ~Frame() {}
};

template <class T>
class FrameLR : public Frame {
	Size sz;

public:
	virtual void FrameLayout(Rect& r);
	virtual void FrameAddSize(Size& sz);
	virtual void FramePaint(Draw& w, const Rect& r);

	FrameLR(int cx);
};

template <class T>
class FrameTB : public Frame {
	Size sz;

public:
	virtual void FrameLayout(Rect& r);
	virtual void FrameAddSize(Size& sz);
	virtual void FramePaint(Draw& w, const Rect& r);

	FrameTB(int cy);
};

template <class T>
class FrameCtrl : public Frame {
	Size sz;
	T   ctrl;

public:
	virtual void FrameLayout(Rect& r);
	virtual void FrameAddSize(Size& sz);
	virtual void FramePaint(Draw& w, const Rect& r);

	T& operator()() { return ctrl; }

	FrameCtrl(int cx, int cy);
};

class ButtonFrame : public FrameLR<ButtonFrame> {
public:
	ButtonFrame() : FrameLR<ButtonFrame>(4) {}
};

}

#endif