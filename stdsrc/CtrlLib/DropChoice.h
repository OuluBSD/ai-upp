#pragma once
#ifndef _CtrlLib_DropChoice_h_
#define _CtrlLib_DropChoice_h_

#include "Ctrl.h"
#include <vector>
#include <memory>

void DropEdge_Write(Value);

class PopUpTable : public ArrayCtrl { // deprecated, replaced with PopUpList
public:
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual bool Key(dword key, int) override;

protected:
	void PopupDeactivate();
	void PopupCancelMode();

	struct Popup : Ctrl {
		PopUpTable *table;

		virtual void Deactivate() { table->PopupDeactivate(); }
		virtual void CancelMode() { table->PopupCancelMode(); }
	};

	int          droplines;
	int          inpopup;
	bool         open;
	std::unique_ptr<Popup>   popup;

	void         DoClose();

public:
	void         PopUp(Ctrl *owner, int x, int top, int bottom, int width);
	void         PopUp(Ctrl *owner, int width);
	void         PopUp(Ctrl *owner);

	Event<>      WhenCancel;
	Event<>      WhenSelect;

	PopUpTable&  SetDropLines(int _droplines);

	void         Normal();

	PopUpTable();
	virtual ~PopUpTable();
};

class PopUpList {
protected:
	void PopupDeactivate();
	void PopupCancelMode();

	struct PopupArrayCtrl : ArrayCtrl {
		PopUpList *list;

		virtual void LeftUp(Point p, dword keyflags) override;
		virtual bool Key(dword key, int) override;
	};

	struct Popup : Ctrl {
		PopUpList     *list;
		PopupArrayCtrl ac;
		bool           closing = false;

		virtual void Deactivate() { if(!closing) list->PopupDeactivate(); }
		virtual void CancelMode() { if(!closing) list->PopupCancelMode(); }

		Popup(PopUpList *list);
	};

	std::vector<Value>           items;
	std::vector<word>            lineinfo;
	std::vector<const Display *> linedisplay;
	std::unique_ptr<Popup>              popup;
	const ScrollBar::Style *sb_style;
	const Display          *display;
	const Convert          *convert;
	int                     linecy;
	int                     cursor;
	int16                   droplines;
	int16                   inpopup:15;
	bool                    permanent:1;

	void          DoSelect();
	void          DoCancel();
	void          DoClose();

	friend struct Popup;

public:
	Event<>      WhenCancel;
	Event<>      WhenSelect;

	void         PopUp(Ctrl *owner, int x, int top, int bottom, int width);
	void         PopUp(Ctrl *owner, int width);
	void         PopUp(Ctrl *owner);
	
	ArrayCtrl&   Permanent();

	void         Clear();
	void         SetCount(int n);
	void         Add(const Value& v);
	void         AddSeparator();
	void         Remove(int i);
	void         Insert(int i, const Value& v);

	void         SetCursor(int i);
	int          GetCursor() const;

	int          GetCount() const;
	void         Set(int i, const Value& v);
	Value        Get(int i) const;
	int          Find(const Value& v) const;
	void         SetScrollBarStyle(const ScrollBar::Style& s);
	void         SetLineCy(int cy);
	int          GetLineCy() const;
	void         SetLineCy(int ii, int cy);
	int          GetLineCy(int ii) const;
	bool         Key(int c);
	bool         IsLineEnabled(int ii) const;

	void           SetDisplay(const Display& d);
	const Display& GetDisplay() const;

	void           SetDisplay(int i, const Display& d);
	const Display& GetDisplay(int i) const;

	void           SetConvert(const Convert& c);

	PopUpList&   SetDropLines(int _droplines);

	PopUpList();
	virtual ~PopUpList();
};

class DropList : public MultiButton, public Convert {
public:
	virtual void  MouseWheel(Point p, int zdelta, dword keyflags) override;
	virtual bool  Key(dword key, int) override;
	virtual void  SetData(const Value& data) override;
	virtual Value GetData() const override;
	virtual void  DropPush() override;

