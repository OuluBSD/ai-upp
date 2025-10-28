#pragma once
// U++-compatible SliderCtrl wrapper for UI sliders
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

#include <string>
#include <algorithm>
#include "../Draw/Color.h"
#include "../Draw/DrawCore.h"
#include "../CtrlCore/Ctrl.h"
#include "../Draw/Point.h"
#include "../Draw/Rect.h"
#include "../CtrlCore/Event.h"

class SliderCtrl : public Ctrl {
private:
    bool vertical;
    int min_val;           // Minimum value
    int max_val;           // Maximum value
    int value;             // Current value
    int tick_freq;         // Tick frequency
    bool show_ticks;       // Whether to show ticks
    bool dragging;         // Whether currently dragging the thumb
    Point drag_start;      // Starting drag position
    int drag_start_value;  // Value at start of drag

public:
    // Constructors
    SliderCtrl() : Ctrl(), vertical(false), min_val(0), max_val(100), 
                   value(50), tick_freq(10), show_ticks(true), dragging(false) {
        SetSize(200, 20);  // Default horizontal size
    }

    explicit SliderCtrl(bool is_vertical) : Ctrl(), vertical(is_vertical), 
                   min_val(0), max_val(100), value(50), tick_freq(10), 
                   show_ticks(true), dragging(false) {
        SetSize(is_vertical ? 20 : 200, is_vertical ? 200 : 20);
    }

    // U++-style static constructors
    static SliderCtrl* Create() { return new SliderCtrl(); }
    static SliderCtrl* CreateVertical() { return new SliderCtrl(true); }
    static SliderCtrl* CreateHorizontal() { return new SliderCtrl(false); }

    // U++-style slider configuration
    void Horz() { 
        vertical = false; 
        SetSize(200, 20);
    }
    
    void Vert() { 
        vertical = true; 
        SetSize(20, 200);
    }

    // U++-style value and range methods
    void SetRange(int minv, int maxv) { 
        min_val = minv; 
        max_val = maxv; 
        value = std::max(min_val, std::min(value, max_val));
        Refresh();
    }
    
    void SetValue(int v) { 
        value = std::max(min_val, std::min(v, max_val)); 
        Refresh();
    }
    
    void SetTickFreq(int freq) { 
        tick_freq = std::max(1, freq); 
        Refresh();
    }
    
    void NoTicks() { 
        show_ticks = false; 
        Refresh();
    }

    // U++-style getters
    int GetMin() const { return min_val; }
    int GetMax() const { return max_val; }
    int GetValue() const { return value; }
    int GetTickFreq() const { return tick_freq; }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        Rect r = GetRect();
        
        // Draw track
        if (vertical) {
            Rect track_rect(r.GetLeft() + 8, r.GetTop() + 10, r.GetRight() - 8, r.GetBottom() - 10);
            draw.DrawRect(track_rect, Color(150, 150, 150));
        } else {
            Rect track_rect(r.GetLeft() + 10, r.GetTop() + 8, r.GetRight() - 10, r.GetBottom() - 8);
            draw.DrawRect(track_rect, Color(150, 150, 150));
        }
        
        // Draw ticks if enabled
        if (show_ticks) {
            if (vertical) {
                int range = max_val - min_val;
                if (range > 0) {
                    int track_top = r.GetTop() + 10;
                    int track_bottom = r.GetBottom() - 10;
                    int track_height = track_bottom - track_top;
                    
                    for (int v = min_val; v <= max_val; v += tick_freq) {
                        int y = track_top + ((max_val - v) * track_height) / range;
                        draw.DrawLine(r.GetLeft() + 2, y, r.GetLeft() + 7, y, Color::Black());
                        draw.DrawLine(r.GetRight() - 7, y, r.GetRight() - 2, y, Color::Black());
                    }
                }
            } else {
                int range = max_val - min_val;
                if (range > 0) {
                    int track_left = r.GetLeft() + 10;
                    int track_right = r.GetRight() - 10;
                    int track_width = track_right - track_left;
                    
                    for (int v = min_val; v <= max_val; v += tick_freq) {
                        int x = track_left + ((v - min_val) * track_width) / range;
                        draw.DrawLine(x, r.GetTop() + 2, x, r.GetTop() + 7, Color::Black());
                        draw.DrawLine(x, r.GetBottom() - 7, x, r.GetBottom() - 2, Color::Black());
                    }
                }
            }
        }
        
