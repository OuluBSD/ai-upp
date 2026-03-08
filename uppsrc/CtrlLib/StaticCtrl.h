class StaticText : public Ctrl {
public:
	bool   Access(Visitor& v) override;
	void   Paint(Draw& w) override;
	Size   GetMinSize() const override;

	enum {
		ATTR_INK = Ctrl::ATTR_LAST,
		ATTR_FONT,
		ATTR_ALIGN,
		ATTR_IMAGE,
		ATTR_IMAGE_SPC,
		ATTR_VALIGN,
		ATTR_ORIENTATION,
		ATTR_LAST,
	};

protected:
	String text;
	int    accesskey = 0;

	void MakeDrawLabel(DrawLabel& l) const;
	Size GetLabelSize() const;
	
public:
	StaticText& SetFont(Font font);
	StaticText& SetInk(Color color);
	StaticText& SetAlign(int align);
	StaticText& AlignLeft()                             { return SetAlign(ALIGN_LEFT); }
	StaticText& AlignCenter()                           { return SetAlign(ALIGN_CENTER); }
	StaticText& AlignRight()                            { return SetAlign(ALIGN_RIGHT); }
	StaticText& SetVAlign(int align);
	StaticText& AlignTop()                              { return SetVAlign(ALIGN_TOP); }
	StaticText& AlignVCenter()                          { return SetVAlign(ALIGN_CENTER); }
	StaticText& AlignBottom()                           { return SetVAlign(ALIGN_BOTTOM); }
	StaticText& SetImage(const Image& img, int spc = 0);
	StaticText& SetText(const char *text);
	StaticText& SetOrientation(int orientation);
	
	String      GetText() const                         { return text; }
	Font        GetFont() const                         { return Nvl(GetFontAttr(ATTR_FONT), StdFont()); }
	Color       GetInk() const                          { return Nvl(GetColorAttr(ATTR_INK), SColorText()); }
	int         GetAlign() const                        { return Nvl(GetIntAttr(ATTR_ALIGN), ALIGN_LEFT); }
	int         GetVAlign() const                       { return Nvl(GetIntAttr(ATTR_VALIGN), ALIGN_CENTER); }
	Image       GetImage() const                        { return GetAttr<Image>(ATTR_IMAGE); }
	int         GetOrientation() const                  { return Nvl(GetIntAttr(ATTR_ORIENTATION), ORIENTATION_NORMAL); }

	StaticText& operator=(const char *s)                { SetText(s); return *this; }

	StaticText();
	~StaticText();
};

class Label : public StaticText {
public:
	bool   HotKey(dword key) override;
	dword  GetAccessKeys() const override;
	void   AssignAccessKeys(dword used) override;

private:
	bool   noac;

public:
	Label& SetText(const char *text);
	Label& SetLabel(const char *lbl);

	Label& operator=(const char *s)                     { SetText(s); return *this; }

	Label();
	virtual ~Label();
};

class LabelBox : public Label {
public:
	void   Paint(Draw& w) override;
	void   AssignAccessKeys(dword used) override;
	Rect   GetVoidRect() const override;


	Color color;

	static Value LabelBoxLook;

public:
	static void  SetLook(const Value& v) { LabelBoxLook = v; }
	static Value GetLook()               { return LabelBoxLook; }

	LabelBox&    SetColor(Color c)       { color = c; return *this; }

	LabelBox();
	virtual ~LabelBox();
	LabelBox& operator=(const char *s)   { SetText(s); return *this; }
};

void PaintLabelBox(Draw& w, Size sz, Color color, int d);

Color LabelBoxTextColor();
Color LabelBoxColor();

void LabelBoxTextColor_Write(Color c);
void LabelBoxColor_Write(Color c);

class ParentCtrl : public Ctrl {
	Size minsize;

public:
	Rect   GetVoidRect() const override;
	Size   GetStdSize() const override;
	Size   GetMinSize() const override;
	void   SetMinSize(Size sz) override         { minsize = sz; }

	ParentCtrl();
};

class StaticRect : public Ctrl {
public:
	void   Paint(Draw& w) override;

protected:
	Value bg;

public:
	StaticRect& Background(const Value& chvalue);
	StaticRect& Color(class Color c)                   { Background(c); return *this; }

	Value GetBackground() const                        { return bg; }

	StaticRect();
	virtual ~StaticRect();
};

class ImageCtrl : public Ctrl {
public:
	void   Paint(Draw& w) override;
	Size   GetStdSize() const override;
	Size   GetMinSize() const override;

protected:
	Image   img;

public:
	ImageCtrl&   SetImage(const Image& _img);

	ImageCtrl()                                       { Transparent(); NoWantFocus(); }
};

class DisplayCtrl : public Ctrl {
public:
	void   Paint(Draw& w) override;
	Size   GetMinSize() const override;
	void   SetData(const Value& v) override;
	Value  GetData() const override;

private:
	PaintRect pr;

public:
	void SetDisplay(const Display& d);
};

class DrawingCtrl : public Ctrl {
public:
	void   Paint(Draw& w) override;

protected:
	Drawing picture;
	Color   background;
	bool    ratio;

public:
	Drawing  Get() const                             { return picture; }

	DrawingCtrl& Background(Color color);
	DrawingCtrl& KeepRatio(bool keep = true)         { ratio = keep; Refresh(); return *this; }
	DrawingCtrl& NoKeepRatio()                       { return KeepRatio(false); }
	DrawingCtrl& Set(const Drawing& _picture)        { picture = _picture; Refresh(); return *this; }

	DrawingCtrl& operator=(const Drawing& _picture)  { return Set(_picture); }
	DrawingCtrl& operator=(const Painting& _picture) { return Set(AsDrawing(_picture)); }

	DrawingCtrl();
};

// BWC
typedef ImageCtrl Icon;
typedef DrawingCtrl Picture;

class SeparatorCtrl : public Ctrl {
public:
	Size GetMinSize() const override;
	void Paint(Draw& w) override;

	struct Style : ChStyle<Style> {
		Value l1, l2;
	};

private:
	int          lmargin, rmargin;
	int          size;
	const Style *style;

public:
	static const Style& StyleDefault();

	SeparatorCtrl& Margin(int l, int r);
	SeparatorCtrl& Margin(int w)                { return Margin(w, w); }
	SeparatorCtrl& SetSize(int w);
	SeparatorCtrl& SetStyle(const Style& s);

	SeparatorCtrl();
};