	virtual Value Format(const Value& q) const override;

private:
	PopUpList          list;
	Index<Value>       key;
	Value              value;
	const Convert     *valueconvert;
	const Display     *valuedisplay;
	int16              dropwidth;
	bool               displayall:1;
	bool               dropfocus:1;
	bool               notnull:1;
	bool               alwaysdrop:1;
	bool               usewheel:1;

	void          Select();
	void          Cancel();
	void          Change(int q);
	void          EnableDrop(bool b = true);
	void          Sync();

	typedef       DropList CLASSNAME;

public:
	typedef MultiButton::Style Style;

	Event<>       WhenDrop;

	DropList&     Add(const Value& key, const Value& value, bool enable = true);
	DropList&     Add(const Value& value);
	DropList&     Add(std::initializer_list<std::pair<Value, Value>> init);

	void          Remove(int i);
	void          ClearList();
	void          Clear();

	DropList&     AddSeparator();

	void          Drop();

	const Value& operator=(const Value& v);
	operator Value() const;

	void          SetIndex(int i);
	int           GetIndex() const;
	void          GoBegin();
	void          GoEnd();
	void          GoPrev();
	void          GoNext();

	bool          HasKey(const Value& k) const;
	int           FindKey(const Value& k) const;
	int           Find(const Value& k) const;
	int           FindValue(const Value& v) const;

	int           GetCount() const;
	void          Trim(int n);
	const Value&  GetKey(int i) const;

	Value         GetValue(int i) const;
	Value         GetValue() const;
	void          SetValue(int i, const Value& v);
	void          SetValue(const Value& v);
	Value         operator[](int i) const;

	void          Adjust();
	void          Adjust(const Value& k);

	ArrayCtrl&    ListObject();

	DropList&     SetDropLines(int d);
	DropList&     SetValueConvert(const Convert& cv);
	DropList&     SetConvert(const Convert& cv);
	DropList&     SetDisplay(int i, const Display& d);
	DropList&     SetDisplay(const Display& d);
	DropList&     SetLineCy(int i, int lcy);
	DropList&     SetLineCy(int lcy);
	DropList&     SetDisplay(const Display& d, int lcy);
	DropList&     ValueDisplay(const Display& d);
	DropList&     DisplayAll(bool b = true);
	DropList&     DropFocus(bool b = true);
	DropList&     NoDropFocus();
	DropList&     AlwaysDrop(bool e = true);
	DropList&     SetStyle(const Style& s);
	DropList&     NotNull(bool b = true);
	DropList&     DropWidth(int w);
	DropList&     DropWidthZ(int w);
	DropList&     Wheel(bool b = true);
	DropList&     NoWheel();

	DropList&     SetScrollBarStyle(const ScrollBar::Style& s);

	DropList();
	virtual ~DropList();
};

void Append(DropList& list, const VectorMap<Value, Value>& values);
void Append(DropList& list, const VectorMap<int, String>& values);
void Append(MapConvert& convert, const VectorMap<Value, Value>& values);
void Append(MapConvert& convert, const VectorMap<int, String>& values);
void Append(DropList& list, const MapConvert& convert);

void operator*=(DropList& list, const VectorMap<Value, Value>& values);
void operator*=(DropList& list, const VectorMap<int, String>& values);
void operator*=(MapConvert& convert, const VectorMap<Value, Value>& values);
void operator*=(MapConvert& convert, const VectorMap<int, String>& values);
void operator*=(DropList& list, const MapConvert& convert);

class DropChoice : public MultiButtonFrame {
public:
	virtual void       Serialize(Stream& s); //empty

protected:
	PopUpList          list;
	Ctrl              *owner;
	bool               appending : 1;
	bool               dropfocus : 1;
	bool               always_drop : 1;
	bool               hide_drop : 1;
	bool               updownkeys : 1;
	bool               rodrop : 1;

	void        Select();
	void        Drop();
	void        EnableDrop(bool b);
	void        PseudoPush();

