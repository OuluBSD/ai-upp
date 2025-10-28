#ifndef _CtrlLib_ColorPopup_h_
#define _CtrlLib_ColorPopup_h_

#include "CtrlLib.h"
#include "Ctrl.h"
#include "Button.h"
#include <functional>
#include <vector>

// Color palette and color selection components
class ColorPopup : public Ctrl {
private:
    Color selected_color;
    std::function<void(Color)> when_change;
    std::function<void(Color)> when_select;
    bool show_custom_color;
    Vector<Color> preset_colors;
    int selected_index;
    Size cell_size;
    
public:
    ColorPopup() : selected_color(Color::Black()), show_custom_color(true), 
                   selected_index(-1), cell_size(16, 16) {
        // Initialize with some preset colors
        AddPresetColor(Color::Black());
        AddPresetColor(Color::White());
        AddPresetColor(Color::Red());
        AddPresetColor(Color::Green());
        AddPresetColor(Color::Blue());
        AddPresetColor(Color::Yellow());
        AddPresetColor(Color::Magenta());
        AddPresetColor(Color::Cyan());
        AddPresetColor(Color::Gray());
        AddPresetColor(Color::LtGray());
        AddPresetColor(Color::DkGray());
    }
    
    virtual void Paint(Draw& draw) const override {
        Rect r = GetRect();
        
        // Draw background
        draw.DrawRect(r, SColorFace);
        
        // Draw color grid
        DrawColorGrid(draw, r);
        
        // Draw selection border if applicable
        if (selected_index >= 0 && selected_index < preset_colors.GetCount()) {
            DrawSelection(draw, r);
        }
    }
    
    // Add a preset color
    ColorPopup& AddPresetColor(Color c) {
        preset_colors.Add(c);
        Refresh();
        return *this;
    }
    
    // Add multiple preset colors
    ColorPopup& AddPresetColors(const Vector<Color>& colors) {
        for (const auto& color : colors) {
            preset_colors.Add(color);
        }
        Refresh();
        return *this;
    }
    
    // Clear all preset colors
    ColorPopup& ClearPresetColors() {
        preset_colors.Clear();
        selected_index = -1;
        Refresh();
        return *this;
    }
    
    // Set selected color
    ColorPopup& SetColor(Color c) {
        selected_color = c;
        
        // Find if this color is in our preset list
        selected_index = -1;
        for (int i = 0; i < preset_colors.GetCount(); i++) {
            if (preset_colors[i] == c) {
                selected_index = i;
                break;
            }
        }
        
        if (when_change) when_change(c);
        Refresh();
        return *this;
    }
    
    // Get selected color
    Color GetColor() const { return selected_color; }
    
    // Event handlers
    ColorPopup& WhenChange(std::function<void(Color)> h) { 
        when_change = h; 
        return *this; 
    }
    
    ColorPopup& WhenSelect(std::function<void(Color)> h) { 
        when_select = h; 
        return *this; 
    }
    
    // Process mouse click
    bool ProcessLeftDown(Point pos, dword flags) override {
        int cell_x = (pos.x - 2) / (cell_size.cx + 2);
        int cell_y = (pos.y - 2) / (cell_size.cy + 2);
        int index = cell_y * GetColumns() + cell_x;
        
        if (index >= 0 && index < preset_colors.GetCount()) {
            selected_color = preset_colors[index];
            selected_index = index;
            if (when_change) when_change(selected_color);
            if (when_select) when_select(selected_color);
            Refresh();
        }
        
        return true;
    }
    
    // Show custom color dialog
    ColorPopup& ShowCustom(bool b = true) { 
        show_custom_color = b; 
        return *this; 
    }
    
    bool IsShowCustom() const { return show_custom_color; }
    
    // Get/Set cell size for color display
    ColorPopup& SetCellSize(int width, int height) {
        cell_size = Size(width, height);
        return *this;
    }
    
    Size GetCellSize() const { return cell_size; }
    
    // Get number of columns that will be displayed
    int GetColumns() const {
        if (cell_size.cx <= 0) return 0;
        return Max(1, (GetRect().GetWidth() - 4) / (cell_size.cx + 2));
    }
    
    // Get number of rows that will be displayed
    int GetRows() const {
        int cols = GetColumns();
        if (cols <= 0) return 0;
        return (preset_colors.GetCount() + cols - 1) / cols;
    }
    
