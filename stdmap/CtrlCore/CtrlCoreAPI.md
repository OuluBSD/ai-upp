# U++ CtrlCore Package API Analysis

## Overview
The U++ CtrlCore package provides the fundamental GUI framework including windowing, input handling, event loops, and platform backends. It serves as the foundation for all GUI elements in U++ applications.

## Core Classes and Types

### Ctrl Class
The base class for all GUI controls and windows, implementing the core functionality for drawing, event handling, and layout management.

```cpp
class Ctrl : public Pte<Ctrl> {
    // Layout management
    void SetRect(const Rect& r);
    void SetPos(LogPos p);
    Rect GetRect() const;
    Rect GetScreenRect() const;
    Size GetSize() const;
    void Layout();
    Size GetMinSize() const;
    Size GetMaxSize() const;
    Size GetStdSize() const;
    
    // Drawing and refreshing
    void Paint(Draw& w);
    void Refresh(const Rect& r);
    void Refresh();
    void Update();
    void UpdateLayout();
    int OverPaint() const;
    
    // Child management
    void AddChild(Ctrl *child);
    void RemoveChild(Ctrl *child);
    Ctrl *GetParent() const;
    Ctrl *GetFirstChild() const;
    Ctrl *GetLastChild() const;
    int GetChildIndex(const Ctrl *child) const;
    
    // Mouse handling
    void MouseEnter(Point p, dword keyflags);
    void MouseMove(Point p, dword keyflags);
    void LeftDown(Point p, dword keyflags);
    void LeftUp(Point p, dword keyflags);
    void RightDown(Point p, dword keyflags);
    void RightUp(Point p, dword keyflags);
    void MouseWheel(Point p, int zdelta, dword keyflags);
    Image MouseEvent(int event, Point p, int zdelta, dword keyflags);
    
    // Keyboard handling
    bool Key(dword key, int count);
    bool HotKey(dword key);
    dword GetAccessKeys() const;
    
    // Focus management
    bool SetFocus();
    bool HasFocus() const;
    Ctrl *GetFocusCtrl();
    void GotFocus();
    void LostFocus();
    Ctrl& WantFocus(bool ft = true);
    
    // Visibility and state
    void Show(bool show = true);
    void Hide();
    bool IsShown() const;
    void Enable(bool enable = true);
    void Disable();
    bool IsEnabled() const;
    bool IsOpen() const;
    
    // Data management
    void SetData(const Value& data);
    Value GetData() const;
    bool IsModified() const;
    
    // Drag and drop support
    void DragAndDrop(Point p, PasteClip& d);
    int DoDragAndDrop(const char *fmts, const Image& sample, dword actions,
                      const VectorMap<String, ClipData>& data);
    
    // Frame support
    Ctrl& SetFrame(int i, CtrlFrame& frm);
    Ctrl& AddFrame(CtrlFrame& frm);
    int GetFrameCount() const;
    void ClearFrames();
    
    // Event loop management
    static void EventLoop(Ctrl *loopctrl = NULL);
    static bool ProcessEvent(bool *quit = NULL);
    static bool ProcessEvents(bool *quit = NULL);
    void EndLoop();
    void EndLoop(int code);
    
    // Clipboard operations
    static PasteClip& Clipboard();
    static PasteClip& Selection();
    static void ClearClipboard();
    static void AppendClipboardText(const String& s);
    static String ReadClipboardText();
    
    // Top-level window operations
    void PopUp(Ctrl *owner = NULL, bool savebits = true, bool activate = true, 
               bool dropshadow = false, bool topmost = false);
    bool IsPopUp() const;
    
    // Static methods for global properties
    static Size GetVirtualScreenArea();
    static Rect GetPrimaryWorkArea();
    static Point GetMousePos();
    static bool GetShift();
    static bool GetCtrl();
    static bool GetAlt();
    
    // Timers
    void SetTimeCallback(int delay_ms, Function<void ()> cb, int id = 0);
    void KillTimeCallback(int id = 0);
    
    // Other methods...
};
```

### CtrlFrame Class
Abstract base class for frame decorations around controls.

```cpp
class CtrlFrame {
    virtual void FrameLayout(Rect& r) = 0;
    virtual void FramePaint(Draw& w, const Rect& r);
    virtual void FrameAddSize(Size& sz) = 0;
    virtual void FrameAdd(Ctrl& parent);
    virtual void FrameRemove();
    virtual int OverPaint() const;
};
```

### TopWindow Class
Represents top-level windows with window management capabilities.

