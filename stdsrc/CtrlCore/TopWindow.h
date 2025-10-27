// U++-compatible TopWindow wrapper for top-level windows
// This header is aggregated and wrapped into namespace Upp by CtrlCore.h

// Forward declarations
class Ctrl;
class Rect;
class Point;
class Size;

class TopWindow : public CtrlBase {
private:
    bool popup;
    bool main_window;
    bool iconize;
    bool maximize;
    std::string title;
    Image icon;
    
    // Platform-specific window handle
    void* platform_window_handle;

public:
    // Constructors
    TopWindow() : CtrlBase(), popup(false), main_window(false), 
                  iconize(false), maximize(false), platform_window_handle(nullptr) {
        title = "TopWindow";
    }

    // U++-style static constructors
    static TopWindow* Create() { return new TopWindow(); }
    
    // U++-style window properties
    TopWindow& Title(const std::string& t) { 
        title = t; 
        return *this; 
    }
    
    TopWindow& Icon(const Image& img) { 
        icon = img; 
        return *this; 
    }
    
    TopWindow& Popup(bool popup_window = true) { 
        popup = popup_window; 
        return *this; 
    }
    
    TopWindow& Main(bool main = true) { 
        main_window = main; 
        return *this; 
    }

    // U++-style window operations
    virtual void Open() {
        // In a real implementation, this would create and show the window
        // using the native platform's windowing system
    }
    
    virtual void Close() {
        // In a real implementation, this would close the window
    }
    
    virtual void Iconize() {
        iconize = true;
        // In a real implementation, this would minimize the window
    }
    
    virtual void Restore() {
        iconize = false;
        maximize = false;
        // In a real implementation, this would restore the window
    }
    
    virtual void Maximize() {
        maximize = true;
        // In a real implementation, this would maximize the window
    }

    // U++-style window state queries
    virtual bool IsOpen() const { 
        // In a real implementation, this would check if the window is visible
        return true; 
    }
    
    virtual bool IsIconized() const { return iconize; }
    virtual bool IsMaximized() const { return maximize; }
    virtual bool IsMain() const { return main_window; }
    virtual bool IsPopup() const { return popup; }

    // U++-style window operations
    virtual void Run() {
        Open(); // Show the window
        
        // In a real implementation, this would enter the window's event loop
        // This is where the UI would remain active until the window is closed
    }
    
    virtual void EventLoop() {
        // In a real implementation, this would process events for this window
        // until the window is closed
    }

    // U++-style window appearance
    virtual TopWindow& NoIcon() {
        // In a real implementation, this would hide the window icon
        return *this;
    }
    
    virtual TopWindow& NoSize() {
        // In a real implementation, this would make the window non-resizable
        return *this;
    }
    
    virtual TopWindow& NoCenter() {
        // In a real implementation, this would prevent auto-centering
        return *this;
    }
    
    virtual TopWindow& NoActivate() {
        // In a real implementation, this would prevent the window from activating
        return *this;
    }

    // U++-style window positioning
    virtual TopWindow& CenterScreen() {
        // In a real implementation, this would center the window on screen
        // Get screen size and center accordingly
        return *this;
    }
    
    virtual TopWindow& CenterRect(const Rect& r) {
        // In a real implementation, this would center relative to a rectangle
        return *this;
    }
    
    virtual TopWindow& ScreenRect(const Rect& r) {
        // In a real implementation, this would set the screen position
        return *this;
    }

    // U++-style operations
    virtual std::string GetTitle() const { return title; }
    virtual Image GetIcon() const { return icon; }
    
    virtual void SetTitle(const std::string& t) { title = t; }
    virtual void SetIcon(const Image& img) { icon = img; }

    // Platform-specific window handle management
    virtual void* GetPlatformWindowHandle() const { return platform_window_handle; }
    virtual void SetPlatformWindowHandle(void* handle) { platform_window_handle = handle; }

    // U++-style window operations for UI
    virtual void AcceptFiles(bool accept = true) {
        // In a real implementation, this would enable file drag-and-drop
    }

    // Inherit methods from parent class
    using CtrlBase::SetRect;
    using CtrlBase::GetRect;
    using CtrlBase::SetSize;
    using CtrlBase::GetSize;
    using CtrlBase::SetPos;
    using CtrlBase::GetPos;
    using CtrlBase::Show;
    using CtrlBase::Hide;

    // U++-style methods for identifying control types
    virtual const char* GetClassName() const override { return "TopWindow"; }
};