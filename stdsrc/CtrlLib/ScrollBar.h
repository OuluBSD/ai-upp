#pragma once
// U++-compatible ScrollBar wrapper for UI scrollbars
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

#include <string>
#include <algorithm>
#include "../Draw/Color.h"
#include "../Draw/DrawCore.h"
#include "../CtrlCore/Ctrl.h"
#include "../Draw/Point.h"
#include "../Draw/Rect.h"
#include "../CtrlCore/Event.h"

class ScrollBar : public Ctrl {
private:
    bool vertical;
    int page;              // Page size (visible area)
    int total;             // Total range
    int pos;               // Current position
    int line;              // Line increment
    bool dragging;
    Point drag_start;
    int drag_start_pos;
    int thumb_start_pos;
    int thumb_size;

public:
    // Constructors
    ScrollBar() : Ctrl(), vertical(true), page(10), total(100), 
                  pos(0), line(1), dragging(false), thumb_size(20) {
        SetSize(16, 100);  // Default vertical scrollbar size
    }

    explicit ScrollBar(bool is_vertical) : Ctrl(), vertical(is_vertical), 
                  page(10), total(100), pos(0), line(1), dragging(false), thumb_size(20) {
        SetSize(is_vertical ? 16 : 100, is_vertical ? 100 : 16);
    }

    // U++-style static constructors
    static ScrollBar* Create() { return new ScrollBar(); }
    static ScrollBar* CreateVertical() { return new ScrollBar(true); }
    static ScrollBar* CreateHorizontal() { return new ScrollBar(false); }

    // U++-style scrollbar configuration
    void Horz() { 
        vertical = false; 
        SetSize(100, 16);
    }
    
    void Vert() { 
        vertical = true; 
        SetSize(16, 100);
    }

    // U++-style range and position methods
    void SetPage(int p) { 
        page = std::max(1, p); 
        pos = std::min(pos, std::max(0, total - page));
        Refresh();
    }
    
    void SetTotal(int t) { 
        total = std::max(page, t); 
        pos = std::min(pos, std::max(0, total - page));
        Refresh();
    }
    
    void SetLine(int l) { 
        line = std::max(1, l); 
    }

    void SetPos(int p) { 
        pos = std::max(0, std::min(p, std::max(0, total - page))); 
        Refresh();
    }

    // U++-style getters
    int GetPage() const { return page; }
    int GetTotal() const { return total; }
    int GetLine() const { return line; }
    int GetPos() const { return pos; }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        Rect r = GetRect();
        
        // Draw scrollbar background
        draw.DrawRect(r, Color(220, 220, 220), Color(200, 200, 200));
        
        // Draw thumb
        Rect thumb_rect = GetThumbRect();
        draw.DrawRect(thumb_rect, Color(180, 180, 180), Color(210, 210, 210));
        
        // Draw thumb griplines
        if (vertical) {
            int center_x = thumb_rect.GetLeft() + (thumb_rect.GetWidth() / 2);
            for (int y = thumb_rect.GetTop() + 4; y < thumb_rect.GetBottom() - 4; y += 3) {
                draw.DrawPoint(Point(center_x - 2, y), Color::Gray());
                draw.DrawPoint(Point(center_x + 2, y), Color::Gray());
            }
        } else {
            int center_y = thumb_rect.GetTop() + (thumb_rect.GetHeight() / 2);
            for (int x = thumb_rect.GetLeft() + 4; x < thumb_rect.GetRight() - 4; x += 3) {
                draw.DrawPoint(Point(x, center_y - 2), Color::Gray());
                draw.DrawPoint(Point(x, center_y + 2), Color::Gray());
            }
        }
    }

    // Calculate thumb position and size
    Rect GetThumbRect() const {
        Rect r = GetRect();
        
        if (total <= page) {
            // No scrolling needed, hide thumb
            return Rect(0, 0, 0, 0);
        }
        
        int available_length = vertical ? (r.GetHeight() - 4) : (r.GetWidth() - 4);
        thumb_size = std::max(10, (page * available_length) / total);
        
        int range = total - page;
        int thumb_pos;
        if (range > 0) {
            thumb_pos = (pos * (available_length - thumb_size)) / range;
        } else {
            thumb_pos = 0;
        }
        
        if (vertical) {
            return Rect(r.GetLeft() + 2, r.GetTop() + 2 + thumb_pos, 
                       r.GetRight() - 2, r.GetTop() + 2 + thumb_pos + thumb_size);
        } else {
            return Rect(r.GetLeft() + 2 + thumb_pos, r.GetTop() + 2, 
                       r.GetLeft() + 2 + thumb_pos + thumb_size, r.GetBottom() - 2);
        }
    }

    // U++-style mouse handling
    void LeftDown(Point p, dword keyflags) override {
        Rect thumb_rect = GetThumbRect();
        Rect r = GetRect();
        
        if (thumb_rect.IsPtInside(p)) {
            // Clicked on thumb
            dragging = true;
            drag_start = p;
            drag_start_pos = pos;
            thumb_start_pos = vertical ? (thumb_rect.GetTop() - r.GetTop() - 2) : (thumb_rect.GetLeft() - r.GetLeft() - 2);
            SetCapture();
        } else {
            // Clicked on track - page up/down
            if (vertical) {
                if (p.y < thumb_rect.GetTop()) {
                    // Page up
                    SetPos(std::max(0, pos - page));
                } else if (p.y > thumb_rect.GetBottom()) {
                    // Page down
                    SetPos(std::min(total - page, pos + page));
                }
            } else {
                if (p.x < thumb_rect.GetLeft()) {
                    // Page left
                    SetPos(std::max(0, pos - page));
                } else if (p.x > thumb_rect.GetRight()) {
                    // Page right
                    SetPos(std::min(total - page, pos + page));
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
            Rect r = GetRect();
            int available_length = vertical ? (r.GetHeight() - 4 - thumb_size) : (r.GetWidth() - 4 - thumb_size);
            
            int new_pos;
            if (available_length > 0) {
                int delta = vertical ? (p.y - drag_start.y) : (p.x - drag_start.x);
                int new_thumb_pos = thumb_start_pos + delta;
                new_thumb_pos = std::max(0, std::min(new_thumb_pos, available_length));
                
                int range = total - page;
                new_pos = (new_thumb_pos * range) / available_length;
                SetPos(new_pos);
            }
        }
    }

    // U++-style wheel handling
    void MouseWheel(Point p, int zdelta, dword keyflags) override {
        if (zdelta > 0) {
            SetPos(pos - line);  // Scroll up/prev
        } else {
            SetPos(pos + line);  // Scroll down/next
        }
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "ScrollBar"; }
};