	int         dropwidth;

	typedef DropChoice CLASSNAME;

public:
	Event<>     WhenDrop;
	Event<>     WhenSelect;

	bool        DoKey(dword key);
	void        DoWheel(int zdelta);

	void        Clear();
	void        Add(const Value& data);
	int         Find(const Value& data) const;
	void        FindAdd(const Value& data);
	void        Set(int i, const Value& data);
	void        Remove(int i);
	void        SerializeList(Stream& s);

	int         GetCount() const;
	Value       Get(int i) const;

	void        AddHistory(const Value& data, int max = 12);

	void        AddTo(Ctrl& _owner);
	bool        IsActive() const;

	Value       Get() const;
	int         GetIndex() const;

	DropChoice& SetDisplay(int i, const Display& d);
	DropChoice& SetDisplay(const Display& d);
	DropChoice& SetLineCy(int lcy);
	DropChoice& SetDisplay(const Display& d, int lcy);
	DropChoice& SetConvert(const Convert& d);
	DropChoice& SetDropLines(int n);
	DropChoice& Appending();
	DropChoice& AlwaysDrop(bool e = true);
	DropChoice& HideDrop(bool e = true);
	DropChoice& RdOnlyDrop(bool e = true);
	DropChoice& NoDropFocus();

	DropChoice& DropWidth(int w);
	DropChoice& DropWidthZ(int w);
	DropChoice& UpDownKeys(bool b = true);

	DropChoice& SetScrollBarStyle(const ScrollBar::Style& s);

	DropChoice();

	static bool DataSelect(Ctrl& owner, DropChoice& drop, const String& appends);
};

template <class T>
class WithDropChoice : public T {
public:
	virtual bool   Key(dword key, int repcnt) override;
	virtual void   MouseEnter(Point p, dword keyflags) override;
	virtual void   MouseLeave() override;
	virtual void   MouseWheel(Point p, int zdelta, dword keyflags) override;
	virtual void   GotFocus() override;
	virtual void   LostFocus() override;

protected:
	DropChoice      select;
	String          appends;
	bool            withwheel;

	void            DoWhenSelect();
	void            DoWhenDrop();

public:
	Event<>         WhenDrop;
	Event<>         WhenSelect;

	void            ClearList();
	void            AddList(const Value& data);
	void            FindAddList(const Value& data);
	int             Find(const Value& data) const;
	void            Set(int i, const Value& data);
	void            Remove(int i);
	void            SerializeList(Stream& s);

	int             GetCount() const;
	Value           Get(int i) const;

	void            AddHistory(int max = 12);

	MultiButton::SubButton& AddButton();
	int                     GetButtonCount() const;
	MultiButton::SubButton& GetButton(int i);
	Rect                    GetPushScreenRect() const;

	const MultiButton::Style& StyleDefault();
	WithDropChoice& SetStyle(const MultiButton::Style& s);

	WithDropChoice& Dropping(bool b = true);
	WithDropChoice& NoDropping();
	WithDropChoice& NoDropFocus();
	WithDropChoice& Appending(const String& s = ", ");
	WithDropChoice& SetDropLines(int n);
	WithDropChoice& SetDisplay(int i, const Display& d);
	WithDropChoice& SetDisplay(const Display& d);
	WithDropChoice& SetLineCy(int lcy);
	WithDropChoice& SetDisplay(const Display& d, int lcy);
	WithDropChoice& SetConvert(const Convert& d);
	WithDropChoice& AlwaysDrop(bool b = true);
	WithDropChoice& HideDrop(bool b = true);
	WithDropChoice& RdOnlyDrop(bool b = true);
	WithDropChoice& WithWheel(bool b = true);
	WithDropChoice& NoWithWheel();
	WithDropChoice& DropWidth(int w);
	WithDropChoice& DropWidthZ(int w);
	WithDropChoice& UpDownKeys(bool b = true);
	WithDropChoice& NoUpDownKeys();

	WithDropChoice();
};

#endif