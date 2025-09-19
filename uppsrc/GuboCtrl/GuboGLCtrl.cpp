#include "GuboCtrl.h"

#ifdef GUI_X11
#include <GL/glx.h>
#include <GL/gl.h>
#endif

NAMESPACE_UPP

GuboGLCtrl::GuboGLCtrl() {
    BackPaint(EXCLUDEPAINT);
    top.SetFrameBox(CubfC(0,0,0,100,100,100));
    top.CreateGeom3DComponent();
}

GuboGLCtrl::~GuboGLCtrl() {
}

void GuboGLCtrl::AttachTop(TopGubo* t) {
    if (!t) return;
    // Replace internal top with provided one
    top = *t; // shallow copy OK if Pte; adjust if needed
}

bool GuboGLCtrl::EnsureGLContext() {
    if (gl_ready)
        return true;
    // TODO: create platform GL context for GetHWND()/native child
    gl_ready = true; // mark as ready for now
    return gl_ready;
}

void GuboGLCtrl::RenderGL() {
    Size sz = GetSize();
#ifdef GUI_X11
    if (x11.ctx) {
        glXMakeCurrent(Xdisplay, hwnd, (GLXContext)x11.ctx);
        glViewport(0, 0, sz.cx, sz.cy);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Basic fixed-function orthographic 3D for command replay
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Y-down ortho to match UI coordinates, generous depth range
        glOrtho(0, sz.cx, sz.cy, 0, -10000, 10000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        // Ensure draw commands are up to date
        if (top.IsPendingLayout())
            top.DeepLayout();
        top.Redraw(false);

        // Replay a minimal subset of 3D commands (currently BOX_OP)
        const DrawCommand3* begin = &top.GetCommandBegin();
        const DrawCommand3* end   = &top.GetCommandEnd();
        for (const DrawCommand3* it = begin ? begin->next : nullptr; it && it != end; it = it->next) {
            switch (it->type) {
            case DRAW3_BOX_OP: {
                // Draw front face of box at z depth with given color
                float x = it->pt.x;
                float y = it->pt.y;
                float z = it->pt.z;
                float w = it->sz.cx;
                float h = it->sz.cy;
                const Color& c = it->color;
                glColor4ub(c.GetR(), c.GetG(), c.GetB(), 255);
                glBegin(GL_QUADS);
                    glVertex3f(x,     y,     z);
                    glVertex3f(x+w,   y,     z);
                    glVertex3f(x+w,   y+h,   z);
                    glVertex3f(x,     y+h,   z);
                glEnd();
                break;
            }
            default:
                break;
            }
        }
        glXSwapBuffers(Xdisplay, hwnd);
    }
#else
    (void)sz;
#endif
}

void GuboGLCtrl::State(int reason) {
    DHCtrl::State(reason);
    if (reason == OPEN) {
        EnsureGLContext();
    } else if (reason == LAYOUTPOS || reason == LAYOUT) {
        Refresh();
    } else if (reason == CLOSE) {
        // TODO: destroy GL context
        gl_ready = false;
    }
}

void GuboGLCtrl::Layout() {
    Cubf c = CubfC(0,0,0, (float)GetSize().cx, (float)GetSize().cy, top.GetFrameBox().Depth());
    top.SetFrameBox(c);
}

void GuboGLCtrl::Paint(Draw& w) {
    if (!EnsureGLContext()) {
        w.DrawRect(GetSize(), LtGray());
        w.DrawText(8, 8, "OpenGL context not ready", StdFont(), Red());
        return;
    }
    RenderGL();
}

void GuboGLCtrl::ForwardMouse(int type, int code, Point p, int z, dword kf) {
    CtrlEvent e;
    e.type = type;
    e.n = (type == EVENT_MOUSEWHEEL ? z : code);
    e.pt = p;
    e.value = kf;
    top.Dispatch(e);
}

void GuboGLCtrl::MouseMove(Point p, dword kf) { ForwardMouse(EVENT_MOUSEMOVE, 0, p, 0, kf); }
void GuboGLCtrl::LeftDown(Point p, dword kf) { ForwardMouse(EVENT_MOUSE_EVENT, MOUSE_LEFTDOWN, p, 0, kf); }
void GuboGLCtrl::LeftUp(Point p, dword kf) { ForwardMouse(EVENT_MOUSE_EVENT, MOUSE_LEFTUP, p, 0, kf); }
void GuboGLCtrl::RightDown(Point p, dword kf) { ForwardMouse(EVENT_MOUSE_EVENT, MOUSE_RIGHTDOWN, p, 0, kf); }
void GuboGLCtrl::RightUp(Point p, dword kf) { ForwardMouse(EVENT_MOUSE_EVENT, MOUSE_RIGHTUP, p, 0, kf); }
void GuboGLCtrl::MouseWheel(Point p, int z, dword kf) { ForwardMouse(EVENT_MOUSEWHEEL, 0, p, z, kf); }

bool GuboGLCtrl::Key(dword key, int count) {
    CtrlEvent e;
    e.type = EVENT_KEYDOWN;
    e.value = key;
    e.n = count;
    return top.Dispatch(e);
}

#ifdef GUI_X11
XVisualInfo* GuboGLCtrl::CreateVisual() {
    int attr[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, None };
    XVisualInfo* vi = glXChooseVisual(Xdisplay, DefaultScreen(Xdisplay), attr);
    if (!vi) {
        SetError(true);
        SetErrorMessage("GuboGLCtrl: glXChooseVisual failed");
        return nullptr;
    }
    x11.vi = vi;
    return vi;
}

void GuboGLCtrl::SetAttributes(unsigned long &ValueMask, XSetWindowAttributes &attr) {
    if (x11.vi) {
        attr.colormap = XCreateColormap(Xdisplay, RootWindow(Xdisplay, x11.vi->screen), x11.vi->visual, AllocNone);
        ValueMask |= CWColormap;
    }
}

void GuboGLCtrl::AfterInit(bool Error) {
    if (Error)
        return;
    if (x11.vi && !x11.ctx) {
        GLXContext ctx = glXCreateContext(Xdisplay, x11.vi, NULL, True);
        if (!ctx) {
            SetError(true);
            SetErrorMessage("GuboGLCtrl: glXCreateContext failed");
            return;
        }
        x11.ctx = ctx;
        glXMakeCurrent(Xdisplay, hwnd, ctx);
        gl_ready = true;
    }
}

void GuboGLCtrl::BeforeTerminate() {
    if (x11.ctx) {
        glXMakeCurrent(Xdisplay, None, NULL);
        glXDestroyContext(Xdisplay, (GLXContext)x11.ctx);
        x11.ctx = nullptr;
    }
    if (x11.vi) {
        XFree(x11.vi);
        x11.vi = nullptr;
    }
}

void GuboGLCtrl::Resize(int w, int h) {
    // Adjust TopGubo depth unchanged; update width/height
    Cubf c = CubfC(0,0,0,(float)w,(float)h, top.GetFrameBox().Depth());
    top.SetFrameBox(c);
}
#endif

END_UPP_NAMESPACE