    // Get total number of preset colors
    int GetPresetCount() const { return preset_colors.GetCount(); }
    
    // Get preset color at index
    Color GetPresetColor(int index) const {
        return index >= 0 && index < preset_colors.GetCount() ? preset_colors[index] : Color(0, 0, 0);
    }
    
private:
    void DrawColorGrid(Draw& draw, const Rect& bounds) const {
        int cols = GetColumns();
        if (cols <= 0) return;
        
        int x = bounds.left + 2;
        int y = bounds.top + 2;
        int current_col = 0;
        
        for (int i = 0; i < preset_colors.GetCount(); i++) {
            // Draw color cell
            Rect cell_rect(x, y, x + cell_size.cx, y + cell_size.cy);
            draw.DrawRect(cell_rect, preset_colors[i]);
            draw.DrawRect(cell_rect, 1, SColorText);
            
            // Move to next position
            current_col++;
            if (current_col >= cols) {
                current_col = 0;
                x = bounds.left + 2;
                y += cell_size.cy + 2;
            } else {
                x += cell_size.cx + 2;
            }
        }
    }
    
    void DrawSelection(Draw& draw, const Rect& bounds) const {
        if (selected_index < 0 || selected_index >= preset_colors.GetCount()) return;
        
        int cols = GetColumns();
        if (cols <= 0) return;
        
        int row = selected_index / cols;
        int col = selected_index % cols;
        
        int x = bounds.left + 2 + col * (cell_size.cx + 2);
        int y = bounds.top + 2 + row * (cell_size.cy + 2);
        
        Rect cell_rect(x, y, x + cell_size.cx, y + cell_size.cy);
        
        // Draw selection indicator (inverted color or border)
        draw.DrawRect(cell_rect.left, cell_rect.top, cell_rect.GetWidth(), 2, Color::White());
        draw.DrawRect(cell_rect.left, cell_rect.top, 2, cell_rect.GetHeight(), Color::White());
        draw.DrawRect(cell_rect.right - 2, cell_rect.top, 2, cell_rect.GetHeight(), Color::Black());
        draw.DrawRect(cell_rect.left, cell_rect.bottom - 2, cell_rect.GetWidth(), 2, Color::Black());
    }
};

// Enhanced color popup with more features
class ColorPopupEx : public ColorPopup {
private:
    Vector<Color> recent_colors;
    int max_recent;
    bool show_recent;
    
public:
    ColorPopupEx() : max_recent(8), show_recent(true) {}
    
    // Add to recent colors
    ColorPopupEx& AddRecentColor(Color c) {
        if (recent_colors.GetCount() == 0 || recent_colors.Top() != c) {
            // Remove color if it already exists
            for (int i = 0; i < recent_colors.GetCount(); i++) {
                if (recent_colors[i] == c) {
                    recent_colors.Remove(i);
                    break;
                }
            }
            
            // Add to top of recent list
            recent_colors.Insert(0, c);
            
            // Maintain max recent colors
            if (recent_colors.GetCount() > max_recent) {
                recent_colors.RemoveLast();
            }
        }
        return *this;
    }
    
    // Set max recent colors
    ColorPopupEx& MaxRecent(int n) { 
        max_recent = n; 
        return *this; 
    }
    
    // Show/hide recent colors
    ColorPopupEx& ShowRecent(bool b = true) { 
        show_recent = b; 
        return *this; 
    }
    
    // Get recent colors
    const Vector<Color>& GetRecentColors() const { return recent_colors; }
    
    // Clear recent colors
    ColorPopupEx& ClearRecent() { 
        recent_colors.Clear(); 
        return *this; 
    }
};

// Standard color palette with common colors
class StdColorPalette : public ColorPopup {
public:
    StdColorPalette() {
        ClearPresetColors();
        
        // Add standard colors
        AddPresetColor(Color(0, 0, 0));       // Black
        AddPresetColor(Color(255, 255, 255)); // White
        AddPresetColor(Color(255, 0, 0));     // Red
        AddPresetColor(Color(0, 255, 0));     // Green
        AddPresetColor(Color(0, 0, 255));     // Blue
        AddPresetColor(Color(255, 255, 0));   // Yellow
        AddPresetColor(Color(255, 0, 255));   // Magenta
        AddPresetColor(Color(0, 255, 255));   // Cyan
        
        // Add grayscale
        AddPresetColor(Color(192, 192, 192)); // Light gray
        AddPresetColor(Color(128, 128, 128)); // Gray
        AddPresetColor(Color(64, 64, 64));    // Dark gray
        AddPresetColor(Color(255, 128, 128)); // Light red
        AddPresetColor(Color(128, 255, 128)); // Light green
        AddPresetColor(Color(128, 128, 255)); // Light blue
        AddPresetColor(Color(255, 255, 128)); // Light yellow
        AddPresetColor(Color(255, 128, 255)); // Light magenta
        AddPresetColor(Color(128, 255, 255)); // Light cyan
    }
};