```cpp
class TopWindow : public Ctrl {
    // Window management
    void Open(Ctrl *owner);
    void Open();
    int Run(bool appmodal = false);
    int Execute();
    void Break(int ID = IDEXIT);
    bool AcceptBreak(int ID);
    void RejectBreak(int ID);
    
    // Window state
    void Minimize(bool effect = false);
    void Maximize(bool effect = false);
    bool IsMaximized() const;
    bool IsMinimized() const;
    
    // Window properties
    TopWindow& Title(const WString& _title);
    TopWindow& Sizeable(bool b = true);
    TopWindow& MinimizeBox(bool b = true);
    TopWindow& MaximizeBox(bool b = true);
    TopWindow& ToolWindow(bool b = true);
    TopWindow& TopMost(bool b = true, bool stay_top = true);
    TopWindow& Background(const PaintRect& prect);
    TopWindow& Icon(const Image& m);
    
    // Layout
    virtual Size GetMinSize() const;
    virtual Size GetStdSize() const;
    virtual void SetMinSize(Size sz);
    
    // Event handling
    virtual void Activate();
    virtual void Deactivate();
    virtual void Close();
    virtual bool Accept();
    virtual void Reject();
    
    // Other methods...
};
```

### PasteClip Class
Handles clipboard operations and drag-and-drop functionality.

```cpp
class PasteClip {
    bool IsAvailable(const char *fmt) const;
    String Get(const char *fmt) const;
    bool Accept();
    bool Accept(const char *fmt);
    void Reject();
    int GetAction() const;
    int GetAllowedActions() const;
    bool IsAccepted() const;
};
```

### TimeCallback Class
Wrapper for timer callbacks.

```cpp
class TimeCallback {
    void Set(int delay, Function<void ()> cb);
    void Post(Function<void ()> cb);
    void Kill();
    void KillSet(int delay, Function<void ()> cb);
    void KillPost(Function<void ()> cb);
};
```

### RectTracker Class
Interactive rectangle tracking for resize operations.

```cpp
class RectTracker : public LocalLoop {
    Rect Track(const Rect& r, int tx = ALIGN_RIGHT, int ty = ALIGN_BOTTOM);
    int TrackHorzLine(int x0, int y0, int cx, int line);
    int TrackVertLine(int x0, int y0, int cy, int line);
    RectTracker& MinSize(Size sz);
    RectTracker& MaxSize(Size sz);
    RectTracker& MaxRect(const Rect& mr);
    RectTracker& Animation(int step_ms = 40);
    Rect Get();
};
```

## Key Functions and Global Operations

### Mouse and Keyboard Input
```cpp
Point GetMousePos();
dword GetMouseFlags();
bool GetShift();
bool GetCtrl();
bool GetAlt();
bool GetCapsLock();
String GetKeyDesc(dword key);
```

### Clipboard Operations
```cpp
void ClearClipboard();
void AppendClipboard(const char *format, const String& data);
String ReadClipboard(const char *format);
void AppendClipboardText(const String& s);
String ReadClipboardText();
bool IsClipboardAvailableText();
```

### Event Loop Control
```cpp
void EventLoop(Ctrl *loopctrl = NULL);
bool ProcessEvent(bool *quit = NULL);
bool ProcessEvents(bool *quit = NULL);
void SetTimeCallback(int delay_ms, Function<void ()> cb, void *id = NULL);
void KillTimeCallback(void *id);
```

### GUI State Management
```cpp
void SetUHDEnabled(bool set = true);
bool IsUHDEnabled();
void SetDarkThemeEnabled(bool set = true);
bool IsDarkThemeEnabled();
void ReSkin();
void PostReSkin();
```

## Potential STL Mappings

| U++ Element | Potential STL Equivalent | Status | Notes |
|-------------|-------------------------|--------|-------|
| Ctrl | No direct equivalent | ❌ | GUI control hierarchy element, no STL equivalent |
| CtrlFrame | No direct equivalent | ❌ | GUI frame abstraction, no STL equivalent |
| TopWindow | No direct equivalent | ❌ | Top-level window implementation, no STL equivalent |
| PasteClip | No direct equivalent | ❌ | Clipboard/drag-drop abstraction, no STL equivalent |
| TimeCallback | std::function + timer | ⚠️ | Could implement using std::function and threading |
| RectTracker | No direct equivalent | ❌ | GUI-specific interaction element |
| GuiLock | std::lock_guard | ✓ | Maps to std::lock_guard<std::mutex> |
| GuiUnlock | Custom wrapper | ⚠️ | Would need custom implementation |
| Event<> | std::function/boost::signals2 | ⚠️ | Could map to std::function or signals library |
| Callback | std::function | ✓ | Maps directly to std::function |
| TimeCallback | std::function + threading | ⚠️ | Would need custom implementation with threading |

## Summary
The CtrlCore package is fundamental to U++'s GUI system and provides the base classes for all GUI elements. It contains platform-specific implementations for Windows, X11, and Cocoa, making it complex to map to STL equivalents. While some components like GuiLock can map directly to STL equivalents (std::lock_guard), most of the functionality is GUI-specific and has no direct STL equivalent. The event system, window management, and input handling would require significant custom implementation using STL containers as building blocks rather than direct mappings.