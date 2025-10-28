#pragma once
// U++-compatible Event wrapper for UI events
// This header is aggregated and wrapped into namespace Upp by CtrlCore.h

#include "../Draw/Point.h"

class Event {
public:
    enum Type {
        UNKNOWN = 0,
        MOUSE_LEFT_DOWN,
        MOUSE_LEFT_UP,
        MOUSE_LEFT_DOUBLE,
        MOUSE_RIGHT_DOWN,
        MOUSE_RIGHT_UP,
        MOUSE_RIGHT_DOUBLE,
        MOUSE_MIDDLE_DOWN,
        MOUSE_MIDDLE_UP,
        MOUSE_MIDDLE_DOUBLE,
        MOUSE_WHEEL,
        MOUSE_MOVE,
        KEY_PRESS,
        KEY_RELEASE,
        KEY_CHAR,
        PAINT,
        ACTIVATE,
        DEACTIVATE,
        GAIN_FOCUS,
        LOSE_FOCUS,
        SIZE,
        MOVE,
        CLOSE,
        DESTROY,
        TIMER,
        USER
    };

    enum KeyModifier {
        NONE = 0,
        SHIFT = 1 << 0,
        CTRL = 1 << 1,
        ALT = 1 << 2,
        CMD = 1 << 3,  // Command key on Mac
        META = 1 << 4  // Meta key (Windows key on Windows)
    };

private:
    Type event_type;
    Point mouse_pos;
    int mouse_wheel_delta;
    int key_code;
    int key_modifier;
    std::string text_input;
    bool handled;

public:
    // Constructors
    Event() : event_type(UNKNOWN), mouse_pos(0, 0), mouse_wheel_delta(0), 
              key_code(0), key_modifier(NONE), handled(false) {}
    
    Event(Type type) : event_type(type), mouse_pos(0, 0), mouse_wheel_delta(0), 
                       key_code(0), key_modifier(NONE), handled(false) {}

    // U++-style event creation methods
    static Event MouseLeftDown(const Point& pos) {
        Event e(MOUSE_LEFT_DOWN);
        e.mouse_pos = pos;
        return e;
    }
    
    static Event MouseLeftUp(const Point& pos) {
        Event e(MOUSE_LEFT_UP);
        e.mouse_pos = pos;
        return e;
    }
    
    static Event MouseMove(const Point& pos) {
        Event e(MOUSE_MOVE);
        e.mouse_pos = pos;
        return e;
    }
    
    static Event MouseWheel(const Point& pos, int delta) {
        Event e(MOUSE_WHEEL);
        e.mouse_pos = pos;
        e.mouse_wheel_delta = delta;
        return e;
    }
    
    static Event KeyPress(int keyCode, int modifiers = NONE) {
        Event e(KEY_PRESS);
        e.key_code = keyCode;
        e.key_modifier = modifiers;
        return e;
    }
    
    static Event KeyChar(char c, int modifiers = NONE) {
        Event e(KEY_CHAR);
        e.key_code = static_cast<int>(c);
        e.key_modifier = modifiers;
        e.text_input = std::string(1, c);
        return e;
    }
    
    static Event Paint() {
        return Event(PAINT);
    }
    
    static Event Close() {
        return Event(CLOSE);
    }
    
    static Event Size(int width, int height) {
        Event e(SIZE);
        // Store width/height in key_code/key_modifier as a simple approach
        // In a real implementation, we'd have dedicated fields
        return e;
    }

    // U++-style accessors
    Type GetType() const { return event_type; }
    Point GetMousePos() const { return mouse_pos; }
    int GetMouseWheelDelta() const { return mouse_wheel_delta; }
    int GetKeyCode() const { return key_code; }
    int GetKeyModifier() const { return key_modifier; }
    std::string GetTextInput() const { return text_input; }

    // U++-style position accessors
    int GetMouseX() const { return mouse_pos.x; }
    int GetMouseY() const { return mouse_pos.y; }

    // U++-style modifier checks
    bool IsShift() const { return (key_modifier & SHIFT) != 0; }
    bool IsCtrl() const { return (key_modifier & CTRL) != 0; }
    bool IsAlt() const { return (key_modifier & ALT) != 0; }
    bool IsCmd() const { return (key_modifier & CMD) != 0; }
    bool IsMeta() const { return (key_modifier & META) != 0; }
    bool IsKey(int code) const { return event_type == KEY_PRESS && key_code == code; }
    
