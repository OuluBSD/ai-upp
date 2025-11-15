#include "TimelineCtrl.h"
#include <Draw/Draw.h>

TimelineCtrl::TimelineCtrl()
    : project(nullptr)
    , animation(nullptr)
    , selected_frame_index(-1)
    , drag_start_index(-1)
    , drag_current_index(-1)
    , is_dragging(false)
    , frame_width(80)
    , frame_height(60)
    , frame_spacing(10)
{
    SetFrameProfile(1);
    selected_frame_index = -1;
}

TimelineCtrl::~TimelineCtrl()
{
}

void TimelineCtrl::SetProject(const AnimationProject* proj) {
    project = proj;
}

void TimelineCtrl::SetAnimation(const Animation* anim) {
    animation = anim;
    selected_frame_index = -1; // Reset selection
    RefreshLayout();
    Refresh();
}

void TimelineCtrl::SetFrameCallback(std::function<void(const Frame*)> callback) {
    frame_callback = callback;
}

void TimelineCtrl::SetOnFrameModified(std::function<void()> callback) {
    on_frame_modified_callback = callback;
}

int TimelineCtrl::HitTest(Point pos) const {
    if (!animation) return -1;
    
    int current_x = 4; // Padding
    for (int i = 0; i < animation->frames.GetCount(); i++) {
        Rect frame_rc(current_x, 4, current_x + frame_width, 4 + frame_height);
        if (frame_rc.Contains(pos)) {
            return i;
        }
        current_x += frame_width + frame_spacing;
    }
    return -1;
}

void TimelineCtrl::DrawFrame(Draw& w, int index, const Rect& rc) {
    if (!animation || index < 0 || index >= animation->frames.GetCount()) return;
    
    const FrameRef& ref = animation->frames[index];
    const Frame* frame = project ? project->FindFrame(ref.frame_id) : nullptr;
    
    // Draw frame background
    Color bg_color = (index == selected_frame_index) ? RGB(50, 150, 200) : RGB(200, 200, 200);
    w.DrawRect(rc, bg_color);
    
    // Draw frame border
    w.DrawRect(rc, 1, Black());
    
    // Draw frame index
    String frame_text = IntStr(index);
    w.DrawText(rc.left + 4, rc.top + 2, frame_text, StdFont(), Black());
    
    // Draw frame duration if different from default
    if (ref.has_duration) {
        String dur_text = Format("%.2f", ref.duration) + "s";
        w.DrawText(rc.left + 4, rc.top + 20, dur_text, StdFont(), Black());
    } else if (frame) {
        String dur_text = Format("%.2f", frame->default_duration) + "s";
        w.DrawText(rc.left + 4, rc.top + 20, dur_text, StdFont(), Black());
    }
    
    // Draw frame ID or name if available
    if (frame) {
        String name = frame->name;
        if (name.IsEmpty()) name = frame->id;
        if (name.GetLength() > 8) name = name.Mid(0, 8) + "...";
        w.DrawText(rc.left + 4, rc.top + 38, name, StdFont(), Black());
    }
}

void TimelineCtrl::RefreshLayout() {
    if (!animation) return;
    
    // Calculate how many frames we can show
    int available_width = GetSize().cx - 8; // Account for padding
    int total_frame_space = (frame_width + frame_spacing) * animation->frames.GetCount() - frame_spacing;
    
    // If all frames fit, we're fine; otherwise we might need to implement scrolling
    // For now, just ensure we don't overflow the available space
    Refresh();
}

void TimelineCtrl::UpdateScroll() {
    // For now, a simple scroll mechanism - would need to be more sophisticated for production
}

void TimelineCtrl::SelectFrame(int index) {
    if (index == selected_frame_index) return;
    
    selected_frame_index = index;
    
    if (frame_callback && animation && index >= 0 && index < animation->frames.GetCount()) {
        const FrameRef& ref = animation->frames[index];
        const Frame* frame = project ? project->FindFrame(ref.frame_id) : nullptr;
        if (frame) {
            frame_callback(frame);
        }
    }
    
    Refresh();
}

void TimelineCtrl::Paint(Draw& w) {
    w.DrawRect(GetSize(), SColorFace());
    
    if (!animation) return;

    int current_x = 4; // Left padding
    int y_pos = 4;     // Top padding
    
    for (int i = 0; i < animation->frames.GetCount(); i++) {
        Rect frame_rc(current_x, y_pos, current_x + frame_width, y_pos + frame_height);
        DrawFrame(w, i, frame_rc);
        current_x += frame_width + frame_spacing;
    }
}

