// U++-compatible Splitter wrapper for UI splitters
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

class Splitter : public CtrlBase {
private:
    bool vertical;
    int position;           // Splitter position in pixels
    int min_pos;           // Minimum position
    int max_pos;           // Maximum position
    std::shared_ptr<Ctrl> left_ctrl;   // Left/top control
    std::shared_ptr<Ctrl> right_ctrl;  // Right/bottom control
    bool dragging;
    Point drag_start;
    int drag_start_pos;

public:
    // Constructors
    Splitter() : CtrlBase(), vertical(false), position(100), 
                 min_pos(20), max_pos(500), dragging(false) {
        SetSize(200, 150);  // Default size
    }

    explicit Splitter(bool is_vertical) : CtrlBase(), vertical(is_vertical), 
                position(100), min_pos(20), max_pos(500), dragging(false) {
        SetSize(vertical ? 150 : 200, vertical ? 200 : 150);
    }

    // U++-style static constructors
    static Splitter* Create() { return new Splitter(); }
    static Splitter* CreateVertical() { return new Splitter(true); }
    static Splitter* CreateHorizontal() { return new Splitter(false); }

    // U++-style splitter configuration
    Splitter& Horz() { 
        vertical = false; 
        return *this; 
    }
    
    Splitter& Vert() { 
        vertical = true; 
        return *this; 
    }

    // U++-style control attachment
    Splitter& First(const std::shared_ptr<Ctrl>& ctrl) { 
        left_ctrl = ctrl; 
        return *this; 
    }
    
    Splitter& Second(const std::shared_ptr<Ctrl>& ctrl) { 
        right_ctrl = ctrl; 
        return *this; 
    }

    // U++-style positioning
    Splitter& SetPos(int pos) { 
        position = std::max(min_pos, std::min(pos, max_pos));
        RefreshLayout(); 
        return *this; 
    }
    
    int GetPos() const { return position; }

    // U++-style range configuration
    Splitter& SetPosRange(int minp, int maxp) { 
        min_pos = minp; 
        max_pos = maxp; 
        position = std::max(min_pos, std::min(position, max_pos));
        RefreshLayout(); 
        return *this; 
    }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        Rect r = GetRect();
        Color bg_color = GetBackgroundColor();
        
        // Draw splitter background
        draw.DrawRect(r, bg_color);

        // Draw splitter bar
        Rect bar_rect;
        if (vertical) {
            bar_rect = Rect(r.GetLeft() + position - 2, r.GetTop(), 
                           r.GetLeft() + position + 2, r.GetBottom());
        } else {
            bar_rect = Rect(r.GetLeft(), r.GetTop() + position - 2, 
                           r.GetRight(), r.GetTop() + position + 2);
        }
        
        draw.DrawRect(bar_rect, Color::Gray());
        
        // Draw gripper
        if (vertical) {
            int center_y = r.GetTop() + (r.GetHeight() / 2);
            for (int i = -6; i <= 6; i += 3) {
                draw.DrawPoint(Point(r.GetLeft() + position + i, center_y - 6), Color::White());
                draw.DrawPoint(Point(r.GetLeft() + position + i, center_y + 6), Color::White());
            }
        } else {
            int center_x = r.GetLeft() + (r.GetWidth() / 2);
            for (int i = -6; i <= 6; i += 3) {
                draw.DrawPoint(Point(center_x - 6, r.GetTop() + position + i), Color::White());
                draw.DrawPoint(Point(center_x + 6, r.GetTop() + position + i), Color::White());
            }
        }

        // Paint child controls
        if (left_ctrl && left_ctrl->IsVisible()) {
            left_ctrl->Paint(draw);
        }
        if (right_ctrl && right_ctrl->IsVisible()) {
            right_ctrl->Paint(draw);
        }
    }

    // U++-style mouse handling
    void LeftDown(Point p, dword keyflags) override {
        Rect bar_rect;
        if (vertical) {
            bar_rect = Rect(GetRect().GetLeft() + position - 4, GetRect().GetTop(), 
                           GetRect().GetLeft() + position + 4, GetRect().GetBottom());
        } else {
            bar_rect = Rect(GetRect().GetLeft(), GetRect().GetTop() + position - 4, 
                           GetRect().GetRight(), GetRect().GetTop() + position + 4);
        }
        
        if (bar_rect.IsPtInside(p)) {
            dragging = true;
            drag_start = p;
            drag_start_pos = position;
            SetCapture();
        }
    }

    void LeftUp(Point p, dword keyflags) override {
        if (dragging) {
            dragging = false;
            ReleaseCapture();
            RefreshLayout();
        }
    }

    void MouseMove(Point p, dword keyflags) override {
        if (dragging) {
            int new_pos;
            if (vertical) {
                new_pos = drag_start_pos + (p.x - drag_start.x);
            } else {
                new_pos = drag_start_pos + (p.y - drag_start.y);
            }
            
            position = std::max(min_pos, std::min(new_pos, max_pos));
            RefreshLayout();
            Refresh();
        } else {
            // Change cursor when hovering over splitter bar
            Rect bar_rect;
            if (vertical) {
                bar_rect = Rect(GetRect().GetLeft() + position - 4, GetRect().GetTop(), 
                               GetRect().GetLeft() + position + 4, GetRect().GetBottom());
            } else {
                bar_rect = Rect(GetRect().GetLeft(), GetRect().GetTop() + position - 4, 
                               GetRect().GetRight(), GetRect().GetTop() + position + 4);
            }
            
            if (bar_rect.IsPtInside(p)) {
                // In a real implementation, this would set the appropriate cursor
                // SetCursor(...);
            }
        }
    }

    // U++-style refresh layout
    void RefreshLayout() {
        Rect r = GetRect();
        if (!left_ctrl && !right_ctrl) return;

        if (left_ctrl) {
            Rect left_rect;
            if (vertical) {
                left_rect = Rect(r.GetLeft(), r.GetTop(), 
                                r.GetLeft() + position, r.GetBottom());
            } else {
                left_rect = Rect(r.GetLeft(), r.GetTop(), 
                                r.GetRight(), r.GetTop() + position);
            }
            left_ctrl->SetRect(left_rect);
        }

        if (right_ctrl) {
            Rect right_rect;
            if (vertical) {
                right_rect = Rect(r.GetLeft() + position, r.GetTop(), 
                                 r.GetRight(), r.GetBottom());
            } else {
                right_rect = Rect(r.GetLeft(), r.GetTop() + position, 
                                 r.GetRight(), r.GetBottom());
            }
            right_ctrl->SetRect(right_rect);
        }
    }

    // U++-style resize handling
    void Size() override {
        RefreshLayout();
        if (left_ctrl) left_ctrl->Size();
        if (right_ctrl) right_ctrl->Size();
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "Splitter"; }
};