    bool IsLeftDown() const { return event_type == MOUSE_LEFT_DOWN; }
    bool IsLeftUp() const { return event_type == MOUSE_LEFT_UP; }
    bool IsLeftDouble() const { return event_type == MOUSE_LEFT_DOUBLE; }
    bool IsRightDown() const { return event_type == MOUSE_RIGHT_DOWN; }
    bool IsRightUp() const { return event_type == MOUSE_RIGHT_UP; }
    bool IsRightDouble() const { return event_type == MOUSE_RIGHT_DOUBLE; }
    bool IsMiddleDown() const { return event_type == MOUSE_MIDDLE_DOWN; }
    bool IsMiddleUp() const { return event_type == MOUSE_MIDDLE_UP; }
    bool IsMiddleDouble() const { return event_type == MOUSE_MIDDLE_DOUBLE; }
    bool IsMouseMove() const { return event_type == MOUSE_MOVE; }
    bool IsMouseWheel() const { return event_type == MOUSE_WHEEL; }
    bool IsPaint() const { return event_type == PAINT; }
    bool IsClose() const { return event_type == CLOSE; }
    bool IsSize() const { return event_type == SIZE; }

    // U++-style event handling
    bool Is() const { return event_type != UNKNOWN; }
    bool IsNull() const { return event_type == UNKNOWN; }
    
    void SetHandled(bool h = true) { handled = h; }
    bool IsHandled() const { return handled; }
    
    void Handled() { SetHandled(true); }  // Convenience method

    // U++-style event type identification
    bool IsMouse() const { 
        return event_type >= MOUSE_LEFT_DOWN && event_type <= MOUSE_WHEEL; 
    }
    
    bool IsKey() const { 
        return event_type >= KEY_PRESS && event_type <= KEY_CHAR; 
    }

    // U++-style methods for identifying event types
    const char* GetTypeName() const {
        switch (event_type) {
            case MOUSE_LEFT_DOWN: return "MOUSE_LEFT_DOWN";
            case MOUSE_LEFT_UP: return "MOUSE_LEFT_UP";
            case MOUSE_LEFT_DOUBLE: return "MOUSE_LEFT_DOUBLE";
            case MOUSE_RIGHT_DOWN: return "MOUSE_RIGHT_DOWN";
            case MOUSE_RIGHT_UP: return "MOUSE_RIGHT_UP";
            case MOUSE_RIGHT_DOUBLE: return "MOUSE_RIGHT_DOUBLE";
            case MOUSE_MIDDLE_DOWN: return "MOUSE_MIDDLE_DOWN";
            case MOUSE_MIDDLE_UP: return "MOUSE_MIDDLE_UP";
            case MOUSE_MIDDLE_DOUBLE: return "MOUSE_MIDDLE_DOUBLE";
            case MOUSE_WHEEL: return "MOUSE_WHEEL";
            case MOUSE_MOVE: return "MOUSE_MOVE";
            case KEY_PRESS: return "KEY_PRESS";
            case KEY_RELEASE: return "KEY_RELEASE";
            case KEY_CHAR: return "KEY_CHAR";
            case PAINT: return "PAINT";
            case ACTIVATE: return "ACTIVATE";
            case DEACTIVATE: return "DEACTIVATE";
            case GAIN_FOCUS: return "GAIN_FOCUS";
            case LOSE_FOCUS: return "LOSE_FOCUS";
            case SIZE: return "SIZE";
            case MOVE: return "MOVE";
            case CLOSE: return "CLOSE";
            case DESTROY: return "DESTROY";
            case TIMER: return "TIMER";
            case USER: return "USER";
            default: return "UNKNOWN";
        }
    }
};

// Common key codes (U++ style)
namespace K {
    const int Left = 37;
    const int Up = 38;
    const int Right = 39;
    const int Down = 40;
    const int Insert = 45;
    const int Delete = 46;
    const int Home = 36;
    const int End = 35;
    const int PgUp = 33;
    const int PgDn = 34;
    const int F1 = 112;
    const int F2 = 113;
    const int F3 = 114;
    const int F4 = 115;
    const int F5 = 116;
    const int F6 = 117;
    const int F7 = 118;
    const int F8 = 119;
    const int F9 = 120;
    const int F10 = 121;
    const int F11 = 122;
    const int F12 = 123;
    const int Tab = 9;
    const int Enter = 13;
    const int Escape = 27;
    const int Backspace = 8;
    const int Space = 32;
    const int CtrlA = 1;
    const int CtrlC = 3;
    const int CtrlV = 22;
    const int CtrlX = 24;
    const int CtrlZ = 26;
}