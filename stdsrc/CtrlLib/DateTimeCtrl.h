#pragma once
#ifndef _CtrlLib_DateTimeCtrl_h_
#define _CtrlLib_DateTimeCtrl_h_

#include "Ctrl.h"
#include <vector>
#include <memory>

enum
{
	SUNDAY    = 0,
	MONDAY    = 1,
	TUESDAY   = 2,
	WEDNESDAY = 3,
	THURSDAY  = 4,
	FRIDAY    = 5,
	SATURDAY  = 6
};

class FlatButton : public Pusher {
public:
	Image img;
	Color fg, bg, hl;
	bool left;
	bool drawedge;
	bool highlight;

	FlatButton();

	void DrawFrame(Draw &w, const Rect &r, Color lc, Color tc, Color rc, Color bc);
	virtual void Paint(Draw &w) override;
	virtual void MouseEnter(Point p, dword kflags) override;
	virtual void MouseLeave() override;

	const Image& GetImage();

	FlatButton& SetImage(const Image &_img);
	FlatButton& SetLeft();
	FlatButton& SetRight();
	FlatButton& DrawEdge(bool b);
	FlatButton& Highlight(bool b);
};

class FlatSpin : public Ctrl
{
private:
	FlatButton left;
	FlatButton right;
	String text;
	Size tsz;
	Font font;
	bool selected;
	bool selectable;

public:
	FlatSpin();
	void SetText(const String& s);
	void SetTips(const char *tipl, const char *tipr);
	void SetCallbacks(const Event<>& cbl, const Event<>& cbr);

	FlatSpin& Selectable(bool b = true);
	int GetWidth(const String& s, bool with_buttons = true);
	int GetHeight();
	void SetFont(const Font& fnt);
	void SetLeftImage(const Image &img);
	void SetRightImage(const Image &img);
	void SetHighlight(bool b);

	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void MouseLeave() override;
	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void Layout() override;
	virtual void Paint(Draw& w) override;
	virtual Image CursorImage(Point p, dword keyflags) override;

	typedef FlatSpin CLASSNAME;
};

class PopUpCtrl : public Ctrl
{
protected:
	Image nbg;
	bool popup;
public:
	PopUpCtrl();

	Event<>  WhenPopDown;
	Event<>  WhenDeactivate;
	virtual void Deactivate();
	virtual Size ComputeSize() = 0;
	virtual void Reset();

	void PopUp(Ctrl *owner, const Rect& rt);
	Size GetPopUpSize();

	bool IsPopUp() const;
	void SetPopUp(bool b = true);
};

class Calendar : public PopUpCtrl
{
public:
	struct Style {
		Color header;

		Color bgmain;
		Color bgtoday;
		Color bgselect;

		Color fgmain;
		Color fgtoday;
		Color fgselect;
		Color outofmonth;
		Color curdate;
		Color today;
		Color selecttoday;
		Color cursorday;
		Color selectday;
		Color line;
		Color dayname;
		Color week;
		Font  font;
		Image spinleftimg;
		Image spinrightimg;
		bool  spinhighlight;
		
		static Style Default() { return Style(); }
	};

protected:
	const Style *style;
	const Style *St() const;

private:
	typedef Calendar CLASSNAME;

	FlatSpin spin_year;
	FlatSpin spin_month;
	FlatSpin spin_all;

	static const int cols = 7;
	static const int rows = 6;

	static const Point nullday;
	int bs; //border size
	int hs; //header vertical size
	int ts; //today vertical size
	float colw;
	float rowh;
	int cw, rh;

	int col;
	int row;
	int lastrow;

	int fh;

	int days[rows][cols];

	Point newday;
	Point oldday;
	Point prevday;
	Point curday, firstday;
	int   newweek, oldweek;

	String stoday;
	Size sztoday;
	String curdate;

	bool selall;
	bool istoday;
	bool wastoday;
	bool time_mode;
	bool swap_month_year;
	bool one_button;

	Time view;
	Time today;
	Time sel;

	int first_day;

	void OnMonthLeft();
	void OnMonthRight();
	void OnYearLeft();
	void OnYearRight();

	void UpdateFields();

	bool MouseOnToday(Point p);
	bool MouseOnHeader(Point p);

	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void Paint(Draw &w) override;
	virtual bool Key(dword key, int count) override;
	virtual void MouseLeave() override;
	virtual void State(int reason) override;
	virtual Image CursorImage(Point p, dword keyflags) override;

	int& Day(int x, int y);
	int& Day(Point p);
	Point GetDay(Point p);
	int GetWeek(Point p);

	virtual Size ComputeSize() override;

public:
	Calendar();
	Event<Time &> WhenTime;
	Event<Date>   WhenWeek;

	static const Style& StyleDefault();

	void Reset();

	int	 DayOfWeek(int day, int month, int year, int zelleroffset = 2);
	int  WeekOfYear(int day, int month, int year);

	virtual Value GetData() const;
	virtual void  SetData(const Value& v);

	Date GetDate() const;
	void SetDate(int y, int m, int d);
	void SetDate(const Date &dt);

	Date GetTime() const;
	void SetTime(int y, int m, int d, int h, int n, int s);
	void SetTime(const Time &tm);

	Date GetCursor() const;
	bool HasCursor() const;

	Date GetView() const;
	void SetView(const Time &v);

	Calendar& SetStyle(const Style& s);
	Calendar& SelectAll(bool b = true);
	Calendar& NoSelectAll();
	Calendar& FirstDay(int n = MONDAY);
	Calendar& TimeMode(bool b = true);
	Calendar& SwapMonthYear(bool b = true);
	Calendar& OneButton(bool b = true);
	Calendar& NoOneButton();

