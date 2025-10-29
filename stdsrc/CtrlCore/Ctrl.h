#pragma once
// U++-compatible Ctrl wrapper for UI controls
// This header is aggregated and wrapped into namespace Upp by CtrlCore.h

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>

// Include the actual definitions instead of forward declarations
#include "../Draw/Point.h"
#include "../Draw/Size.h"
#include "../Draw/Rect.h"
#include "../Draw/Color.h"
#include "../Draw/DrawCore.h"
#include "../Draw/Image.h"
#include "../Draw/Font.h"

// Forward declaration of event types
class Event;

namespace Upp {

class Ctrl {
protected:
    Rect rect;
    std::string label;
    std::weak_ptr<Ctrl> parent;
    std::vector<std::shared_ptr<Ctrl>> children;
    Color background_color;
    bool enabled;
    bool visible;
    std::string tooltip;
    
    // Event handlers
    std::function<void()> click_handler;
    std::function<void(const Event<>&)> event_handler;

public:
    // U++-style constants
    enum {
        TIMEID_COUNT = 1000,  // Standard U++ timer ID count
    };

    // Constructors
    Ctrl();
    
    explicit Ctrl(const Rect& r);

    // Virtual destructor for proper inheritance
    virtual ~Ctrl() = default;

    // U++-style size and position methods
    virtual void SetRect(const Rect& r);
    virtual void SetRect(int x, int y, int cx, int cy);
    
    virtual Rect GetRect() const;
    virtual Rect GetScreenRect() const;
    
    virtual Point GetScreenPoint(const Point& pt) const;
    
    virtual Size GetSize() const;
    virtual Point GetPos() const;
    
    virtual void SetSize(const Size& sz);
    virtual void SetSize(int cx, int cy);
    
    virtual void SetPos(const Point& pt);
    virtual void SetPos(int x, int y);

    // U++-style parent-child relationship
    virtual std::shared_ptr<Ctrl> GetParent() const;
    
    virtual void SetParent(const std::shared_ptr<Ctrl>& p);
    
    virtual void AddChild(const std::shared_ptr<Ctrl>& child);
    
    virtual void RemoveChild(const std::shared_ptr<Ctrl>& child);

    // U++-style visual properties
    virtual void SetLabel(const std::string& lbl);
    virtual std::string GetLabel() const;
    
    virtual void SetBackgroundColor(const Color& color);
    virtual Color GetBackgroundColor() const;
    
    virtual void SetEnabled(bool enable);
    virtual bool IsEnabled() const;
    
    virtual void SetVisible(bool show);
    virtual bool IsVisible() const;
    
    virtual void SetToolTip(const std::string& tip);
    virtual std::string GetToolTip() const;

    // U++-style painting
    virtual void Paint(Draw& draw) const;

    // U++-style event handling
    virtual void SetHandler(const std::function<void()>& handler);
    
    virtual void SetHandler(const std::function<void(const Event<>&)>& handler);

    virtual bool IsPointInside(const Point& pt) const;

    // U++-style control operations
    virtual void Show();
    virtual void Hide();
    
    virtual void Enable();
    virtual void Disable();

    // U++-style child control enumeration
    virtual int GetChildCount() const;
    virtual std::shared_ptr<Ctrl> GetChild(int index) const;

    // U++-style painting helper
    virtual void Refresh();
    
    virtual void Refresh(const Rect& r);

    // U++-style coordinate transformations
    virtual Point ClientToScreen(const Point& pt) const;
    
    virtual Point ScreenToClient(const Point& pt) const;

    // U++-style methods for identifying control types
    virtual const char* GetClassName() const;
    
    virtual bool IsNull() const;
    virtual bool Is() const;
    
    virtual bool IsOpen() const;  // Basic control always "open"

    // U++-style operations for UI updates
    virtual void SetFocus();
    
    virtual bool HasFocus() const;
    
    // U++-style timer handling
    enum {
        TIMEID_COUNT = 1000,  // Maximum number of timer IDs per control
    };

    virtual void SetTimeCallback(int delay_ms, const Event<>& cb, int id = 0);
    virtual void KillTimeCallback(int id = 0);
    virtual void KillSetTimeCallback(int delay_ms, const Event<>& cb, int id = 0);
    virtual bool ExistsTimeCallback(int id = 0) const;
    virtual void PostCallback(const Event<>& cb, int id = 0);
    virtual void KillPostCallback(const Event<>& cb, int id = 0);
    
    // U++-style keyboard handling
    virtual bool Key(dword key, int count);
    virtual void GotFocus();
    virtual void LostFocus();
    virtual bool HotKey(dword key);
    virtual dword GetAccessKeys() const;
    virtual void AssignAccessKeys(dword used);
    virtual void ChildGotFocus();
    virtual void ChildLostFocus();
    
    // Focus-related methods
    virtual bool HasFocusDeep() const;
    virtual Ctrl* GetFocusChild() const;
    virtual Ctrl* GetFocusChildDeep() const;
    Ctrl& WantFocus(bool ft = true);
    Ctrl& NoWantFocus();
    bool IsWantFocus() const;
    bool SetWantFocus();
    Ctrl& InitFocus(bool ft = true);
    Ctrl& NoInitFocus();
    bool IsInitFocus() const;
    
    // Access keys support
    void RefreshAccessKeys();
    void RefreshAccessKeysDo(bool vis);
    static dword AccessKeyBit(int accesskey);
    dword GetAccessKeysDeep() const;
    void DistributeAccessKeys();
    bool VisibleAccessKeys();
    
    // Static focus management
    static Ctrl* GetFocusCtrl();
    static bool IterateFocusForward(Ctrl *ctrl, Ctrl *top, bool noframe = false, bool init = false, bool all = false);
    static bool IterateFocusBackward(Ctrl *ctrl, Ctrl *top, bool noframe = false, bool all = false);

    // U++-style operations for UI layout
    virtual void SetFrame(int left, int top, int right, int bottom);
    
    virtual void SetFrameRect(const Rect& frame);
    
    virtual Rect GetFrameRect() const;
};

// Enable Ctrl to be used with shared_ptr by inheriting from enable_shared_from_this
class CtrlBase : public Ctrl, public std::enable_shared_from_this<CtrlBase> {
public:
    CtrlBase() = default;
    explicit CtrlBase(const Rect& r) : Ctrl(r) {}
};

}