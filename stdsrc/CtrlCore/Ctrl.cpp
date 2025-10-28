// STL-backed CtrlCore API implementation

#include "Ctrl.h"

namespace Upp {

// Constructors
Ctrl::Ctrl() : rect(0, 0, 100, 100), background_color(Color::White()), 
               enabled(true), visible(true) {}

Ctrl::Ctrl(const Rect& r) : rect(r), background_color(Color::White()), 
                            enabled(true), visible(true) {}

// U++-style size and position methods
void Ctrl::SetRect(const Rect& r) { 
    rect = r; 
}

void Ctrl::SetRect(int x, int y, int cx, int cy) { 
    SetRect(Rect(x, y, cx, cy)); 
}

Rect Ctrl::GetRect() const { 
    return rect; 
}

Rect Ctrl::GetScreenRect() const { 
    // In a real implementation, this would calculate screen coordinates
    return rect; 
}

Point Ctrl::GetScreenPoint(const Point& pt) const { 
    // In a real implementation, this would convert to screen coordinates
    return pt + rect.GetTopLeft(); 
}

Size Ctrl::GetSize() const { 
    return rect.GetSize(); 
}

Point Ctrl::GetPos() const { 
    return rect.GetTopLeft(); 
}

void Ctrl::SetSize(const Size& sz) { 
    rect.right = rect.left + sz.cx;
    rect.bottom = rect.top + sz.cy;
}

void Ctrl::SetSize(int cx, int cy) { 
    SetSize(Size(cx, cy)); 
}

void Ctrl::SetPos(const Point& pt) { 
    Point current_pos = rect.GetTopLeft();
    int dx = pt.x - current_pos.x;
    int dy = pt.y - current_pos.y;
    rect.left += dx;
    rect.right += dx;
    rect.top += dy;
    rect.bottom += dy;
}

void Ctrl::SetPos(int x, int y) { 
    SetPos(Point(x, y)); 
}

// U++-style parent-child relationship
std::shared_ptr<Ctrl> Ctrl::GetParent() const { 
    return parent.lock(); 
}

void Ctrl::SetParent(const std::shared_ptr<Ctrl>& p) { 
    parent = p; 
}

void Ctrl::AddChild(const std::shared_ptr<Ctrl>& child) {
    if (child) {
        children.push_back(child);
        child->SetParent(shared_from_this());
    }
}

void Ctrl::RemoveChild(const std::shared_ptr<Ctrl>& child) {
    children.erase(
        std::remove(children.begin(), children.end(), child),
        children.end()
    );
    if (child) {
        child->SetParent(nullptr);
    }
}

// U++-style visual properties
void Ctrl::SetLabel(const std::string& lbl) { 
    label = lbl; 
}

std::string Ctrl::GetLabel() const { 
    return label; 
}

void Ctrl::SetBackgroundColor(const Color& color) { 
    background_color = color; 
}

Color Ctrl::GetBackgroundColor() const { 
    return background_color; 
}

void Ctrl::SetEnabled(bool enable) { 
    enabled = enable; 
}

bool Ctrl::IsEnabled() const { 
    return enabled; 
}

void Ctrl::SetVisible(bool show) { 
    visible = show; 
}

bool Ctrl::IsVisible() const { 
    return visible; 
}

void Ctrl::SetToolTip(const std::string& tip) { 
    tooltip = tip; 
}

std::string Ctrl::GetToolTip() const { 
    return tooltip; 
}

// U++-style painting
void Ctrl::Paint(Draw& draw) const {
    // Default implementation just paints background
    if (visible) {
        draw.DrawRect(rect, background_color);
        // Render label text if present
        if (!label.empty()) {
            // In a real implementation, this would use proper text rendering
        }
    }
}

// U++-style event handling
void Ctrl::SetHandler(const std::function<void()>& handler) { 
    click_handler = handler; 
}

void Ctrl::SetHandler(const std::function<void(const Event&)>& handler) { 
    event_handler = handler; 
}

bool Ctrl::IsPointInside(const Point& pt) const {
    return rect.IsPtInside(pt);
}

// U++-style control operations
void Ctrl::Show() { 
    SetVisible(true); 
}

void Ctrl::Hide() { 
    SetVisible(false); 
}

void Ctrl::Enable() { 
    SetEnabled(true); 
}

void Ctrl::Disable() { 
    SetEnabled(false); 
}

// U++-style child control enumeration
int Ctrl::GetChildCount() const { 
    return static_cast<int>(children.size()); 
}

std::shared_ptr<Ctrl> Ctrl::GetChild(int index) const {
    if (index >= 0 && index < static_cast<int>(children.size())) {
        return children[index];
    }
    return nullptr;
}

// U++-style painting helper
void Ctrl::Refresh() {
    // In a real implementation, this would trigger a repaint
}

void Ctrl::Refresh(const Rect& r) {
    // In a real implementation, this would trigger a partial repaint
}

// U++-style coordinate transformations
Point Ctrl::ClientToScreen(const Point& pt) const {
    return pt + rect.GetTopLeft();
}

Point Ctrl::ScreenToClient(const Point& pt) const {
    return pt - rect.GetTopLeft();
}

// U++-style methods for identifying control types
const char* Ctrl::GetClassName() const { 
    return "Ctrl"; 
}

bool Ctrl::IsNull() const { 
    return false; 
}

bool Ctrl::Is() const { 
    return true; 
}

bool Ctrl::IsOpen() const { 
    return Is(); // Basic control always "open"
}

// U++-style operations for UI updates
void Ctrl::SetFocus() {
    // In a real implementation, this would set focus to the control
}

bool Ctrl::HasFocus() const { 
    // In a real implementation, this would check if control has focus
    return false; 
}

// U++-style operations for UI layout
void Ctrl::SetFrame(int left, int top, int right, int bottom) {
    // In a real implementation, this would set frame margins
}

void Ctrl::SetFrameRect(const Rect& frame) {
    SetFrame(frame.GetLeft(), frame.GetTop(), frame.GetRight(), frame.GetBottom());
}

Rect Ctrl::GetFrameRect() const {
    // In a real implementation, this would return frame margins
    return Rect(0, 0, 0, 0);
}

}