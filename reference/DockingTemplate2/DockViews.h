#ifndef _DockingTemplate2_DockViews_h_
#define _DockingTemplate2_DockViews_h_

class DockViewBase : public DockableCtrl {
public:
	Event<Point> WhenCursorEmit;
	bool emit_cursor = false;

	virtual void OnCursorChanged(Point p) = 0;

protected:
	bool updating_ = false;
};

class DockViewA : public DockViewBase {
public:
	typedef DockViewA CLASSNAME;
	DockViewA();
	void OnCursorChanged(Point p) override;

private:
	Label   label_;
	Label   cursor_label_;
};

class DockViewB : public DockViewBase {
public:
	typedef DockViewB CLASSNAME;
	DockViewB();
	void OnCursorChanged(Point p) override;

private:
	Label   label_;
	Label   cursor_label_;
};

class DockViewC : public DockViewBase {
public:
	typedef DockViewC CLASSNAME;
	DockViewC();
	void OnCursorChanged(Point p) override;

private:
	Label   label_;
	Label   cursor_label_;
};

#endif
