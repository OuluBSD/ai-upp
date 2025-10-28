#pragma once
// U++-compatible Button wrapper for UI buttons
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

#include <string>
#include "../Draw/Color.h"
#include "../Draw/DrawCore.h"
#include "../CtrlCore/Ctrl.h"

class Button : public Ctrl {
private:
    std::string text;
    bool pressed;

public:
    // Constructors
    Button() : CtrlBase(), text("Button"), pressed(false) {
        // Default size for a button
        SetSize(75, 23);
    }
    
    explicit Button(const std::string& label) : CtrlBase(), text(label), pressed(false) {
        SetSize(75, 23);
    }

    // U++-style static constructors
    static Button* Create() { return new Button(); }
    static Button* Create(const std::string& label) { return new Button(label); }

    // U++-style button properties
    void Label(const std::string& lbl) { 
        text = lbl; 
    }
    
    void SetLabel(const std::string& lbl) { 
        text = lbl; 
    }

    // U++-style text accessors
    std::string GetLabel() const { return text; }
    std::string GetText() const { return text; }

    // U++-style button state
    void SetPressed(bool p = true) { 
        pressed = p; 
    }
    
    bool IsPressed() const { return pressed; }
    bool IsPush() const { return pressed; }  // Synonym for IsPressed

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        // Draw button background
        Color bgColor = IsEnabled() ? GetBackgroundColor() : Color(200, 200, 200);
        draw.DrawRect(GetRect(), bgColor);

        // Draw button border
        Rect borderRect = GetRect();
        draw.DrawRect(borderRect, Color::Black());

        // Draw button text centered
        if (!text.empty()) {
            // Calculate text position to center it
            Size textSize = Font().GetTextSize(text);
            Rect r = GetRect();
            Point textPos(
                r.GetLeft() + (r.GetWidth() - textSize.cx) / 2,
                r.GetTop() + (r.GetHeight() - textSize.cy) / 2
            );
            
            // In a real implementation, this would use proper text rendering
            // draw.DrawText(textPos, text, Font(), Color::Black());
        }
    }

    // U++-style button operations
    void Click() {
        if (IsEnabled() && click_handler) {
            click_handler();
        }
    }

    // U++-style button appearance
    void NoWantFocus() {
        // In a real implementation, this would prevent the button from receiving keyboard focus
    }
    
    void NoRigid() {
        // In a real implementation, this would control button sizing behavior
    }

    // U++-style button types
    void OK() {
        text = "OK";
    }
    
    void Cancel() {
        text = "Cancel";
    }
    
    void Yes() {
        text = "Yes";
    }
    
    void No() {
        text = "No";
    }

    // U++-style button size adjustment
    void SizePos() {
        // In a real implementation, this would set the size and position based on content
        // For now, we'll set a reasonable default size
        SetSize(75, 23);
    }
    
    void SetStd() {
        // Set to standard button appearance
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "Button"; }
    
    // U++-style method to determine if button is pressed state
    bool IsDown() const { return pressed; }
    
    // U++-style toggle button functionality
    void Toggle(bool state = true) {
        pressed = state;
    }
    
    bool IsToggle() const {
        // In a real implementation, this would indicate if it's a toggle button
        return false;
    }
    
    // U++-style visual feedback for press state
    void SetInk(const Color& color) {
        // In a real implementation, this would set the ink (text) color
    }
    
    void Inverted() {
        // In a real implementation, this would invert the button appearance
    }
};