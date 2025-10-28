#ifndef _CtrlLib_ColorPusher_h_
#define _CtrlLib_ColorPusher_h_

#include "CtrlLib.h"
#include "Ctrl.h"
#include "Button.h"
#include <functional>
#include <vector>

// ColorPusher - A control that allows selecting from a grid of colors
class ColorPusher : public Ctrl {
private:
    Vector<Color> colors;
    std::vector<bool> enabled;  // Enable/disable status for each color
    int selected_index;
    int rows, cols;
    std::function<void(int, Color)> when_select;
    std::function<void(int, Color)> when_change;
    Size cell_size;
    bool allow_none;
    int none_index;
    String none_label;
    
public:
    ColorPusher() : selected_index(-1), rows(0), cols(0), 
                   cell_size(20, 20), allow_none(false), none_index(-1) {}
    
    virtual void Paint(Draw& draw) const override {
        Rect r = GetRect();
        
        // Draw background
        draw.DrawRect(r, SColorFace);
        
        // Draw color grid
        DrawColorGrid(draw, r);
    }
    
    // Add a color to the pusher
    ColorPusher& Add(Color c) {
        colors.Add(c);
        enabled.push_back(true);
        UpdateGridSize();
        Refresh();
        return *this;
    }
    
    // Add multiple colors
    ColorPusher& Add(const Vector<Color>& color_list) {
        for (const auto& color : color_list) {
            colors.Add(color);
            enabled.push_back(true);
        }
        UpdateGridSize();
        Refresh();
        return *this;
    }
    
    // Set colors from vector
    ColorPusher& Set(const Vector<Color>& color_list) {
        colors = color_list;
        enabled.clear();
        enabled.resize(colors.GetCount(), true);
        UpdateGridSize();
        selected_index = -1; // Reset selection
        Refresh();
        return *this;
    }
    
    // Set color at specific index
    ColorPusher& Set(int index, Color c) {
        if (index >= 0 && index < colors.GetCount()) {
            colors[index] = c;
            Refresh();
        }
        return *this;
    }
    
    // Remove color at index
    ColorPusher& Remove(int index) {
        if (index >= 0 && index < colors.GetCount()) {
            colors.Remove(index);
            enabled.erase(enabled.begin() + index);
            if (selected_index == index) {
                selected_index = -1;
            } else if (selected_index > index) {
                selected_index--;
            }
            UpdateGridSize();
            Refresh();
        }
        return *this;
    }
    
    // Clear all colors
    ColorPusher& Clear() {
        colors.Clear();
        enabled.clear();
        selected_index = -1;
        UpdateGridSize();
        Refresh();
        return *this;
    }
    
    // Get number of colors
    int GetCount() const { return colors.GetCount(); }
    
    // Get color at index
    Color Get(int index) const {
        return index >= 0 && index < colors.GetCount() ? colors[index] : Color(0, 0, 0);
    }
    
    // Select color by index
    ColorPusher& Select(int index) {
        if (index >= -1 && index < colors.GetCount() && 
            (index == -1 || enabled[index])) {
            int old_index = selected_index;
            selected_index = index;
            
            if (old_index != selected_index && when_change) {
                Color old_color = old_index >= 0 ? colors[old_index] : Color(0, 0, 0);
                Color new_color = selected_index >= 0 ? colors[selected_index] : Color(0, 0, 0);
                when_change(selected_index, new_color);
            }
            
            Refresh();
        }
        return *this;
    }
    
    // Get selected index
    int GetIndex() const { return selected_index; }
    
    // Get selected color
    Color Get() const {
        return selected_index >= 0 && selected_index < colors.GetCount() 
               ? colors[selected_index] : Color(0, 0, 0);
    }
    
    // Event handlers
    ColorPusher& WhenSelect(std::function<void(int, Color)> h) { 
        when_select = h; 
        return *this; 
    }
    
    ColorPusher& WhenChange(std::function<void(int, Color)> h) { 
        when_change = h; 
        return *this; 
    }
    
    // Process mouse click
    bool ProcessLeftDown(Point pos, dword flags) override {
        int index = GetIndexFromPoint(pos);
        if (index != -2) { // -2 means outside any cell
            if (index >= 0 && index < static_cast<int>(enabled.size()) && enabled[index]) {
                Select(index);
                if (when_select) {
                    when_select(selected_index, Get());
                }
            }
        }
        return true;
    }
    
