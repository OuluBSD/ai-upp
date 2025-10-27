// U++-compatible Label wrapper for UI labels
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

class Label : public CtrlBase {
private:
    std::string text;
    Font font;
    Color text_color;
    int alignment;  // 0=left, 1=center, 2=right

public:
    // Constructors
    Label() : CtrlBase(), text("Label"), font(Font::Arial(12)), 
              text_color(Color::Black()), alignment(0) {
        // Default size for a label
        SetSize(100, 20);
    }
    
    explicit Label(const std::string& label_text) : CtrlBase(), text(label_text), 
              font(Font::Arial(12)), text_color(Color::Black()), alignment(0) {
        SetSize(100, 20);
    }

    // U++-style static constructors
    static Label* Create() { return new Label(); }
    static Label* Create(const std::string& label_text) { return new Label(label_text); }

    // U++-style label properties
    Label& Label(const std::string& lbl) { 
        text = lbl; 
        return *this; 
    }
    
    Label& SetLabel(const std::string& lbl) { 
        text = lbl; 
        return *this; 
    }

    // U++-style text accessors
    std::string GetLabel() const { return text; }
    std::string GetText() const { return text; }

    // U++-style font operations
    Label& SetFont(const Font& f) { 
        font = f; 
        return *this; 
    }
    
    const Font& GetFont() const { return font; }
    
    Label& Font(const Font& f) { 
        font = f; 
        return *this; 
    }

    // U++-style text color operations
    Label& SetTextColor(const Color& color) { 
        text_color = color; 
        return *this; 
    }
    
    const Color& GetTextColor() const { return text_color; }
    
    Label& TextColor(const Color& color) { 
        text_color = color; 
        return *this; 
    }

    // U++-style alignment operations
    Label& SetAlign(int align) { 
        alignment = align; 
        return *this; 
    }
    
    Label& Left() { 
        alignment = 0; 
        return *this; 
    }
    
    Label& Center() { 
        alignment = 1; 
        return *this; 
    }
    
    Label& Right() { 
        alignment = 2; 
        return *this; 
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
    Label& NoWantFocus() {
        // Labels don't typically receive focus
        return *this;
    }

    // U++-style label appearance
    Label& Inset(int left, int top, int right, int bottom) {
        // In a real implementation, this would set text inset values
        return *this;
    }
    
    Label& SetFrame(int left, int top, int right, int bottom) {
        // In a real implementation, this would set frame margins
        return *this;
    }

    // U++-style size adjustment
    Label& SizePos() {
        // In a real implementation, this would set the size and position based on content
        // For now, we'll set a reasonable default size based on text content
        if (!text.empty()) {
            Size textSize = font.GetTextSize(text);
            SetSize(textSize.cx + 10, textSize.cy + 4); // Add some padding
        }
        return *this;
    }
    
    // U++-style auto-size to fit content
    Label& AutoSize() {
        return SizePos();  // For labels, SizePos() effectively autosizes
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "Label"; }
    
    // U++-style method to get alignment
    int GetAlign() const { return alignment; }
    
    // U++-style multi-line text support
    Label& NoEllipsis() {
        // In a real implementation, this would prevent text ellipsis
        return *this;
    }
    
    // U++-style text formatting
    Label& SetSingle() {
        // In a real implementation, this would set single line display
        return *this;
    }
    
    // U++-style method to set text with formatting
    Label& SetData(const std::string& data) {
        SetLabel(data);
        return *this;
    }
    
    std::string GetData() const {
        return GetLabel();
    }
};