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

// U++-style timer handling
void Ctrl::SetTimeCallback(int delay_ms, const Event<>& cb, int id) {
    // In a real implementation, this would set a timer callback
    // For now, just store it in a static map for demonstration
}

void Ctrl::KillTimeCallback(int id) {
    // In a real implementation, this would kill a timer callback
}

void Ctrl::KillSetTimeCallback(int delay_ms, const Event<>& cb, int id) {
    // In a real implementation, this would kill and set a timer callback
    KillTimeCallback(id);
    SetTimeCallback(delay_ms, cb, id);
}

bool Ctrl::ExistsTimeCallback(int id) const {
    // In a real implementation, this would check if a timer callback exists
    return false;
}

void Ctrl::PostCallback(const Event<>& cb, int id) {
    // In a real implementation, this would post a callback for immediate execution
}

void Ctrl::KillPostCallback(const Event<>& cb, int id) {
    // In a real implementation, this would kill a posted callback
}

// U++-style keyboard handling
bool Ctrl::Key(dword key, int count) {
    // In a real implementation, this would handle keyboard input
    return false; // Not handled
}

void Ctrl::GotFocus() {
    // In a real implementation, this would be called when control gains focus
}

void Ctrl::LostFocus() {
    // In a real implementation, this would be called when control loses focus
}

bool Ctrl::HotKey(dword key) {
    // In a real implementation, this would handle hotkeys
    return false; // Not handled
}

dword Ctrl::GetAccessKeys() const {
    // In a real implementation, this would return access keys for the control
    return 0;
}

void Ctrl::AssignAccessKeys(dword used) {
    // In a real implementation, this would assign access keys to the control
}

void Ctrl::ChildGotFocus() {
    // In a real implementation, this would be called when a child gains focus
}

void Ctrl::ChildLostFocus() {
    // In a real implementation, this would be called when a child loses focus
}

// Focus-related methods
bool Ctrl::HasFocusDeep() const {
    // In a real implementation, this would check if control or any child has focus
    return HasFocus();
}

Ctrl* Ctrl::GetFocusChild() const {
    // In a real implementation, this would return the focused child if any
    return nullptr;
}

Ctrl* Ctrl::GetFocusChildDeep() const {
    // In a real implementation, this would return the deeply focused child if any
    return nullptr;
}

Ctrl& Ctrl::WantFocus(bool ft) {
    // In a real implementation, this would mark control as wanting focus
    return *this;
}

Ctrl& Ctrl::NoWantFocus() {
    return WantFocus(false);
}

bool Ctrl::IsWantFocus() const {
    // In a real implementation, this would return whether control wants focus
    return false;
}

bool Ctrl::SetWantFocus() {
    // In a real implementation, this would set focus if control wants it
    return false;
}

Ctrl& Ctrl::InitFocus(bool ft) {
    // In a real implementation, this would initialize focus for the control
    return *this;
}

Ctrl& Ctrl::NoInitFocus() {
    return InitFocus(false);
}

bool Ctrl::IsInitFocus() const {
    // In a real implementation, this would return whether focus is initialized
    return false;
}

// Access keys support
void Ctrl::RefreshAccessKeys() {
    // In a real implementation, this would refresh access keys display
}

void Ctrl::RefreshAccessKeysDo(bool vis) {
    // In a real implementation, this would show/hide access keys based on visibility
}

dword Ctrl::GetAccessKeysDeep() const {
    // In a real implementation, this would return access keys for control and children
    dword keys = GetAccessKeys();
    for (const auto& child : children) {
        keys |= child->GetAccessKeysDeep();
    }
    return keys;
}

void Ctrl::DistributeAccessKeys() {
    // In a real implementation, this would distribute access keys among controls
}

bool Ctrl::VisibleAccessKeys() {
    // In a real implementation, this would check if access keys are visible
    return false;
}

// Static focus management
Ctrl* Ctrl::GetFocusCtrl() {
    // In a real implementation, this would return the control that currently has focus
    return nullptr;
}

bool Ctrl::IterateFocusForward(Ctrl *ctrl, Ctrl *top, bool noframe, bool init, bool all) {
    // In a real implementation, this would iterate focus forward
    return false;
}

bool Ctrl::IterateFocusBackward(Ctrl *ctrl, Ctrl *top, bool noframe, bool all) {
    // In a real implementation, this would iterate focus backward
    return false;
}

dword Ctrl::AccessKeyBit(int accesskey) {
    // In a real implementation, this would convert access key to bit mask
    return 0;
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