    // Set cell size
    ColorPusher& SetCellSize(Size sz) { 
        cell_size = sz; 
        return *this; 
    }
    
    ColorPusher& SetCellSize(int cx, int cy) { 
        cell_size = Size(cx, cy); 
        return *this; 
    }
    
    Size GetCellSize() const { return cell_size; }
    
    // Enable/disable a specific color
    ColorPusher& Enable(int index, bool enable = true) {
        if (index >= 0 && index < static_cast<int>(enabled.size())) {
            enabled[index] = enable;
            Refresh();
        }
        return *this;
    }
    
    ColorPusher& Disable(int index) { return Enable(index, false); }
    
    bool IsEnabled(int index) const {
        return index >= 0 && index < static_cast<int>(enabled.size()) && enabled[index];
    }
    
    // Set number of rows and columns
    ColorPusher& SetGrid(int r, int c) { 
        rows = r; 
        cols = c; 
        return *this; 
    }
    
    // Allow selection of "none" option
    ColorPusher& NoColor(String label = "None") { 
        allow_none = true; 
        none_label = label;
        return *this; 
    }
    
    bool HasNoColor() const { return allow_none; }
    
    // Set the "no color" index
    ColorPusher& SetNoColorIndex(int index) { 
        none_index = index; 
        return *this; 
    }
    
    // Get the current grid dimensions
    int GetRows() const { return rows; }
    int GetCols() const { return cols; }
    
private:
    void UpdateGridSize() {
        if (rows == 0 && cols == 0) {
            // Auto-calculate dimensions
            int count = colors.GetCount();
            if (count == 0) {
                rows = cols = 0;
            } else {
                cols = std::max(1, (int)std::sqrt((double)count));
                rows = (count + cols - 1) / cols;
            }
        }
    }
    
    int GetIndexFromPoint(Point pt) const {
        Rect r = GetRect();
        
        // Calculate which cell was clicked
        int col_width = (r.Width() - 4) / cols;
        int row_height = (r.Height() - 4) / rows;
        
        if (col_width <= 0 || row_height <= 0) return -2;
        
        int col = (pt.x - r.left - 2) / col_width;
        int row = (pt.y - r.top - 2) / row_height;
        
        int index = row * cols + col;
        
        if (index < 0 || index >= colors.GetCount()) {
            return -2; // Outside any cell
        }
        
        return index;
    }
    
    void DrawColorGrid(Draw& draw, const Rect& bounds) const {
        if (colors.IsEmpty()) return;
        
        int actual_cols = cols > 0 ? cols : std::max(1, (int)std::sqrt((double)colors.GetCount()));
        int actual_rows = rows > 0 ? rows : (colors.GetCount() + actual_cols - 1) / actual_cols;
        
        int cell_width = (bounds.Width() - 4) / actual_cols;
        int cell_height = (bounds.Height() - 4) / actual_rows;
        
        if (cell_width <= 0 || cell_height <= 0) return;
        
        for (int i = 0; i < colors.GetCount(); i++) {
            int row = i / actual_cols;
            int col = i % actual_cols;
            
            if (row >= actual_rows) break;
            
            int x = bounds.left + 2 + col * cell_width;
            int y = bounds.top + 2 + row * cell_height;
            
            Rect cell_rect(x, y, x + cell_width - 1, y + cell_height - 1);
            
            // Draw color
            draw.DrawRect(cell_rect, colors[i]);
            
            // Draw cell border
            Color border_color = (i == selected_index) ? Color::Black() : Color::Gray();
            draw.DrawRect(cell_rect, 1, border_color);
            
            // Draw disabled indicator if needed
            if (!enabled[i]) {
                // Draw diagonal cross
                draw.DrawLine(cell_rect.left, cell_rect.top, cell_rect.right, cell_rect.bottom, Color::Red());
                draw.DrawLine(cell_rect.right, cell_rect.top, cell_rect.left, cell_rect.bottom, Color::Red());
            }
        }
    }
};

