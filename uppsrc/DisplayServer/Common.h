#ifndef _DisplayServer_Common_h_
#define _DisplayServer_Common_h_

#include <Ctrl/Display/Display.h>

NAMESPACE_UPP

// Set once at startup by ParseVerboseFlag(), read by the logging helpers below.
extern bool g_verbose;
void ParseVerboseFlag();

// One line per window paint / composite pass, used by both backends
// (NetworkDisplay/0007 will consume this log to confirm drawing happened without
// visually inspecting the screen).
void LogPaint(const char *backend, int window_id, const String& title, Rect rect);
void LogComposite(const char *backend, int window_count);

struct WindowSpec : Moveable<WindowSpec> {
	String title;
	Color  color;
	int    shape; // 0 = plain, 1 = ellipse accent, 2 = cross accent

	WindowSpec() : shape(0) {}
	WindowSpec(const char *title, Color color, int shape) : title(title), color(color), shape(shape) {}
};

Vector<WindowSpec> DefaultWindowSpecs();

// Shared content painter (solid color + shape + title), driven through the plain
// Draw-shaped API so it works unmodified against both plain Draw (software backend)
// and GLDraw (OpenGL backend, which implements the same Draw-derived interface).
void PaintSyntheticContent(Draw& w, Size sz, const WindowSpec& spec, bool active);

#ifdef flagGUI
// Software backend only: a Ctrl hosted inside a FrameT<CtxUpp2D>'s client area.
class SyntheticContent : public Ctrl {
public:
	typedef SyntheticContent CLASSNAME;

	int            id = -1;
	WindowSpec     spec;
	WindowManager* scope = NULL;

	void Paint(Draw& w) override;
	void LeftDown(Point p, dword keyflags) override;
};

// The client (content) rect in the frame's own local coordinate space: (0,0)-based,
// so it is valid both to position a child Ctrl inside the frame (software backend)
// and to draw directly once a Draw/GLDraw has been offset+clipped to the frame's
// box (OpenGL backend's manual compositor).
Rect FrameClientLocal(WindowManager::Frame& h);
#endif

END_UPP_NAMESPACE

#endif
