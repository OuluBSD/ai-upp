#pragma once
// U++-compatible Label wrapper for UI labels
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

#include <string>
#include "../Draw/Color.h"
#include "../Draw/Font.h"
#include "../Draw/DrawCore.h"
#include "../CtrlCore/Ctrl.h"

class Label : public Ctrl {
private:
    std::string text;
    Font font;
    Color text_color;
    int alignment;  // 0=left, 1=center, 2=right

public:
    // Constructors
    Label() : Ctrl(), text("Label"), font(Font::Arial(12)), 
              text_color(Color::Black()), alignment(0) {
        // Default size for a label
        SetSize(100, 20);
    }
    
    explicit Label(const std::string& label_text) : Ctrl(), text(label_text), 
              font(Font::Arial(12)), text_color(Color::Black()), alignment(0) {
        SetSize(100, 20);
    }

    // U++-style static constructors
    static Label* Create() { return new Label(); }
    static Label* Create(const std::string& label_text) { return new Label(label_text); }

    // U++-style label properties
    void SetLabelText(const std::string& lbl) { 
        text = lbl; 
    }
    
    void SetLabel(const std::string& lbl) { 
        text = lbl; 
    }

    // U++-style text accessors
    std::string GetLabel() const { return text; }
    std::string GetText() const { return text; }

    // U++-style font operations
    void SetFont(const Font& f) { 
        font = f; 
    }
    
    const Font& GetFont() const { return font; }
    
    void Font(const Font& f) { 
        font = f; 
    }

    // U++-style text color operations
    void SetTextColor(const Color& color) { 
        text_color = color; 
    }
    
    const Color& GetTextColor() const { return text_color; }
    
    void TextColor(const Color& color) { 
        text_color = color; 
    }

    // U++-style alignment operations
    void SetAlign(int align) { 
        alignment = align; 
    }
    
    void Left() { 
        alignment = 0; 
    }
    
    void Center() { 
        alignment = 1; 
    }
    
    void Right() { 
        alignment = 2; 
    }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        // Draw label background (usually transparent)
        if (GetBackgroundColor().GetA() > 0) {
            draw.DrawRect(GetRect(), GetBackgroundColor());
        }

        // Draw label text
        if (!text.empty()) {
            Rect r = GetRect();
            Size textSize = font.GetTextSize(text);
            
            int x = r.GetLeft();
            if (alignment == 1) {  // Center
                x = r.GetLeft() + (r.GetWidth() - textSize.cx) / 2;
            } else if (alignment == 2) {  // Right
                x = r.GetRight() - textSize.cx;
            }
            
            int y = r.GetTop() + (r.GetHeight() - textSize.cy) / 2;
            
            // In a real implementation, this would use proper text rendering
            // draw.DrawText(Point(x, y), text, font, text_color);
        }
    }

    // U++-style label operations
    void NoWantFocus() {
        // Labels don't typically receive focus
    }

    // U++-style label appearance
    void Inset(int left, int top, int right, int bottom) {
        // In a real implementation, this would set text inset values
    }
    
    void SetFrame(int left, int top, int right, int bottom) {
        // In a real implementation, this would set frame margins
    }

    // U++-style size adjustment
    void SizePos() {
        // In a real implementation, this would set the size and position based on content
        // For now, we'll set a reasonable default size based on text content
        if (!text.empty()) {
            Size textSize = font.GetTextSize(text);
            SetSize(textSize.cx + 10, textSize.cy + 4); // Add some padding
        }
    }
    
    // U++-style auto-size to fit content
    void AutoSize() {
        SizePos();  // For labels, SizePos() effectively autosizes
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "Label"; }
    
    // U++-style method to get alignment
    int GetAlign() const { return alignment; }
    
    // U++-style multi-line text support
    void NoEllipsis() {
        // In a real implementation, this would prevent text ellipsis
    }
    
    // U++-style text formatting
    void SetSingle() {
        // In a real implementation, this would set single line display
    }
    
    // U++-style method to set text with formatting
    void SetData(const std::string& data) {
        SetLabel(data);
    }
    
    std::string GetData() const {
        return GetLabel();
    }
};