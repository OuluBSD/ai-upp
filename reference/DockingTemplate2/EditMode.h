#ifndef _DockingTemplate2_EditMode_h_
#define _DockingTemplate2_EditMode_h_

class ModeManager {
public:
	enum Mode { MODE_VIEW = 0, MODE_EDIT = 1 };

	Event<int>   WhenModeChanged;
	Event<Point> WhenCursorChanged;

	void SetMode(Mode m);
	Mode GetMode() const { return current_; }

	void FireCursor(Point p);

private:
	Mode current_ = MODE_VIEW;
};

#endif
