//$ class Ctrl {
private:
	static Ptr<Ctrl>      desktop;
	static Vector<Ctrl *> topctrl;
	static bool           invalid;

	static Point fbCursorPos;
	static Image fbCursorImage;
	
	static Rect  fbCaretRect;
	static int   fbCaretTm;
	
	static bool  fbEndSession;
	static int64 fbEventLoop;
	static int64 fbEndSessionLoop;
	
	static void CursorSync();
	
	int FindTopCtrl() const;
	static Rect GetClipBound(const Vector<Rect>& inv, const Rect& r);
	static void DoPaint();
	static void SyncTopWindows();

//	static void AddInvalid(const Rect& rect);

	void DestroyWnd();

	void NewTop()                       { SetTop(new Top); GetTop()->owner_window = NULL; }
	void PutForeground();
	static void MouseEventFB(Ptr<Ctrl> t, int event, Point p, int zdelta);

	static void DrawLine(const Vector<Rect>& clip, int x, int y, int cx, int cy, bool horz,
	                     const byte *pattern, int animation);
	static void DragRectDraw0(const Vector<Rect>& clip, const Rect& rect, int n,
	                          const byte *pattern, int animation);

	friend class TopWindowFrame;
	friend class SystemDraw;
	friend struct DnDLoop;

	void  SetOpen(bool b)               { isopen = b; }

	static void DeleteDesktopTop();

	static int    GetCaretBlinkTime()               { return 500; }
	
protected:
	static int PaintLock;

public:
	static void DoMouseFB(int event, Point p, int zdelta = 0);
	static bool DoKeyFB(dword key, int cnt);

	static void InitFB();
	static void ExitFB();
	static void EndSession();
	
	static void PaintAll()                     { DoPaint(); }

	static void  SetDesktop(Ctrl& q);
	static Ctrl *GetDesktop()                  { return desktop; }
	static void  SetDesktopSize(Size sz);
	
	static void Invalidate()                   { invalid = true; }

	void DragRectDraw(const Rect& rect1, const Rect& rect2, const Rect& clip, int n,
	                  Color color, int type, int animation);

	static Ctrl *FindMouseTopCtrl();

	static void PaintScene(SystemDraw& draw);
	// NetworkDisplay/0014: entered from PaintScene() instead of the single-canvas
	// loop whenever VirtualGuiPtr->WantsPerWindowRouting() is true (NetDpy only --
	// see VirtualGui.h). Declared here (not just Turtle/Ctrl.h) because Wnd.cpp is
	// shared source compiled into both Turtle and NetDpy backends via VirtualGui's
	// own package; mirrored verbatim into Turtle/Ctrl.h so Wnd.cpp still compiles
	// there even though Turtle never reaches this function.
	static void PaintPerWindowScene(SystemDraw& draw);
	static void PaintCaretCursor(SystemDraw& draw);
	
	enum { DRAWDRAGRECT_SCREEN = 0x8000 };

//$ };