	void PopUp(Ctrl *owner, Rect &rt);

	Event<>  WhenSelect;
};

struct LineCtrl : Ctrl
{
	int pos, real_pos;

	virtual void Paint(Draw& w) override;
	void SetPos(Point p);
	int GetPos();
	void SetPos(int p);

	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;
	LineCtrl();
};

class Clock : public PopUpCtrl
{
public:
	struct Style {
		Color header;
		Color bgmain;
		Color fgmain;

		Color arrowhl;
		Color arrowhour;
		Color arrowminute;
		Color arrowsecond;

		Font font;
		
		static Style Default() { return Style(); }
	};

protected:
	const Style *style;
	const Style *St() const;

private:
	FlatSpin spin_hour;
	FlatSpin spin_minute;
	EditIntSpin ed_hour, ed_minute;
	LineCtrl ln_hour, ln_minute;

	Time sel;
	int hs;
	int dir;
	bool accept_time;

	struct Line {
		Pointf s, e;
	};

	struct MinMax {
		int diff;
		int value;
	};

	Line lines[3];
	Size sz;
	Pointf cm; //circle middle
	Pointf cf; //circle factor

	int64 cur_time;
	int   cur_line;
	int   prv_line;
	int   cur_hour;
	int   cur_minute;
	int   cur_second;
	int   cur_point;
	int   prv_point;

	bool seconds;
	bool colon;

	void PaintPtr(int n, Draw& w, Pointf p, double pos, double m, double rd, int d, Color color, Point cf);
	void PaintCenteredText(Draw& w, int x, int y, const char *text, const Font& fnt, Color c);
	void PaintCenteredImage(Draw &w, int x, int y, const Image& img);

	void SetHourEdit();
	void SetMinuteEdit();
	void SetHourLine();
	void SetMinuteLine();
	void SetHourLeft();
	void SetHourRight();
	void SetMinuteLeft();
	void SetMinuteRight();

	MinMax SetMinMax(int v, int min, int max);
	void UpdateFields();
	void UpdateTime();
	int  IncFactor(int dir, int pp, int cp);

	int  GetDir(int prev_point, int cur_point);
	int  GetPointedLine(Point p);
	int  GetPoint(Pointf p, double tolerance = 4.0);
	bool IsCircle(Pointf p, Pointf s, double r);
	bool IsLine(Pointf s, Pointf e, Pointf p, double tolerance = 3.0);
	void CalcSizes();

	void Timer();

public:
	virtual bool Key(dword key, int count) override;
	virtual void Paint(Draw& w) override;
	virtual void State(int reason) override;
	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual Image CursorImage(Point p, dword keyflags) override;
	virtual Size ComputeSize() override;

	virtual Value GetData() const override;
	virtual void  SetData(const Value& v) override;

	Time GetTime() const;
	void SetTime(int h, int n, int s);
	void SetTime(const Time& tm);

	static const Style& StyleDefault();

	void Reset();

	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;

	Clock& SetStyle(const Style& s);
	Clock& Seconds(bool b = true);
	Clock& NoSeconds();
	Clock& Colon(bool b = true);
	Clock& NoColon();

	Clock();

	typedef Clock CLASSNAME;
};

class CalendarClock : public Ctrl
{
private:
	int mode;

	Size calendar_size;
	Size clock_size;
	Size sz;

public:
	enum {
		MODE_DATE = 0,
		MODE_TIME = 1
	};

	typedef CalendarClock CLASSNAME;

	Calendar calendar;
	Clock clock;

	CalendarClock(int m = MODE_TIME);
	Event<>  WhenPopDown;

	virtual void Deactivate() override;
	virtual bool Key(dword key, int count) override;
	virtual void Layout() override;

	Size ComputeSize();
	Size GetCalendarClockSize();
	void UpdateTime(Time &tm);

	void PopUp(Ctrl *owner, const Rect& rt);
};

template<class T>
class DateTimeCtrl : public T {
	MultiButtonFrame drop;
	CalendarClock cc;

	int mode;

	void OnCalendarChoice();
	void OnClockChoice();
	void OnClose();
	void OnDrop();

public:
	typedef DateTimeCtrl CLASSNAME;
	
	Event<Date> WhenWeek;

	DateTimeCtrl(int m);
	
	virtual void Skin() override;
	virtual void GotFocus() override;
	virtual void LostFocus() override;
	virtual Size GetMinSize() const override;

	DateTimeCtrl& SetCalendarStyle(const Calendar::Style& style);
	DateTimeCtrl& SetClockStyle(const Clock::Style& style);
	DateTimeCtrl& SetButtonStyle(const MultiButton::Style& style);
	DateTimeCtrl& SelectAll(bool b = true);
	DateTimeCtrl& NoSelectAll();
	DateTimeCtrl& Seconds(bool b = true);
	DateTimeCtrl& NoSeconds();
	DateTimeCtrl& Colon(bool b = true);
	DateTimeCtrl& NoColon();
	DateTimeCtrl& SwapMonthYear(bool b = true);
	DateTimeCtrl& OneButton(bool b = true);
	DateTimeCtrl& NoOneButton();
	DateTimeCtrl& DayEnd(bool b = true);
	DateTimeCtrl& TimeAlways(bool b = true);

	Event<>  WhenSelect;
};

class DropDate : public DateTimeCtrl<EditDate>
{
public:
	DropDate();
	DropDate& SetDate(int y, int m, int d);
};

class DropTime : public DateTimeCtrl<EditTime>
{
public:
	DropTime();
	DropTime& SetTime(int y, int m, int d, int h, int n, int s);
	DropTime& Seconds(bool b = true);
	DropTime& NoSeconds();
};

#endif