void TimelineCtrl::MouseDown(Point pos, dword button) {
    Ctrl::MouseDown(pos, button);
    
    int frame_idx = HitTest(pos);
    if (frame_idx >= 0) {
        SelectFrame(frame_idx);
    }
}

void TimelineCtrl::LeftDown(Point pos, dword flags) {
    int frame_idx = HitTest(pos);
    if (frame_idx >= 0) {
        // Start drag operation if we're clicking on a frame
        drag_start_index = frame_idx;
        is_dragging = false; // We'll set this to true after a threshold movement
        SetCapture();
    } else {
        // Clicked outside a frame, deselect
        if (selected_frame_index != -1) {
            selected_frame_index = -1;
            Refresh();
        }
    }
    Ctrl::LeftDown(pos, flags);
}

void TimelineCtrl::LeftUp(Point pos, dword flags) {
    if (IsCapture()) {
        ReleaseCapture();
    }
    
    if (is_dragging && drag_start_index >= 0 && drag_current_index >= 0 && drag_start_index != drag_current_index) {
        // Perform the reordering
        ReorderFrame(drag_start_index, drag_current_index);
        if (on_frame_modified_callback) {
            on_frame_modified_callback();
        }
    }
    
    drag_start_index = -1;
    drag_current_index = -1;
    is_dragging = false;
    Ctrl::LeftUp(pos, flags);
}

void TimelineCtrl::MouseMove(Point pos, dword keyflags) {
    Ctrl::MouseMove(pos, keyflags);

    if (IsCapture() && drag_start_index >= 0) {
        if (!is_dragging && IsDragThresholdExceeded(pos)) {
            is_dragging = true;
        }

        if (is_dragging) {
            // Find the position where we are now to potentially swap frames
            int new_index = HitTest(pos);
            if (new_index >= 0 && new_index != drag_current_index) {
                drag_current_index = new_index;
                // Visual feedback would go here
                Refresh();
            }
        }
    }
}

bool TimelineCtrl::Key(dword key, int count) {
    if (!animation || selected_frame_index < 0) return false;
    
    switch(key) {
        case K_LEFT:
            if (selected_frame_index > 0) {
                SelectFrame(selected_frame_index - 1);
                return true;
            }
            break;
        case K_RIGHT:
            if (selected_frame_index < animation->frames.GetCount() - 1) {
                SelectFrame(selected_frame_index + 1);
                return true;
            }
            break;
        case K_DELETE:
            // Delete the selected frame from the animation
            if (animation && selected_frame_index >= 0) {
                // This would require modification of the animation, which should go through a proper API
                // For now, just show an alert
                PromptOK("Delete frame functionality would go here");
            }
            break;
    }
    return Ctrl::Key(key, count);
}bool TimelineCtrl::IsDragThresholdExceeded(Point pos) {
    if (drag_start_index < 0) return false;
    
    // Get the rect of the starting frame
    int current_x = 4; // Padding
    for (int i = 0; i < animation->frames.GetCount(); i++) {
        Rect frame_rc(current_x, 4, current_x + frame_width, 4 + frame_height);
        if (i == drag_start_index) {
            // Calculate the center of the frame
            Point frame_center = frame_rc.CenterPos();
            int distance = abs(pos.x - frame_center.x);
            static const int DRAG_THRESHOLD = 5;
            return distance > DRAG_THRESHOLD;
        }
        current_x += frame_width + frame_spacing;
    }
    return false; // Should not happen if drag_start_index is valid
}

void TimelineCtrl::ReorderFrame(int from_index, int to_index) {
    if (!animation || from_index < 0 || to_index < 0 || 
        from_index >= animation->frames.GetCount() || to_index >= animation->frames.GetCount()) {
        return;
    }
    
    // Move the frame from from_index to to_index
    FrameRef temp = animation->frames[from_index];
    animation->frames.Remove(from_index);
    animation->frames.Insert(to_index, temp);
    
    // Update the selected index if necessary
    if (selected_frame_index == from_index) {
        selected_frame_index = to_index;
    } else if (selected_frame_index == to_index && from_index < to_index) {
        selected_frame_index = to_index - 1;  // Adjust for the removed element
    } else if (selected_frame_index == to_index && from_index > to_index) {
        selected_frame_index = to_index + 1;  // Adjust for the shifted element
    }
    
    Refresh();
}

void TimelineCtrl::StartDrag(int index) {
    drag_start_index = index;
    is_dragging = false; // We'll set this to true after the threshold is exceeded
    drag_current_index = index;
    SetCapture();
}

void TimelineCtrl::EndDrag() {
    if (IsCapture()) {
        ReleaseCapture();
    }
    drag_start_index = -1;
    drag_current_index = -1;
    is_dragging = false;
}