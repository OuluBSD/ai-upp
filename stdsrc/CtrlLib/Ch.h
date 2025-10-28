#pragma once
#ifndef _CtrlLib_Ch_h_
#define _CtrlLib_Ch_h_

#include "CtrlLib.h"
#include "Ctrl.h"
#include "Label.h"
#include <functional>
#include <vector>
#include <memory>

// Character input control (ChCtrl) and related functionality
class ChCtrl : public Ctrl {
private:
    bool is_password;
    String text;
    int cursor_pos;
    int selection_start;
    int selection_end;
    bool focused;
    std::function<void()> when_change;
    std::function<void()> when_action;
    bool multi_line;
    int max_chars;
    String allowed_chars;
    
public:
    ChCtrl() : is_password(false), cursor_pos(0), selection_start(-1), selection_end(-1), 
               focused(false), multi_line(false), max_chars(0) {}
    
    virtual void Paint(Draw& draw) const override {
        Rect r = GetRect();
        
        // Draw background
        draw.DrawRect(r, GetBackgroundColor());
        
        // Determine text to display
        String display_text = is_password ? String('*', text.GetLength()) : text;
        
        // Draw text
        if (!display_text.IsEmpty()) {
            // In a real implementation, this would draw the text with proper font and positioning
            draw.DrawText(r.left + 2, r.top + 2, display_text, StdFont(), SColorText);
        }
        
        // Draw cursor if focused
        if (focused) {
            // Calculate cursor position
            // In a real implementation, this would calculate based on font metrics
            int cursor_x = r.left + 2 + cursor_pos;  // Simplified position
            
            // Draw cursor
            draw.DrawLine(cursor_x, r.top + 2, cursor_x, r.bottom - 2, SColorText);
        }
        
        // Draw border
        draw.DrawRect(r.left, r.top, r.GetWidth(), r.GetHeight(), 1, SColorText);
    }
    
    // Text operations
    ChCtrl& SetText(const String& s) {
        String new_text = s;
        if (max_chars > 0 && new_text.GetLength() > max_chars) {
            new_text = new_text.Mid(0, max_chars);
        }
        
        if (allowed_chars.GetCount() > 0) {
            // Filter characters based on allowed_chars
            String filtered;
            for (int i = 0; i < new_text.GetLength(); i++) {
                if (allowed_chars.Find(new_text[i]) >= 0) {
                    filtered.Cat(new_text[i]);
                }
            }
            new_text = filtered;
        }
        
        if (text != new_text) {
            text = new_text;
            if (when_change) when_change();
            Refresh();
        }
        return *this;
    }
    
    const String& GetText() const { return text; }
    
    ChCtrl& SetPassword(bool b = true) { 
        is_password = b; 
        Refresh(); 
        return *this; 
    }
    
    bool IsPassword() const { return is_password; }
    
    // Multi-line support
    ChCtrl& MultiLine(bool b = true) { 
        multi_line = b; 
        return *this; 
    }
    
    bool IsMultiLine() const { return multi_line; }
    
    // Character limit
    ChCtrl& SetMaxChars(int n) { 
        max_chars = n; 
        return *this; 
    }
    
    int GetMaxChars() const { return max_chars; }
    
    // Allowed characters
    ChCtrl& SetAllowed(const String& chars) { 
        allowed_chars = chars; 
        return *this; 
    }
    
    const String& GetAllowed() const { return allowed_chars; }
    
    // Event handlers
    ChCtrl& WhenChange(std::function<void()> h) { 
        when_change = h; 
        return *this; 
    }
    
    ChCtrl& WhenAction(std::function<void()> h) { 
        when_action = h; 
        return *this; 
    }
    