// Preset color pushers
class StandardColorPusher : public ColorPusher {
public:
    StandardColorPusher() {
        Vector<Color> standard_colors;
        standard_colors
            << Color(0, 0, 0) << Color(128, 128, 128) << Color(192, 192, 192) << Color(255, 255, 255)
            << Color(255, 0, 0) << Color(128, 0, 0) << Color(255, 128, 128) << Color(255, 192, 192)
            << Color(0, 255, 0) << Color(0, 128, 0) << Color(128, 255, 128) << Color(192, 255, 192)
            << Color(0, 0, 255) << Color(0, 0, 128) << Color(128, 128, 255) << Color(192, 192, 255)
            << Color(255, 255, 0) << Color(128, 128, 0) << Color(255, 255, 128) << Color(255, 255, 192)
            << Color(255, 0, 255) << Color(128, 0, 128) << Color(255, 128, 255) << Color(255, 192, 255)
            << Color(0, 255, 255) << Color(0, 128, 128) << Color(128, 255, 255) << Color(192, 255, 255);
        
        Set(standard_colors);
        SetGrid(7, 4);
    }
};

class WebSafeColorPusher : public ColorPusher {
public:
    WebSafeColorPusher() {
        Vector<Color> web_safe_colors;
        
        // Web safe colors (multiples of 51)
        byte levels[] = {0, 51, 102, 153, 204, 255};
        for (int r = 0; r < 6; r++) {
            for (int g = 0; g < 6; g++) {
                for (int b = 0; b < 6; b++) {
                    web_safe_colors.Add(Color(levels[r], levels[g], levels[b]));
                }
            }
        }
        
        Set(web_safe_colors);
        SetGrid(18, 12); // 36 colors per row, 12 rows
    }
};

// Gradient color pusher
class GradientColorPusher : public ColorPusher {
public:
    GradientColorPusher(Color start, Color end, int steps = 10) {
        Vector<Color> gradient_colors;
        
        for (int i = 0; i < steps; i++) {
            double factor = (double)i / (steps - 1);
            Color c(
                (byte)(start.r * (1.0 - factor) + end.r * factor),
                (byte)(start.g * (1.0 - factor) + end.g * factor),
                (byte)(start.b * (1.0 - factor) + end.b * factor)
            );
            gradient_colors.Add(c);
        }
        
        Set(gradient_colors);
        SetGrid(1, steps);
    }
};

// HSL-based color pusher
class HSLColorPusher : public ColorPusher {
public:
    HSLColorPusher(double hue, int saturation_steps = 5, int lightness_steps = 5) {
        Vector<Color> hsl_colors;
        
        for (int s = 0; s < saturation_steps; s++) {
            for (int l = 0; l < lightness_steps; l++) {
                double sat = (double)s / (saturation_steps - 1);
                double light = 0.2 + (double)l / lightness_steps * 0.6; // Range 0.2 to 0.8
                hsl_colors.Add(HSLtoRGB(hue, sat, light));
            }
        }
        
        Set(hsl_colors);
        SetGrid(saturation_steps, lightness_steps);
    }
    
private:
    Color HSLtoRGB(double h, double s, double l) {
        if (s == 0) {
            // Achromatic (gray)
            byte gray = (byte)(l * 255);
            return Color(gray, gray, gray);
        }
        
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        
        double r = HueToRGB(p, q, h + 1.0/3);
        double g = HueToRGB(p, q, h);
        double b = HueToRGB(p, q, h - 1.0/3);
        
        return Color((byte)(r * 255), (byte)(g * 255), (byte)(b * 255));
    }
    
    double HueToRGB(double p, double q, double t) {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0/6) return p + (q - p) * 6 * t;
        if (t < 1.0/2) return q;
        if (t < 2.0/3) return p + (q - p) * (2.0/3 - t) * 6;
        return p;
    }
};

// Helper functions
inline std::shared_ptr<ColorPusher> CreateColorPusher() {
    return std::make_shared<ColorPusher>();
}

inline std::shared_ptr<StandardColorPusher> CreateStandardColorPusher() {
    return std::make_shared<StandardColorPusher>();
}

inline std::shared_ptr<WebSafeColorPusher> CreateWebSafeColorPusher() {
    return std::make_shared<WebSafeColorPusher>();
}

inline std::shared_ptr<GradientColorPusher> CreateGradientColorPusher(Color start, Color end, int steps = 10) {
    return std::make_shared<GradientColorPusher>(start, end, steps);
}

inline std::shared_ptr<HSLColorPusher> CreateHSLColorPusher(double hue, int saturation_steps = 5, int lightness_steps = 5) {
    return std::make_shared<HSLColorPusher>(hue, saturation_steps, lightness_steps);
}

#endif