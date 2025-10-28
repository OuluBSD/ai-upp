#pragma once
#ifndef _CtrlLib_ColumnList_h_
#define _CtrlLib_ColumnList_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

// ColumnList - a control that displays a list of items in columns
class ColumnList : public Ctrl {
public:
	enum {
		MODE_LIST = 0,
		MODE_COLUMN = 1,
		MODE_ROWS = 2
	};

private:
	struct Item {
		Value key;
		Value value;
		bool  sel;
		bool  canselect;
		const Display *display;
	};

	std::vector<Item> item;
	const Display    *display;
	ScrollBar         sb;
	Scroller          scroller;
	InfoCtrl          info;
	
	int               cursor;
	int               anchor;
	int               dropitem;
	int               selcount;
	int               ncl; // number of columns
	int               cx, cy; // cell dimensions
	int               mode;
	int               ci; // dragging column
	int               dx; // drag offset
	int               mpos; // mouse position during drag
	bool              multi;
	bool              nobg;
	bool              popupex;
	bool              insert;
	bool              selclick;
	bool              clickkill;
	
	CtrlFrame        *frame;

	// Internal methods
	void SetSb();
	void ScrollInto(int pos);
	void RefreshItem(int i, int ex = 0);
	void SetCursor0(int c, bool sel);
	void GetItemStyle(int i, Color& fg, Color& bg, dword& st);
	dword PaintItem(Draw& w, int i, const Rect& r);
	Rect  GetItemRect(int i) const;
	void  PaintRows(Draw &w, Size &sz);
	void  RefreshSel();
	void  SyncInfo();
	void  ShiftSelect();
	int   GetSbPos(const Size& sz = Size(0, 0)) const;
	int   GetPageItems() const;
	int   GetColumnItems() const;
	int   GetDragColumn(int x) const;
	int   GetItem(Point p);
	void  PointDown(Point p);
	void  DoClick(Point p, dword flags);
	void  DoLeftDown(Point p, dword flags);
	void  PaintItem(Draw& w, int i, const Rect& r, const Rect& clip);
	int   RoundedCy();

public:
	// Drawing and events
	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
	virtual void LeftUp(Point p, dword flags) override;
	virtual void LeftDrag(Point p, dword keyflags) override;
	virtual void RightDown(Point p, dword flags) override;
	virtual void LeftDouble(Point p, dword flags) override;
	virtual void MouseMove(Point p, dword flags) override;
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) override;
	virtual void MouseLeave() override;
	virtual Image CursorImage(Point p, dword keyflags) override;
	virtual bool Key(dword key, int count) override;
	virtual void CancelMode() override;
	virtual void GotFocus() override;
	virtual void LostFocus() override;
	
	// Drag and drop
	virtual void DragAndDrop(Point p, PasteClip& d) override;
	virtual void DragRepeat(Point p) override;
	virtual void DragEnter() override;
	virtual void DragLeave() override;
	virtual Image GetDragSample() override;
	
	// Layout
	virtual void Layout() override;
	virtual void FrameLayout(Rect& r) override;
	virtual void FrameAddSize(Size& sz) override;
	virtual void FramePaint(Draw& w, const Rect& r) override;
	
	// Item management
	void Clear();
	void Insert(int i, const Value& key, const Value& val, bool canselect = true);
	void Insert(int i, const Value& key, const Value& val, const Display& display, bool canselect = true);
	void Set(int i, const Value& key, const Value& val, bool canselect = true);
	void Set(int i, const Value& key, const Value& val, const Display& display, bool canselect = true);
	void Set(const Value& key, const Value& val, const Display& display, bool canselect = true);
	void Set(const Value& key, const Value& val, bool canselect = true);
	void Remove(int i);
	void Add(const Value& key, const Value& val, bool canselect = true);
	void Add(const Value& key, const Value& val, const Display& display, bool canselect = true);
	void RemoveSelection();
	
	Value Get(int i) const { return i >= 0 && i < item.size() ? item[i].value : Value(); }
	Value GetKey(int i) const { return i >= 0 && i < item.size() ? item[i].key : Value(); }
	Value GetValue(int i) const { return Get(i); }
	int   GetCount() const { return item.size(); }
	
	int   Find(const Value& key) const;
	
	// Selection
	void   ClearSelection();
	void   SelectOne(int i, bool sel);
	void   UpdateSelect();
	void   SetMultiSelect(bool b) { multi = b; }
	bool   IsMultiSelect() const { return multi; }
	bool   IsSelected(int i) const;
	bool   IsSel(int i) const;
	int    GetSelectCount() const { return selcount; }
	Value  GetData() const;
	void   SetData(const Value& key);
	
	// Cursor
	void   SetCursor(int c);
	void   KillCursor();
	int    GetCursor() const { return cursor; }
	void   ScrollToCursor();
	
	// Column management
	void   SetColumns(int n) { ncl = max(1, n); Refresh(); }
	int    GetColumns() const { return ncl; }
	void   SetColumnWidth(int width) { cx = width; Refresh(); }
	int    GetColumnWidth() const { return cx; }
	void   SetItemHeight(int height) { cy = height; Refresh(); Layout(); }
	int    GetItemHeight() const { return cy; }
	
	// Mode and appearance
	ColumnList& Mode(int m);
	ColumnList& ListMode() { return Mode(MODE_LIST); }
	ColumnList& ColumnMode() { return Mode(MODE_COLUMN); }
	ColumnList& RowsMode() { return Mode(MODE_ROWS); }
	ColumnList& MultiSelect() { multi = true; return *this; }
	ColumnList& SingleSelect() { multi = false; return *this; }
	ColumnList& NoBackground() { nobg = true; return *this; }
	ColumnList& PopupEx(bool b = true) { popupex = b; return *this; }
	ColumnList& ClickKillCursor(bool b = true) { clickkill = b; return *this; }
	ColumnList& SetDisplay(const Display& d) { display = &d; return *this; }
	
	ColumnList& RoundSize(bool b = true);
	void SetFrame(CtrlFrame& frame);
	
	// Sorting
	void Sort(const ValueOrder& order);
	
	// Drag and drop helpers
	void DnD(int drop, bool insert);
	void InsertDrop(int ii, const Vector<Value>& data, PasteClip& d, bool self = false);
	void InsertDrop(int ii, const Vector<Value>& keys, const Vector<Value>& data, PasteClip& d, bool self = false);
	void InsertDrop(int ii, const ColumnList& src, PasteClip& d);
	void InsertDrop(int ii, PasteClip& d);
	
	// Scrolling
	void Scroll() ;
	int  GetSbPos() const { return GetSbPos(GetSize()); }
	void SetSbPos(int pos);
	
	// Serialization
	void SerializeSettings(Stream& s);
	
	// Constructor/Destructor
	ColumnList();
	virtual ~ColumnList();
};

}

#endif