    // Keyboard handling
    bool ProcessKey(dword key, int count = 1) {
        switch(key) {
            case K_LEFT:
                if (cursor_pos > 0) cursor_pos--;
                Refresh();
                return true;
            case K_RIGHT:
                if (cursor_pos < text.GetLength()) cursor_pos++;
                Refresh();
                return true;
            case K_HOME:
                cursor_pos = 0;
                Refresh();
                return true;
            case K_END:
                cursor_pos = text.GetLength();
                Refresh();
                return true;
            case K_BACKSPACE:
                if (cursor_pos > 0) {
                    text.Remove(cursor_pos - 1, 1);
                    cursor_pos--;
                    if (when_change) when_change();
                    Refresh();
                }
                return true;
            case K_DELETE:
                if (cursor_pos < text.GetLength()) {
                    text.Remove(cursor_pos, 1);
                    if (when_change) when_change();
                    Refresh();
                }
                return true;
            case K_ENTER:
                if (when_action) when_action();
                return true;
            default:
                if (key >= 32 && key <= 255) { // Printable characters
                    char c = (char)key;
                    if (max_chars <= 0 || text.GetLength() < max_chars) {
                        if (allowed_chars.IsEmpty() || allowed_chars.Find(c) >= 0) {
                            text.Insert(cursor_pos, String(c));
                            cursor_pos++;
                            if (when_change) when_change();
                            Refresh();
                            return true;
                        }
                    }
                }
        }
        return false;
    }
    
    // Mouse handling
    bool ProcessLeftDown(Point pos, dword flags) {
        SetFocus();
        // Calculate cursor position based on click position
        // Simplified implementation
        cursor_pos = Min(pos.x, text.GetLength());
        Refresh();
        return true;
    }
    
    // Focus handling
    void SetFocus() {
        focused = true;
        Ctrl::SetFocus();
        Refresh();
    }
    
    bool HasFocus() const {
        return focused;
    }
    
    // Get character at cursor position
    char GetCharAtCursor() const {
        if (cursor_pos >= 0 && cursor_pos < text.GetLength()) {
            return text[cursor_pos];
        }
        return 0; // Null character if out of bounds
    }
    
    // Insert text at cursor position
    ChCtrl& InsertText(const String& s) {
        text.Insert(cursor_pos, s);
        cursor_pos += s.GetLength();
        if (when_change) when_change();
        Refresh();
        return *this;
    }
    
    // Replace selected text
    ChCtrl& ReplaceSelection(const String& s) {
        if (selection_start >= 0 && selection_end > selection_start) {
            text.Remove(selection_start, selection_end - selection_start);
            text.Insert(selection_start, s);
            cursor_pos = selection_start + s.GetLength();
            selection_start = selection_end = -1;
            if (when_change) when_change();
            Refresh();
        }
        return *this;
    }
    
    // Clear the text
    ChCtrl& ClearText() {
        text.Clear();
        cursor_pos = 0;
        selection_start = selection_end = -1;
        if (when_change) when_change();
        Refresh();
        return *this;
    }
    
    // Get text length
    int GetTextLength() const { return text.GetLength(); }
    
    // Select all text
    ChCtrl& SelectAll() {
        selection_start = 0;
        selection_end = text.GetLength();
        cursor_pos = selection_end;
        Refresh();
        return *this;
    }
    
    // Check if text is selected
    bool IsTextSelected() const { 
        return selection_start >= 0 && selection_end > selection_start; 
    }
    
    // Get selected text
    String GetSelectedText() const {
        if (IsTextSelected()) {
            return text.Mid(selection_start, selection_end - selection_start);
        }
        return String();
    }
};

// Specialized character controls
class NumericChCtrl : public ChCtrl {
private:
    double value;
    double min_value;
    double max_value;
    int decimal_places;
    
public:
    NumericChCtrl() : value(0), min_value(-DBL_MAX), max_value(DBL_MAX), decimal_places(2) {
        SetAllowed("0123456789.-+");
    }
    
    NumericChCtrl& SetRange(double min, double max) {
        min_value = min;
        max_value = max;
        return *this;
    }
    
