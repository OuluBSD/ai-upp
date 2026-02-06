class TreeArrayCtrl : public ArrayCtrl {
public:
	class Node {
		void Init() { display = NULL; size = Null; margin = 2; canopen = false; canselect = true; }

	public:
		Image          image;
		int            margin;
		Value          key;
		Value          value;
		const Display *display;
		Size           size;
		Ptr<Ctrl>      ctrl;
		bool           canopen;
		bool           canselect;

		Node& SetImage(const Image& img)          { image = img; return *this; }
		Node& SetMargin(int m)                    { margin = m; return *this; }
		Node& Set(const Value& v)                 { key = value = v; return *this; }
		Node& Set(const Value& v, const Value& t) { key = v; value = t; return *this; }
		Node& SetDisplay(const Display& d)        { display = &d; return *this; }
		Node& SetSize(Size sz)                    { size = sz; return *this; }
		Node& SetCtrl(Ctrl& _ctrl)                { ctrl = &_ctrl; return *this; }
		Node& CanOpen(bool b = true)              { canopen = b; return *this; }
		Node& CanSelect(bool b)                   { canselect = b; return *this; }

		Node();
		Node(const Image& img, const Value& v);
		Node(const Image& img, const Value& v, const Value& t);
		Node(const Value& v);
		Node(const Value& v, const Value& t);
		Node(Ctrl& ctrl);
		Node(const Image& img, Ctrl& ctrl, int cx = 0, int cy = 0);
	};

private:
	struct Item : Node {
		union {
			int parent;
			int freelink;
		};

		bool        free;
		bool        isopen;
		bool        sel;
		Vector<int> child;
		Vector<Value> cols;
		int         level;
		int         line;

		Item() { isopen = false; line = -1; parent = -1; canselect = true; sel = false; free = false; level = 0; }
	};

	class TreeDisplay : public Display {
		TreeArrayCtrl *owner = NULL;

	public:
		TreeDisplay() {}
		explicit TreeDisplay(TreeArrayCtrl *o) : owner(o) {}
		void SetOwner(TreeArrayCtrl *o) { owner = o; }

		virtual void Paint(Draw& w, const Rect& r, const Value& v, Color ink, Color paper, dword style) const;
		virtual Size GetStdSize(const Value& v) const;
	};

	Array<Item>   item;
	Vector<int>   line;
	int           freelist = -1;
	int           levelcx = 16;
	double        levelcx_scale = 1.0;
	int           cursor_id = -1;
	bool          noroot = false;
	bool          show_empty_open = false;
	bool          dirty = true;
	bool          syncing = false;
	TreeDisplay   treedisplay;

	void   SyncTree();
	void   BuildLine(int id, int level);
	void   EnsureRoot();
	void   EnsureColumnCount();
	int    NewItem();
	bool   IsVisible(int id) const;
	int    GetLevelCx() const;
	bool   HasOpenIndicator(const Item& m) const;
	Rect   GetToggleRect(int line, const Rect& cell) const;
	Rect   GetIconRect(int line, const Rect& cell) const;

protected:
	virtual void  LeftDown(Point p, dword flags);
	virtual void  LeftDouble(Point p, dword flags);
	virtual void  RightDown(Point p, dword flags);
	virtual bool  Key(dword key, int count);

public:
	Event<int> WhenOpen;
	Event<int> WhenClose;
	Event<Bar&> WhenMenu;

	TreeArrayCtrl();
	~TreeArrayCtrl();

	Column& AddColumn(const char *text, int w = 0);
	Column& AddColumn(const Id& id, const char *text, int w = 0);

	void   SetRoot(const Node& n);
	void   SetRoot(const Image& img, Value v);
	void   SetRoot(const Image& img, Value v, Value t);
	void   SetRoot(const Image& img, Ctrl& ctrl, int cx = 0, int cy = 0);

	int    Insert(int parentid, int i, const Node& n);
	int    Add(int parentid, const Node& n);
	int    Insert(int parentid, int i);
	int    Add(int parentid);
	int    Insert(int parentid, int i, const Image& img, Value v, bool withopen = true);
	int    Insert(int parentid, int i, const Image& img, Value v, Value t, bool withopen = true);
	int    Insert(int parentid, int i, const Image& img, Value key, const String& value, bool withopen = true);
	int    Insert(int parentid, int i, const Image& img, Value key, const char *value, bool withopen = true);
	int    Add(int parentid, const Image& img, Value v, bool withopen = true);
	int    Add(int parentid, const Image& img, Value v, Value t, bool withopen = true);
	int    Add(int parentid, const Image& img, Value key, const String& value, bool withopen = true);
	int    Add(int parentid, const Image& img, Value key, const char *value, bool withopen = true);
	int    Insert(int parentid, int i, const Image& img, Ctrl& ctrl, int cx = 0, int cy = 0, bool withopen = true);
	int    Add(int parentid, const Image& img, Ctrl& ctrl, int cx = 0, int cy = 0, bool withopen = true);

	void   RemoveChildren(int id);
	void   Remove(int id);
	void   Clear();

	bool   IsValid(int id) const;
	int    GetChildCount(int id) const;
	int    GetChild(int id, int i) const;
	int    GetChildIndex(int id, int childid) const;
	int    GetParent(int id) const;

	Value  Get(int id) const;
	Value  GetValue(int id) const;
	Value  operator[](int id) const               { return GetValue(id); }
	void   Set(int id, Value v);
	void   Set(int id, Value k, Value v);
	void   SetValue(const Value& v);

	void   SetDisplay(int id, const Display& display);
	const Display& GetDisplay(int id) const;

	bool   IsOpen(int id) const;
	void   Open(int id, bool open = true);
	void   Close(int id)                           { Open(id, false); }
	void   OpenDeep(int id, bool open = true);
	void   CloseDeep(int id)                       { OpenDeep(id, false); }

	int    GetLineCount();
	int    GetItemAtLine(int i);
	int    GetLineAtItem(int id);

	void   SetCursor(int id);
	int    GetCursor() const                       { return cursor_id; }
	bool   IsCursor() const                        { return cursor_id >= 0; }
	void   KillCursor();

	void   MakeVisible(int id);

	TreeArrayCtrl& NoRoot(bool b = true)          { noroot = b; SyncTree(); return *this; }
	TreeArrayCtrl& LevelCx(int cx)                { levelcx = cx; Refresh(); return *this; }
	TreeArrayCtrl& LevelCxScale(double s)         { levelcx_scale = s; Refresh(); return *this; }
	TreeArrayCtrl& LevelCxFromLineCy(double s)    { levelcx = max(1, (int)floor(GetLineCy() * s)); Refresh(); return *this; }
	TreeArrayCtrl& ShowEmptyOpen(bool b = true)   { show_empty_open = b; Refresh(); return *this; }
	TreeArrayCtrl& MultiSelect(bool b = true)     { ArrayCtrl::MultiSelect(b); return *this; }
	TreeArrayCtrl& NoCursor(bool b = true);

	void   SetRowValue(int id, int col, const Value& v);
	Value  GetRowValue(int id, int col) const;
	Vector<Value> GetRowValues(int id) const;

	typedef TreeArrayCtrl CLASSNAME;
};