// Web-safe color palette
class WebSafePalette : public ColorPopup {
public:
    WebSafePalette() {
        ClearPresetColors();
        
        // Web-safe colors (multiples of 51 for each RGB component)
        for (int r = 0; r <= 255; r += 51) {
            for (int g = 0; g <= 255; g += 51) {
                for (int b = 0; b <= 255; b += 51) {
                    if (r != g || g != b) { // Exclude grays to save space in example
                        AddPresetColor(Color(r, g, b));
                    }
                }
            }
        }
    }
};

// Hue-based color palette
class HuePalette : public ColorPopup {
public:
    HuePalette() {
        ClearPresetColors();
        
        // Add colors with different hues
        for (int hue = 0; hue < 360; hue += 30) {
            AddPresetColor(HSVtoRGB(hue, 1.0, 1.0));
        }
    }
    
private:
    Color HSVtoRGB(double h, double s, double v) {
        // Convert HSV to RGB
        double c = v * s;
        double x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
        double m = v - c;
        
        double r, g, b;
        if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
        else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
        else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
        else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
        else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }
        
        return Color(
            (int)((r + m) * 255),
            (int)((g + m) * 255),
            (int)((b + m) * 255)
        );
    }
};

// Color picker button that shows a color popup
class ColorButton : public Button {
private:
    Color selected_color;
    std::function<void(Color)> when_color;
    std::unique_ptr<ColorPopup> color_popup;
    bool popup_visible;
    
public:
    ColorButton() : selected_color(Color::White()), popup_visible(false) {
        SetLabel("  ");
        SetStyle(Button::PUSH);
    }
    
    virtual void Paint(Draw& draw) override {
        Button::Paint(draw); // Draw button background
        
        // Draw color indicator
        Rect r = GetRect();
        Rect color_rect(r.right - 20, r.top + 2, r.right - 2, r.bottom - 2);
        draw.DrawRect(color_rect, selected_color);
        draw.DrawRect(color_rect, 1, SColorText);
    }
    
    ColorButton& SetColor(Color c) {
        selected_color = c;
        Refresh();
        return *this;
    }
    
    Color GetColor() const { return selected_color; }
    
    ColorButton& WhenColor(std::function<void(Color)> h) {
        when_color = h;
        return *this;
    }
    
    // Show color popup
    ColorButton& ShowColorPopup() {
        // In a real implementation, this would create and show a color popup
        // at the appropriate position
        if (!color_popup) {
            color_popup = std::make_unique<ColorPopup>();
            color_popup->SetColor(selected_color);
            color_popup->WhenSelect([this](Color c) {
                SetColor(c);
                if (when_color) when_color(c);
                // Hide popup after selection
            });
        }
        
        // Position the popup relative to this button
        Rect btn_rect = GetScreenRect();
        color_popup->SetRect(btn_rect.right, btn_rect.top, 140, 140);
        // In a real implementation, this would actually show the popup
        
        popup_visible = true;
        return *this;
    }
    
    // Process mouse click
    bool ProcessLeftDown(Point pos, dword flags) override {
        ShowColorPopup();
        return Button::ProcessLeftDown(pos, flags);
    }
};

// Helper functions
inline std::shared_ptr<ColorPopup> CreateColorPopup() {
    return std::make_shared<ColorPopup>();
}

inline std::shared_ptr<StdColorPalette> CreateStdColorPalette() {
    return std::make_shared<StdColorPalette>();
}

inline std::shared_ptr<WebSafePalette> CreateWebSafePalette() {
    return std::make_shared<WebSafePalette>();
}

inline std::shared_ptr<HuePalette> CreateHuePalette() {
    return std::make_shared<HuePalette>();
}

inline std::shared_ptr<ColorButton> CreateColorButton() {
    return std::make_shared<ColorButton>();
}

#endif