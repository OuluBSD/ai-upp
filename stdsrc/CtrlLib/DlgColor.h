#pragma once
#ifndef _CtrlLib_DlgColor_h_
#define _CtrlLib_DlgColor_h_

#include "Ctrl.h"
#include <vector>
#include <memory>

class WheelRampCtrl : public Ctrl
{
public:
	WheelRampCtrl(bool ramp);
	~WheelRampCtrl();

	virtual void  Layout() override;
	virtual void  Paint(Draw& draw) override;
	virtual void  SetData(const Value& value) override;
	virtual Value GetData() const override;

	virtual void  LeftDown(Point pt, dword keyflags) override;
	virtual void  LeftUp(Point pt, dword keyflags) override;
	virtual void  LeftDouble(Point pt, dword keyflags) override;
	virtual void  MouseMove(Point pt, dword keyflags) override;

	Event<>       WhenLeftDouble;
	
	WheelRampCtrl& DarkContent(bool b = true);
	WheelRampCtrl& AllowDarkContent(bool b = true);
	bool           IsDarkContent() const;

private:
	void          SetColor(Color color, bool set_norm, bool set_hsv);

	Image         PaintRamp(Size size);
	Image         PaintWheel(Size size);
	void          PaintColumn(Draw& draw);

	int           ClientToLevel(int y) const;
	int           LevelToClient(int l) const;

private:
	bool          ramp;
	bool          dark_content = false;
	bool          allow_dark_content = false;
	Color         color;
	Color         normalized_color;
	int           h16, s16, v16;
	int           round_step;
	enum STYLE { S_WHEEL, S_RECT, S_HEXAGON };
	STYLE         style;
	Image         cache;
	int           cache_level;
	Image         wheel_cache;
	Rect          wheel_rect;
	Rect          column_rect;

	int           firstclick;

	struct WheelBuff
	{
		int arg;
		int l;
	};
};

struct ColorWheelCtrl : public WheelRampCtrl {
	ColorWheelCtrl() : WheelRampCtrl(false) {}
};

struct ColorRampCtrl : public WheelRampCtrl {
	ColorRampCtrl() : WheelRampCtrl(true) {}
};

class ColorPopUp : public Ctrl {
public:
	virtual  void Paint(Draw& w) override;
	virtual  void LeftUp(Point p, dword) override;
	virtual  void LeftDown(Point p, dword) override;
	virtual  void MouseMove(Point p, dword) override;
	virtual  void MouseLeave() override;
	virtual  bool Key(dword key, int count) override;
	virtual  void Layout() override;

private:
	void PopupDeactivate();

	struct Popup : Ctrl {
		ColorPopUp *color;
		
		virtual void Deactivate() { color->PopupDeactivate(); }
	};

	int      Get(Point p);
	int      GetCy();
	void     Setup(Color c);
	void     Finish();
	void     Ramp();
	void     Wheel();
	int      GetColorCount() const;
	Color    GetColor(int i) const;
	void     Select();

	void     DrawFilledFrame(Draw &w, int x, int y, int cx, int cy, Color fcol, Color bcol);
	void     DrawFilledFrame(Draw &w, Rect &r, Color fcol, Color bcol);

	int      colori;
	bool     notnull;
	bool     scolors;
	bool     norampwheel;
	bool     animating;
	bool     hints;
	bool     open;
	bool     withvoid;
	String   nulltext;
	String   voidtext;
	Color    color;

	ColorRampCtrl  ramp;
	ColorWheelCtrl wheel;
	Button         settext;
	std::unique_ptr<Popup>     popup;

	static Color   hint[18];
	
	friend void ColorPopUp_InitHint();

public:
	Event<>  WhenCancel;
	Event<>  WhenSelect;

	static void Hint(Color c);

	typedef ColorPopUp CLASSNAME;

	void     PopUp(Ctrl *owner, Color c = White());
	Color    Get() const;
	
	ColorPopUp& NotNull(bool b = true);
	ColorPopUp& SColors(bool b = true);
	ColorPopUp& NullText(const char *s);
	ColorPopUp& WithVoid(bool b = true);
	ColorPopUp& VoidText(const char *s);
	ColorPopUp& NoRampWheel(bool b = true);
	ColorPopUp& Hints(bool b = true);
	ColorPopUp& DarkContent(bool b = true);
	ColorPopUp& AllowDarkContent(bool b = true);
	bool        IsDarkContent() const;

	ColorPopUp();
};

class ColorPusher : public Ctrl {
public:
	virtual void  Paint(Draw& w) override;
	virtual void  LeftDown(Point p, dword) override;
	virtual bool  Key(dword key, int) override;
	virtual void  GotFocus() override;
	virtual void  LostFocus() override;
	virtual void  SetData(const Value& v) override;
	virtual Value GetData() const override;

protected:
	bool       push;
	bool       withtext;
	bool       withhex;
	bool       track;
	Color      color, saved_color;
	ColorPopUp colors;
	String     nulltext;
	String     voidtext;

	void AcceptColors();
	void CloseColors();
	void NewColor();
	void Drop();

public:
	typedef ColorPusher CLASSNAME;

	ColorPusher& NullText(const char *s);
	ColorPusher& NotNull(bool b = true);
	ColorPusher& WithVoid(bool b = true);
	ColorPusher& VoidText(const char *s);
	ColorPusher& SColors(bool b = true);
	ColorPusher& WithText(bool b = true);
	ColorPusher& WithHex(bool b = true);
	ColorPusher& Track(bool b = true);
	ColorPusher& NoTrack();
	ColorPusher& NoRampWheel(bool b = true);
	ColorPusher& Hints(bool b = true);
	ColorPusher& DarkContent(bool b = true);
	ColorPusher& AllowDarkContent(bool b = true);

	ColorPusher();
};

class ColorButton : public ColorPusher {
public:
	virtual void  Paint(Draw& w) override;
	virtual void  MouseEnter(Point p, dword keyflags) override;
	virtual void  MouseLeave() override;
	virtual Size  GetMinSize() const override;

protected:
	Image      image, nullimage, staticimage;
	const ToolButton::Style *style;

public:
	ColorButton& ColorImage(const Image& img);
	ColorButton& NullImage(const Image& img);
	ColorButton& StaticImage(const Image& img);
	ColorButton& SetStyle(const ToolButton::Style& s);

	ColorButton();
};

const Display& StdColorDisplayNull();

String FormatColor(Color c);
Color  ReadColor(CParser& p);
Color  RealizeColor(Color c);

#endif