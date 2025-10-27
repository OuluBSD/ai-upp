// U++-compatible Display wrapper for UI display operations
// This header is aggregated and wrapped into namespace Upp by CtrlCore.h

class Display {
public:
    // Virtual destructor for proper inheritance
    virtual ~Display() = default;

    // U++-style display operations
    virtual bool IsOpen() const { return true; }  // Basic display is always available
    virtual bool Is() const { return true; }      // Basic display always exists
    
    // U++-style display information
    virtual Size GetSize() const { 
        // In a real implementation, this would query the actual display size
        return Size(1920, 1080);  // Default HD resolution
    }
    
    virtual Point GetOffset() const { return Point(0, 0); }  // Top-left corner by default
    
    virtual Rect GetRect() const { 
        return Rect(GetOffset(), GetSize()); 
    }

    // U++-style display capabilities
    virtual int GetDpi() const { 
        // In a real implementation, this would query the actual DPI
        return 96;  // Standard DPI
    }
    
    virtual int GetWidth() const { return GetSize().cx; }
    virtual int GetHeight() const { return GetSize().cy; }

    // U++-style display types and properties
    virtual bool IsPrimary() const { return true; }  // Default display is primary
    virtual bool IsEnabled() const { return true; }  // Default display is enabled
    virtual bool IsActive() const { return true; }   // Default display is active
    
    virtual std::string GetId() const { return "Primary Display"; }
    virtual std::string GetName() const { return "Default Display"; }

    // U++-style coordinate transformations
    virtual Point ClientToScreen(const Point& pt) const { 
        return pt + GetOffset(); 
    }
    
    virtual Point ScreenToClient(const Point& pt) const { 
        return pt - GetOffset(); 
    }

    // Static methods for display enumeration (U++-style)
    static int GetCount() { 
        // In a real implementation, this would count available displays
        return 1; 
    }
    
    static Display* Get(int index) { 
        // In a real implementation, this would return the appropriate display
        // For now, return a singleton instance for the primary display
        static Display primary_display;
        return (index == 0) ? &primary_display : nullptr;
    }
    
    static Display* GetPrimary() { 
        return Get(0); 
    }

    // U++-style display operations
    virtual bool SetMode(int width, int height, int colorDepth = 32, int refreshRate = 60) {
        // In a real implementation, this would change the display mode
        return true;  // Assume success for basic implementation
    }
    
    virtual bool SetFullscreen(bool fullscreen = true) {
        // In a real implementation, this would set fullscreen mode
        return true;
    }

    // U++-style display features
    virtual bool HasTouch() const { 
        // In a real implementation, this would check for touch capability
        return false; 
    }
    
    virtual bool HasHardwareCursor() const { 
        // In a real implementation, this would check hardware cursor support
        return true; 
    }

    // U++-style color depth information
    virtual int GetColorDepth() const { 
        // In a real implementation, this would return actual color depth
        return 32; 
    }
    
    virtual int GetBitsPerPixel() const { 
        return GetColorDepth(); 
    }

    // U++-style refresh rate
    virtual int GetRefreshRate() const { 
        // In a real implementation, this would return actual refresh rate
        return 60; 
    }

    // U++-style orientation
    virtual int GetOrientation() const { 
        // 0 = normal, 1 = 90°, 2 = 180°, 3 = 270°
        return 0; 
    }
    
    virtual bool SetOrientation(int orientation) { 
        // In a real implementation, this would set display orientation
        return (orientation >= 0 && orientation <= 3);
    }

    // U++-style methods for identifying display types
    virtual const char* GetClassName() const { return "Display"; }
};

// Singleton instance for the primary display
inline Display primaryDisplay;