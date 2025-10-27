// U++-compatible Ctrl wrapper for UI controls
// This header is aggregated and wrapped into namespace Upp by CtrlCore.h

// Forward declarations
class Point;
class Size;
class Rect;
class Color;
class Draw;
class Image;
class Font;

// Forward declaration of event types
class Event;

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
    std::function<void(const Event&)> event_handler;

public:
    // Constructors
    Ctrl() : rect(0, 0, 100, 100), background_color(Color::White()), 
             enabled(true), visible(true) {}
    
    explicit Ctrl(const Rect& r) : rect(r), background_color(Color::White()), 
                                   enabled(true), visible(true) {}

    // Virtual destructor for proper inheritance
    virtual ~Ctrl() = default;

    // U++-style size and position methods
    virtual void SetRect(const Rect& r) { rect = r; }
    virtual void SetRect(int x, int y, int cx, int cy) { SetRect(Rect(x, y, cx, cy)); }
    
    virtual Rect GetRect() const { return rect; }
    virtual Rect GetScreenRect() const { 
        // In a real implementation, this would calculate screen coordinates
        return rect; 
    }
    
    virtual Point GetScreenPoint(const Point& pt) const { 
        // In a real implementation, this would convert to screen coordinates
        return pt + rect.GetTopLeft(); 
    }
    
    virtual Size GetSize() const { return rect.GetSize(); }
    virtual Point GetPos() const { return rect.GetTopLeft(); }
    
    virtual void SetSize(const Size& sz) { 
        rect.right = rect.left + sz.cx;
        rect.bottom = rect.top + sz.cy;
    }
    
    virtual void SetSize(int cx, int cy) { SetSize(Size(cx, cy)); }
    
    virtual void SetPos(const Point& pt) { 
        Point current_pos = rect.GetTopLeft();
        int dx = pt.x - current_pos.x;
        int dy = pt.y - current_pos.y;
        rect.left += dx;
        rect.right += dx;
        rect.top += dy;
        rect.bottom += dy;
    }
    
    virtual void SetPos(int x, int y) { SetPos(Point(x, y)); }

    // U++-style parent-child relationship
    virtual std::shared_ptr<Ctrl> GetParent() const { 
        return parent.lock(); 
    }
    
    virtual void SetParent(const std::shared_ptr<Ctrl>& p) { 
        parent = p; 
    }
    
    virtual void AddChild(const std::shared_ptr<Ctrl>& child) {
        if (child) {
            children.push_back(child);
            child->SetParent(shared_from_this());
        }
    }
    
    virtual void RemoveChild(const std::shared_ptr<Ctrl>& child) {
        children.erase(
            std::remove(children.begin(), children.end(), child),
            children.end()
        );
        if (child) {
            child->SetParent(nullptr);
        }
    }

    // U++-style visual properties
    virtual void SetLabel(const std::string& lbl) { label = lbl; }
    virtual std::string GetLabel() const { return label; }
    
    virtual void SetBackgroundColor(const Color& color) { background_color = color; }
    virtual Color GetBackgroundColor() const { return background_color; }
    
    virtual void SetEnabled(bool enable) { enabled = enable; }
    virtual bool IsEnabled() const { return enabled; }
    
    virtual void SetVisible(bool show) { visible = show; }
    virtual bool IsVisible() const { return visible; }
    
    virtual void SetToolTip(const std::string& tip) { tooltip = tip; }
    virtual std::string GetToolTip() const { return tooltip; }

    // U++-style painting
    virtual void Paint(Draw& draw) const {
        // Default implementation just paints background
        if (visible) {
            draw.DrawRect(rect, background_color);
            // Render label text if present
            if (!label.empty()) {
                // In a real implementation, this would use proper text rendering
            }
        }
    }

    // U++-style event handling
    virtual void SetHandler(const std::function<void()>& handler) { 
        click_handler = handler; 
    }
    
    virtual void SetHandler(const std::function<void(const Event&)>& handler) { 
        event_handler = handler; 
    }

    virtual bool IsPointInside(const Point& pt) const {
        return rect.IsPtInside(pt);
    }

    // U++-style control operations
    virtual void Show() { SetVisible(true); }
    virtual void Hide() { SetVisible(false); }
    
    virtual void Enable() { SetEnabled(true); }
    virtual void Disable() { SetEnabled(false); }

    // U++-style child control enumeration
    virtual int GetChildCount() const { return static_cast<int>(children.size()); }
    virtual std::shared_ptr<Ctrl> GetChild(int index) const {
        if (index >= 0 && index < static_cast<int>(children.size())) {
            return children[index];
        }
        return nullptr;
    }

    // U++-style painting helper
    virtual void Refresh() {
        // In a real implementation, this would trigger a repaint
    }
    
    virtual void Refresh(const Rect& r) {
        // In a real implementation, this would trigger a partial repaint
    }

    // U++-style coordinate transformations
    virtual Point ClientToScreen(const Point& pt) const {
        return pt + rect.GetTopLeft();
    }
    
    virtual Point ScreenToClient(const Point& pt) const {
        return pt - rect.GetTopLeft();
    }

    // U++-style methods for identifying control types
    virtual const char* GetClassName() const { return "Ctrl"; }
    
    virtual bool IsNull() const { return false; }
    virtual bool Is() const { return true; }
    
    virtual bool IsOpen() const { return Is(); }  // Basic control always "open"

    // U++-style operations for UI updates
    virtual void SetFocus() {
        // In a real implementation, this would set focus to the control
    }
    
    virtual bool HasFocus() const { 
        // In a real implementation, this would check if control has focus
        return false; 
    }

    // U++-style operations for UI layout
    virtual void SetFrame(int left, int top, int right, int bottom) {
        // In a real implementation, this would set frame margins
    }
    
    virtual void SetFrameRect(const Rect& frame) {
        SetFrame(frame.GetLeft(), frame.GetTop(), frame.GetRight(), frame.GetBottom());
    }
    
    virtual Rect GetFrameRect() const {
        // In a real implementation, this would return frame margins
        return Rect(0, 0, 0, 0);
    }
};

// Enable Ctrl to be used with shared_ptr by inheriting from enable_shared_from_this
class CtrlBase : public Ctrl, public std::enable_shared_from_this<CtrlBase> {
public:
    CtrlBase() = default;
    explicit CtrlBase(const Rect& r) : Ctrl(r) {}
};