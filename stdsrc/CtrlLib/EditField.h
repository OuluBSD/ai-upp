// U++-compatible EditField wrapper for text input
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

class EditField : public CtrlBase {
private:
    std::string text;
    std::string prompt_text;  // Placeholder text
    Font font;
    Color text_color;
    Color prompt_color;
    bool password_mode;
    int cursor_pos;
    int selection_start;
    int selection_end;
    bool focused;

public:
    // Constructors
    EditField() : CtrlBase(), text(""), prompt_text(""), font(Font::Arial(12)), 
                  text_color(Color::Black()), prompt_color(Color(150, 150, 150)), 
                  password_mode(false), cursor_pos(0), selection_start(-1), 
                  selection_end(-1), focused(false) {
        // Default size for an edit field
        SetSize(150, 23);
    }
    
    explicit EditField(const std::string& initial_text) : CtrlBase(), text(initial_text), 
                  prompt_text(""), font(Font::Arial(12)), text_color(Color::Black()), 
                  prompt_color(Color(150, 150, 150)), password_mode(false), 
                  cursor_pos(0), selection_start(-1), selection_end(-1), focused(false) {
        SetSize(150, 23);
    }

    // U++-style static constructors
    static EditField* Create() { return new EditField(); }
    static EditField* Create(const std::string& initial_text) { return new EditField(initial_text); }

    // U++-style text operations
    EditField& Text(const std::string& txt) { 
        text = txt; 
        cursor_pos = static_cast<int>(text.length());  // Move cursor to end
        return *this; 
    }
    
    EditField& SetText(const std::string& txt) { 
        text = txt; 
        cursor_pos = static_cast<int>(text.length());
        return *this; 
    }
    
    std::string GetText() const { return text; }
    std::string GetData() const { return text; }

    // U++-style prompt/placeholder text
    EditField& Prompt(const std::string& prompt) { 
        prompt_text = prompt; 
        return *this; 
    }
    
    const std::string& GetPrompt() const { return prompt_text; }

    // U++-style font operations
    EditField& SetFont(const Font& f) { 
        font = f; 
        return *this; 
    }
    
    const Font& GetFont() const { return font; }
    
    EditField& Font(const Font& f) { 
        font = f; 
        return *this; 
    }

    // U++-style text color operations
    EditField& SetTextColor(const Color& color) { 
        text_color = color; 
        return *this; 
    }
    
    const Color& GetTextColor() const { return text_color; }
    
    EditField& TextColor(const Color& color) { 
        text_color = color; 
        return *this; 
    }

    // U++-style password mode
    EditField& Password() { 
        password_mode = true; 
        return *this; 
    }
    
    bool IsPassword() const { return password_mode; }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        Rect r = GetRect();
        
        // Draw edit field background
        Color bgColor = IsEnabled() ? GetBackgroundColor() : Color(240, 240, 240);
        draw.DrawRect(r, Color::Gray(), bgColor);

        // Draw edit field border
        draw.DrawRect(r, Color::Black());

        // Draw text or prompt
        std::string displayText = text;
        Color textColor = text_color;
        
        if (displayText.empty() && !prompt_text.empty() && !focused) {
            displayText = prompt_text;
            textColor = prompt_color;
        } else if (password_mode) {
            // In password mode, show asterisks
            displayText = std::string(text.length(), '*');
        }

        if (!displayText.empty()) {
            Size textSize = font.GetTextSize(displayText);
            int textX = r.GetLeft() + 2;  // Add some padding
            int textY = r.GetTop() + (r.GetHeight() - textSize.cy) / 2;
            
            // In a real implementation, this would use proper text rendering
            // draw.DrawText(Point(textX, textY), displayText, font, textColor);
        }

        // Draw cursor if focused (simplified representation)
        if (focused && !password_mode) {
            // Calculate cursor position based on text and font
            Size textBeforeCursorSize = font.GetTextSize(text.substr(0, cursor_pos));
            int cursorX = r.GetLeft() + 2 + textBeforeCursorSize.cx;
            draw.DrawLine(cursorX, r.GetTop() + 2, cursorX, r.GetBottom() - 2, Color::Black(), 1);
        }
    }

    // U++-style text manipulation
    EditField& Clear() { 
        text.clear(); 
        cursor_pos = 0;
        return *this; 
    }
    
    EditField& SelectAll() { 
        selection_start = 0;
        selection_end = static_cast<int>(text.length());
        return *this; 
    }
    
    EditField& NoWantFocus() {
        // In a real implementation, this would control focus behavior
        return *this;
    }
    
    EditField& SetFocus() {
        focused = true;
        // In a real implementation, this would set focus to the edit field
        return *this;
    }
    
    bool HasFocus() const { return focused; }

    // U++-style size adjustment
    EditField& SizePos() {
        // In a real implementation, this would size based on content
        return *this;
    }
    
    // U++-style text input handling (simplified)
    void InsertText(const std::string& new_text) {
        if (selection_start != -1 && selection_end != -1) {
            // If there's a selection, replace it
            int start = std::min(selection_start, selection_end);
            int end = std::max(selection_start, selection_end);
            text.erase(start, end - start);
            cursor_pos = start;
        }
        
        text.insert(cursor_pos, new_text);
        cursor_pos += static_cast<int>(new_text.length());
        
        // Clear selection after insertion
        selection_start = -1;
        selection_end = -1;
    }
    
    void DeleteSelection() {
        if (selection_start != -1 && selection_end != -1) {
            int start = std::min(selection_start, selection_end);
            int end = std::max(selection_start, selection_end);
            text.erase(start, end - start);
            cursor_pos = start;
            selection_start = -1;
            selection_end = -1;
        }
    }
    
    void SetCursor(int pos) {
        int len = static_cast<int>(text.length());
        cursor_pos = std::max(0, std::min(pos, len));
    }
    
    int GetCursor() const { return cursor_pos; }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "EditField"; }
    
    // U++-style data operations
    EditField& SetData(const std::string& data) {
        SetText(data);
        return *this;
    }
    
    // U++-style event handling
    virtual void OnFocus() { 
        focused = true; 
    }
    
    virtual void OnBlur() { 
        focused = false; 
    }
    
    // U++-style text operations
    int GetLength() const { return static_cast<int>(text.length()); }
    bool IsEmpty() const { return text.empty(); }
};