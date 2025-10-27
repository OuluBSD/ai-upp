// Test for stdsrc CtrlCore functionality

#include "CtrlCore/CtrlCore.h"

using namespace Upp;

CONSOLE_APP_MAIN {
    // Test Ctrl functionality
    {
        CtrlBase ctrl;
        
        // Test initial state
        Rect initialRect = ctrl.GetRect();
        ASSERT(initialRect.left == 0);
        ASSERT(initialRect.top == 0);
        ASSERT(initialRect.right == 100);
        ASSERT(initialRect.bottom == 100);
        
        // Test size and position
        ctrl.SetRect(10, 20, 110, 220);
        Rect newRect = ctrl.GetRect();
        ASSERT(newRect.left == 10);
        ASSERT(newRect.top == 20);
        ASSERT(newRect.right == 110);
        ASSERT(newRect.bottom == 220);
        
        Size size = ctrl.GetSize();
        ASSERT(size.cx == 100);
        ASSERT(size.cy == 200);
        
        Point pos = ctrl.GetPos();
        ASSERT(pos.x == 10);
        ASSERT(pos.y == 20);
        
        ctrl.SetSize(150, 250);
        ASSERT(ctrl.GetSize().cx == 150);
        ASSERT(ctrl.GetSize().cy == 250);
        
        ctrl.SetPos(5, 15);
        ASSERT(ctrl.GetPos().x == 5);
        ASSERT(ctrl.GetPos().y == 15);
        
        // Test visual properties
        ctrl.SetLabel("Test Control");
        ASSERT(ctrl.GetLabel() == "Test Control");
        
        Color bgColor(200, 200, 255);
        ctrl.SetBackgroundColor(bgColor);
        ASSERT(ctrl.GetBackgroundColor().GetR() == 200);
        ASSERT(ctrl.GetBackgroundColor().GetG() == 200);
        ASSERT(ctrl.GetBackgroundColor().GetB() == 255);
        
        ctrl.SetEnabled(true);
        ASSERT(ctrl.IsEnabled());
        
        ctrl.SetEnabled(false);
        ASSERT(!ctrl.IsEnabled());
        
        ctrl.SetVisible(true);
        ASSERT(ctrl.IsVisible());
        
        ctrl.SetVisible(false);
        ASSERT(!ctrl.IsVisible());
        
        ctrl.SetToolTip("This is a test tooltip");
        ASSERT(ctrl.GetToolTip() == "This is a test tooltip");
        
        // Test point inside
        ctrl.SetRect(0, 0, 100, 100);
        ASSERT(ctrl.IsPointInside(Point(50, 50)));
        ASSERT(!ctrl.IsPointInside(Point(150, 150)));
        
        // Test visibility operations
        ctrl.Show();
        ASSERT(ctrl.IsVisible());
        
        ctrl.Hide();
        ASSERT(!ctrl.IsVisible());
        
        ctrl.Show();
        ASSERT(ctrl.IsVisible());
        
        ctrl.Disable();
        ASSERT(!ctrl.IsEnabled());
        
        ctrl.Enable();
        ASSERT(ctrl.IsEnabled());
        
        // Test class name
        ASSERT(std::string(ctrl.GetClassName()) == "Ctrl");
        
        // Test null/is
        ASSERT(ctrl.Is());
        ASSERT(!ctrl.IsNull());
        
        RLOG("Ctrl tests passed!");
    }
    
    // Test TopWindow functionality
    {
        TopWindow win;
        
        // Test initial state
        ASSERT(!win.IsMain());
        ASSERT(!win.IsPopup());
        ASSERT(!win.IsIconized());
        ASSERT(!win.IsMaximized());
        
        // Test title
        win.Title("Test Window");
        ASSERT(win.GetTitle() == "Test Window");
        
        // Test main/popup
        TopWindow mainWin;
        mainWin.Main();
        ASSERT(mainWin.IsMain());
        
        TopWindow popupWin;
        popupWin.Popup();
        ASSERT(popupWin.IsPopup());
        
        // Test size and position
        win.SetRect(100, 100, 500, 400);
        ASSERT(win.GetRect().GetWidth() == 400);
        ASSERT(win.GetRect().GetHeight() == 300);
        
        // Test operations
        ASSERT(win.IsOpen());  // Always true in our basic implementation
        
        win.Iconize();
        ASSERT(win.IsIconized());
        
        win.Restore();
        ASSERT(!win.IsIconized());
        ASSERT(!win.IsMaximized());
        
        win.Maximize();
        ASSERT(win.IsMaximized());
        
        win.Restore();
        ASSERT(!win.IsMaximized());
        
        // Test class name
        ASSERT(std::string(win.GetClassName()) == "TopWindow");
        
        RLOG("TopWindow tests passed!");
    }
    
    // Test Display functionality
    {
        Display display;
        
        // Test basic properties
        ASSERT(display.Is());
        ASSERT(display.IsOpen());
        ASSERT(display.IsPrimary());
        ASSERT(display.IsEnabled());
        ASSERT(display.IsActive());
        
        Size size = display.GetSize();
        ASSERT(size.cx == 1920);  // Default HD resolution
        ASSERT(size.cy == 1080);
        
        Point offset = display.GetOffset();
        ASSERT(offset.x == 0);
        ASSERT(offset.y == 0);
        
        Rect rect = display.GetRect();
        ASSERT(rect.GetWidth() == 1920);
        ASSERT(rect.GetHeight() == 1080);
        
        int dpi = display.GetDpi();
        ASSERT(dpi == 96);  // Standard DPI
        
        int width = display.GetWidth();
        int height = display.GetHeight();
        ASSERT(width == 1920);
        ASSERT(height == 1080);
        
        // Test display enumeration
        int count = Display::GetCount();
        ASSERT(count >= 1);
        
        Display* primary = Display::GetPrimary();
        ASSERT(primary != nullptr);
        
        Display* other = Display::Get(0);
        ASSERT(other != nullptr);
        ASSERT(other == primary);
        
        // Test invalid display
        Display* invalid = Display::Get(count);  // Out of range
        ASSERT(invalid == nullptr);
        
        // Test features
        ASSERT(!display.HasTouch());  // Default implementation
        ASSERT(display.HasHardwareCursor());
        
        // Test color depth
        int colorDepth = display.GetColorDepth();
        ASSERT(colorDepth == 32);
        ASSERT(display.GetBitsPerPixel() == 32);
        
        // Test refresh rate
        int refreshRate = display.GetRefreshRate();
        ASSERT(refreshRate == 60);
        
        // Test orientation
        int orientation = display.GetOrientation();
        ASSERT(orientation == 0);  // Normal orientation
        
        bool setOrient = display.SetOrientation(1);  // 90 degrees
        ASSERT(setOrient);
        
        setOrient = display.SetOrientation(4);  // Invalid
        ASSERT(!setOrient);
        
        // Test class name
        ASSERT(std::string(display.GetClassName()) == "Display");
        
        RLOG("Display tests passed!");
    }
    
    // Test Event functionality
    {
        // Test basic event
        Event evt1;
        ASSERT(evt1.GetType() == Event::UNKNOWN);
        ASSERT(!evt1.Is());
        ASSERT(evt1.IsNull());
        
        // Test mouse events
        Point mousePos(100, 200);
        Event mouseDown = Event::MouseLeftDown(mousePos);
        ASSERT(mouseDown.GetType() == Event::MOUSE_LEFT_DOWN);
        ASSERT(mouseDown.GetMousePos().x == 100);
        ASSERT(mouseDown.GetMousePos().y == 200);
        ASSERT(mouseDown.GetMouseX() == 100);
        ASSERT(mouseDown.GetMouseY() == 200);
        ASSERT(mouseDown.IsMouse());
        ASSERT(mouseDown.IsLeftDown());
        
        Event mouseMove = Event::MouseMove(mousePos);
        ASSERT(mouseMove.GetType() == Event::MOUSE_MOVE);
        ASSERT(mouseMove.IsMouseMove());
        
        Event mouseWheel = Event::MouseWheel(mousePos, 120);
        ASSERT(mouseWheel.GetType() == Event::MOUSE_WHEEL);
        ASSERT(mouseWheel.GetMouseWheelDelta() == 120);
        ASSERT(mouseWheel.IsMouseWheel());
        
        // Test key events
        Event keyPress = Event::KeyPress(65);  // 'A' key
        ASSERT(keyPress.GetType() == Event::KEY_PRESS);
        ASSERT(keyPress.GetKeyCode() == 65);
        ASSERT(keyPress.IsKey());
        
        Event keyChar = Event::KeyChar('B');
        ASSERT(keyChar.GetType() == Event::KEY_CHAR);
        ASSERT(keyChar.GetKeyCode() == static_cast<int>('B'));
        ASSERT(keyChar.GetTextInput() == "B");
        ASSERT(keyChar.IsKey());
        
        // Test modifiers
        ASSERT(!keyChar.IsShift());
        ASSERT(!keyChar.IsCtrl());
        ASSERT(!keyChar.IsAlt());
        ASSERT(!keyChar.IsCmd());
        ASSERT(!keyChar.IsMeta());
        
        // Test paint and close events
        Event paintEvt = Event::Paint();
        ASSERT(paintEvt.GetType() == Event::PAINT);
        ASSERT(paintEvt.IsPaint());
        
        Event closeEvt = Event::Close();
        ASSERT(closeEvt.GetType() == Event::CLOSE);
        ASSERT(closeEvt.IsClose());
        
        // Test event handling
        ASSERT(!paintEvt.IsHandled());
        paintEvt.SetHandled();
        ASSERT(paintEvt.IsHandled());
        paintEvt.Handled();  // Convenience method
        ASSERT(paintEvt.IsHandled());
        
        // Test event type names
        ASSERT(std::string(mouseDown.GetTypeName()) == "MOUSE_LEFT_DOWN");
        ASSERT(std::string(keyPress.GetTypeName()) == "KEY_PRESS");
        ASSERT(std::string(paintEvt.GetTypeName()) == "PAINT");
        
        RLOG("Event tests passed!");
    }
    
    RLOG("All CtrlCore tests passed successfully!");
}