        // Draw thumb
        Rect thumb_rect = GetThumbRect();
        draw.DrawRect(thumb_rect, Color(200, 200, 200), Color(180, 180, 180));
        draw.DrawRect(thumb_rect, Color::Black());  // Border
    }

    // Calculate thumb position and size
    Rect GetThumbRect() const {
        Rect r = GetRect();
        
        int range = max_val - min_val;
        if (range <= 0) {
            // Return middle position if range is invalid
            if (vertical) {
                return Rect(r.GetLeft() + 4, r.GetTop() + (r.GetHeight() / 2) - 7, 
                           r.GetRight() - 4, r.GetTop() + (r.GetHeight() / 2) + 7);
            } else {
                return Rect(r.GetLeft() + (r.GetWidth() / 2) - 7, r.GetTop() + 4, 
                           r.GetLeft() + (r.GetWidth() / 2) + 7, r.GetBottom() - 4);
            }
        }
        
        if (vertical) {
            int track_top = r.GetTop() + 10;
            int track_bottom = r.GetBottom() - 10;
            int track_height = track_bottom - track_top;
            
            int thumb_pos = track_top + ((max_val - value) * track_height) / range;
            return Rect(r.GetLeft() + 4, thumb_pos - 7, r.GetRight() - 4, thumb_pos + 7);
        } else {
            int track_left = r.GetLeft() + 10;
            int track_right = r.GetRight() - 10;
            int track_width = track_right - track_left;
            
            int thumb_pos = track_left + ((value - min_val) * track_width) / range;
            return Rect(thumb_pos - 7, r.GetTop() + 4, thumb_pos + 7, r.GetBottom() - 4);
        }
    }

    // U++-style mouse handling
    void LeftDown(Point p, dword keyflags) override {
        Rect thumb_rect = GetThumbRect();
        
        if (thumb_rect.IsPtInside(p)) {
            // Clicked on thumb - start dragging
            dragging = true;
            drag_start = p;
            drag_start_value = value;
            SetCapture();
        } else {
            // Clicked elsewhere on track - jump to position
            if (vertical) {
                Rect track_rect(GetRect().GetLeft() + 10, GetRect().GetTop() + 10, 
                               GetRect().GetRight() - 10, GetRect().GetBottom() - 10);
                if (track_rect.IsPtInside(p)) {
                    int track_top = GetRect().GetTop() + 10;
                    int track_bottom = GetRect().GetBottom() - 10;
                    int track_height = track_bottom - track_top;
                    
                    int new_value = max_val - ((p.y - track_top) * (max_val - min_val)) / track_height;
                    SetValue(std::max(min_val, std::min(new_value, max_val)));
                }
            } else {
                Rect track_rect(GetRect().GetLeft() + 10, GetRect().GetTop() + 10, 
                               GetRect().GetRight() - 10, GetRect().GetBottom() - 10);
                if (track_rect.IsPtInside(p)) {
                    int track_left = GetRect().GetLeft() + 10;
                    int track_right = GetRect().GetRight() - 10;
                    int track_width = track_right - track_left;
                    
                    int new_value = min_val + ((p.x - track_left) * (max_val - min_val)) / track_width;
                    SetValue(std::max(min_val, std::min(new_value, max_val)));
                }
            }
        }
    }

    void LeftUp(Point p, dword keyflags) override {
        if (dragging) {
            dragging = false;
            ReleaseCapture();
        }
    }

    void MouseMove(Point p, dword keyflags) override {
        if (dragging) {
            int range = max_val - min_val;
            if (range > 0) {
                int new_value;
                if (vertical) {
                    int track_top = GetRect().GetTop() + 10;
                    int track_bottom = GetRect().GetBottom() - 10;
                    int track_height = track_bottom - track_top;
                    
                    int pos_diff = p.y - drag_start.y;
                    int pixel_range = track_height;
                    int value_change = (pos_diff * range) / pixel_range;
                    new_value = drag_start_value - value_change;
                } else {
                    int track_left = GetRect().GetLeft() + 10;
                    int track_right = GetRect().GetRight() - 10;
                    int track_width = track_right - track_left;
                    
                    int pos_diff = p.x - drag_start.x;
                    int pixel_range = track_width;
                    int value_change = (pos_diff * range) / pixel_range;
                    new_value = drag_start_value + value_change;
                }
                
                SetValue(std::max(min_val, std::min(new_value, max_val)));
            }
        }
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "SliderCtrl"; }
};