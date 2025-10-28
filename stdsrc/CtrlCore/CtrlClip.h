#ifndef _CtrlCore_CtrlClip_h_
#define _CtrlCore_CtrlClip_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"

// Clipboard and clipping region functionality for controls
class CtrlClip {
protected:
    Ctrl& ctrl;
    
public:
    explicit CtrlClip(Ctrl& c) : ctrl(c) {}
    
    // Copy text to clipboard
    static bool SetClipboard(const String& text);
    
    // Get text from clipboard
    static String GetClipboard();
    
    // Set clipboard with multiple format support (text, HTML, etc.)
    static bool SetClipboardMulti(const String& text, const String& html = String());
    
    // Check if clipboard contains text
    static bool HasClipboardText();
    
    // Clear clipboard
    static bool ClearClipboard();
    
    // Copy image to clipboard
    static bool SetClipboardImage(const Image& img);
    
    // Get image from clipboard
    static Image GetClipboardImage();
    
    // Check if clipboard contains an image
    static bool HasClipboardImage();
    
    // Copy custom data to clipboard
    template<typename T>
    static bool SetClipboardData(const T& data) {
        // In a real implementation, this would serialize the data
        return false;
    }
    
    // Get custom data from clipboard
    template<typename T>
    static T GetClipboardData() {
        // In a real implementation, this would deserialize the data
        return T{};
    }
    
    // Control-specific clipboard operations
    bool SetClipText(const String& text) { return SetClipboard(text); }
    String GetClipText() { return GetClipboard(); }
    bool HasClipText() { return HasClipboardText(); }
    bool SetClipImage(const Image& img) { return SetClipboardImage(img); }
    Image GetClipImage() { return GetClipboardImage(); }
    bool HasClipImage() { return HasClipboardImage(); }
    
    // Clipping region for drawing operations
    class ClipRegion {
    private:
        bool has_clip;
        Rect clip_rect;
        
    public:
        ClipRegion() : has_clip(false), clip_rect(0, 0, 0, 0) {}
        explicit ClipRegion(const Rect& r) : has_clip(true), clip_rect(r) {}
        
        bool HasClip() const { return has_clip; }
        const Rect& GetClipRect() const { return clip_rect; }
        
        void SetClipRect(const Rect& r) { 
            clip_rect = r; 
            has_clip = true; 
        }
        
        void ClearClip() { 
            has_clip = false; 
            clip_rect = Rect(0, 0, 0, 0); 
        }
        
        // Check if a point is within the clipping region
        bool IsInside(const Point& pt) const {
            return !has_clip || clip_rect.IsPtInside(pt);
        }
        
        // Check if a rectangle intersects the clipping region
        bool Intersects(const Rect& r) const {
            if (!has_clip) return true;
            return clip_rect.Intersects(r);
        }
        
        // Get intersection with clipping region
        Rect GetIntersection(const Rect& r) const {
            if (!has_clip) return r;
            return clip_rect.Intersect(r);
        }
    };
    
    // Set clipping region for drawing operations
    void SetClip(const Rect& r);
    void SetClip(const ClipRegion& clip);
    
    // Clear clipping region
    void ClearClip();
    
    // Get current clipping region
    ClipRegion GetClip() const;
    
    // Push/Pop clipping region stack (for nested clipping)
    void PushClip(const Rect& r);
    void PushClip(const ClipRegion& clip);
    void PopClip();
    int GetClipStackDepth() const;
    
    // Clipping utilities
    static Rect ClipRect(const Rect& rect, const Rect& clip_rect);
    static bool ClipLine(int& x1, int& y1, int& x2, int& y2, const Rect& clip_rect);
    static bool ClipPolygon(Vector<Point>& points, const Rect& clip_rect);
    
    // Region-based clipping
    class Region {
    private:
        Vector<Rect> rectangles;
        
    public:
        Region() = default;
        explicit Region(const Rect& r) { rectangles.Add(r); }
        
        void AddRect(const Rect& r);
        void SubtractRect(const Rect& r);
        void IntersectRect(const Rect& r);
        
        const Vector<Rect>& GetRects() const { return rectangles; }
        bool IsEmpty() const { return rectangles.IsEmpty(); }
        void Clear() { rectangles.Clear(); }
        
        // Check if point is in region
        bool IsPtInRegion(const Point& pt) const;
        
        // Check if rectangle intersects region
        bool Intersects(const Rect& r) const;
        
        // Get bounding box of region
        Rect GetBound() const;
    };
    
    // Set clipping region using a complex region
    void SetClipRegion(const Region& region);
    
    // Scroll control content
    bool Scroll(int dx, int dy);
    bool Scroll(const Point& delta);
    
    // Scroll specific area
    bool ScrollArea(const Rect& area, int dx, int dy);
    bool ScrollArea(const Rect& area, const Point& delta);
    
    // Get scroll offset
    Point GetScrollOffset() const;
    void SetScrollOffset(const Point& offset);
    
    // Set scrollable area
    void SetScrollSize(const Size& sz);
    void SetScrollSize(int cx, int cy);
    Size GetScrollSize() const;
    
    // Enable/disable scrolling
    void SetScroll(bool hscroll, bool vscroll);
    bool IsHScroll() const;
    bool IsVScroll() const;
    
    // Update scrollbars if they exist
    void UpdateScroll();
    
    // Scroll to make a specific rectangle visible
    void ScrollToRect(const Rect& r);
};

// Helper class for temporary clipping
class ClipScope {
private:
    Ctrl& ctrl;
    bool need_pop;
    
public:
    ClipScope(Ctrl& c, const Rect& clip_rect) : ctrl(c), need_pop(true) {
        ctrl.PushClip(clip_rect);
    }
    
    ClipScope(Ctrl& c, const CtrlClip::ClipRegion& clip) : ctrl(c), need_pop(true) {
        ctrl.PushClip(clip);
    }
    
    ~ClipScope() {
        if (need_pop) {
            ctrl.PopClip();
        }
    }
    
    void Cancel() { need_pop = false; }
};

// Global clipboard functions
inline bool SetClipboardText(const String& text) {
    return CtrlClip::SetClipboard(text);
}

inline String GetClipboardText() {
    return CtrlClip::GetClipboard();
}

inline bool SetClipboardImage(const Image& img) {
    return CtrlClip::SetClipboardImage(img);
}

inline Image GetClipboardImage() {
    return CtrlClip::GetClipboardImage();
}

// Macro for convenient clipping
#define WITH_CLIP(ctrl, rect) ClipScope _clip_scope_(ctrl, rect)

#endif