    NumericChCtrl& SetValue(double val) {
        val = max(min_value, min(max_value, val));
        value = val;
        SetText(Format(val, decimal_places));
        return *this;
    }
    
    double GetValue() const { return value; }
    
    NumericChCtrl& SetDecimalPlaces(int places) {
        decimal_places = places;
        return *this;
    }
    
    int GetDecimalPlaces() const { return decimal_places; }
    
    // Override text setting to maintain value synchronization
    NumericChCtrl& SetText(const String& s) {
        double val = ScanDouble(s);
        if (val >= min_value && val <= max_value) {
            value = val;
        }
        ChCtrl::SetText(s);
        return *this;
    }
};

class IntegerChCtrl : public NumericChCtrl {
public:
    IntegerChCtrl() {
        SetAllowed("0123456789-+");
    }
    
    IntegerChCtrl& SetValue(int val) {
        NumericChCtrl::SetValue(val);
        return *this;
    }
    
    int GetIntValue() const {
        return (int)GetValue();
    }
    
    // Override text setting to maintain integer value
    IntegerChCtrl& SetText(const String& s) {
        int val = ScanInt(s);
        NumericChCtrl::SetValue(val);
        return *this;
    }
};

class HexChCtrl : public ChCtrl {
public:
    HexChCtrl() {
        SetAllowed("0123456789ABCDEFabcdef");
    }
    
    HexChCtrl& SetValue(int val) {
        SetText(Format("%X", val));
        return *this;
    }
    
    int GetIntValue() const {
        return StrHexToUInt64(GetText());
    }
    
    // Override text setting to maintain hex format
    HexChCtrl& SetText(const String& s) {
        String hex = s;
        // Ensure it's valid hex
        for(int i = 0; i < hex.GetLength(); i++) {
            char c = ToUpper(hex[i]);
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                hex.Remove(i--, 1);
            }
        }
        ChCtrl::SetText(ToUpper(hex));
        return *this;
    }
};

// Character control with validation
class ValidatedChCtrl : public ChCtrl {
private:
    std::function<bool(const String&)> validator;
    String error_tooltip;
    bool validation_ok;
    
public:
    ValidatedChCtrl() : validation_ok(true) {}
    
    ValidatedChCtrl& SetValidator(std::function<bool(const String&)> v, const String& error_msg = "Invalid input") {
        validator = v;
        error_tooltip = error_msg;
        return *this;
    }
    
    ValidatedChCtrl& SetText(const String& s) {
        ChCtrl::SetText(s);
        Validate();
        return *this;
    }
    
    bool Validate() {
        if (validator) {
            validation_ok = validator(GetText());
            if (!validation_ok) {
                SetToolTip(error_tooltip);
            } else {
                SetToolTip(String()); // Clear tooltip
            }
            Refresh();
        }
        return validation_ok;
    }
    
    bool IsValid() const { return validation_ok; }
    
    // Override WhenChange to perform validation
    ValidatedChCtrl& WhenChange(std::function<void()> h) {
        return ChCtrl::WhenChange([this, h]() {
            Validate();
            if (h) h();
        });
    }
};

// Helper function to create a ChCtrl with common configurations
inline std::shared_ptr<ChCtrl> CreateChCtrl() {
    return std::make_shared<ChCtrl>();
}

inline std::shared_ptr<NumericChCtrl> CreateNumericChCtrl() {
    return std::make_shared<NumericChCtrl>();
}

inline std::shared_ptr<IntegerChCtrl> CreateIntegerChCtrl() {
    return std::make_shared<IntegerChCtrl>();
}

inline std::shared_ptr<HexChCtrl> CreateHexChCtrl() {
    return std::make_shared<HexChCtrl>();
}

inline std::shared_ptr<ValidatedChCtrl> CreateValidatedChCtrl() {
    return std::make_shared<ValidatedChCtrl>();
}

#endif