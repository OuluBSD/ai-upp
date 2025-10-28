#ifndef _CtrlLib_Bar_h_
#define _CtrlLib_Bar_h_

#include "CtrlLib.h"
#include "Ctrl.h"
#include "Button.h"
#include "Label.h"
#include <vector>
#include <functional>

// Menu and toolbar base class
class Bar {
protected:
    Vector<std::shared_ptr<Ctrl>> items;
    bool horizontal;
    int spacing;
    std::function<void()> when_action;
    
public:
    Bar(bool horz = true) : horizontal(horz), spacing(2) {}
    
    // Add a control to the bar
    Bar& Add(const std::shared_ptr<Ctrl>& ctrl) {
        if (ctrl) {
            items.Add(ctrl);
        }
        return *this;
    }
    
    // Add a separator
    Bar& AddSeparator() {
        // In a real implementation, this would add a separator control
        return *this;
    }
    
    // Add a button with text and handler
    Bar& Add(String text, std::function<void()> handler) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(text);
        btn->WhenAction = handler;
        return Add(btn);
    }
    
    // Add a label
    Bar& AddLabel(String text) {
        auto label = std::make_shared<Label>();
        label->SetLabel(text);
        return Add(label);
    }
    
    // Get number of items
    int GetCount() const { return items.GetCount(); }
    
    // Get item at index
    std::shared_ptr<Ctrl> Get(int i) const { 
        return i >= 0 && i < items.GetCount() ? items[i] : nullptr; 
    }
    
    // Clear all items
    Bar& Clear() {
        items.Clear();
        return *this;
    }
    
    // Set layout direction
    Bar& Horz() { horizontal = true; return *this; }
    Bar& Vert() { horizontal = false; return *this; }
    
    // Set spacing between items
    Bar& Spacing(int sp) { spacing = sp; return *this; }
    int GetSpacing() const { return spacing; }
    
    // Set action handler for all items
    Bar& When(std::function<void()> handler) { 
        when_action = handler; 
        return *this; 
    }
    
    // Set enabled/disabled state for all items
    Bar& Enable(bool b = true) {
        for (auto& item : items) {
            if (item) {
                item->SetEnabled(b);
            }
        }
        return *this;
    }
    
    Bar& Disable() { return Enable(false); }
    
    // Set visibility for all items
    Bar& Show(bool b = true) {
        for (auto& item : items) {
            if (item) {
                item->SetVisible(b);
            }
        }
        return *this;
    }
    
    Bar& Hide() { return Show(false); }
    
    // Layout the bar items
    void Layout(Size sz) {
        int pos = 0;
        int max_extent = 0;
        
        if (horizontal) {
            // Layout horizontally
            for (auto& item : items) {
                if (item && item->IsVisible()) {
                    Size item_sz = item->GetSize();
                    item->SetPos(pos, 0);
                    item->SetSize(item_sz.cx, sz.cy);
                    
                    pos += item_sz.cx + spacing;
                    if (item_sz.cy > max_extent) max_extent = item_sz.cy;
                }
            }
        } else {
            // Layout vertically
            for (auto& item : items) {
                if (item && item->IsVisible()) {
                    Size item_sz = item->GetSize();
                    item->SetPos(0, pos);
                    item->SetSize(sz.cx, item_sz.cy);
                    
                    pos += item_sz.cy + spacing;
                    if (item_sz.cx > max_extent) max_extent = item_sz.cx;
                }
            }
        }
    }
    
    // Get minimum required size
    Size GetMinSize() const {
        int total_size = 0;
        int max_extent = 0;
        
        for (const auto& item : items) {
            if (item && item->IsVisible()) {
                Size item_sz = item->GetSize();
                if (horizontal) {
                    total_size += item_sz.cx + spacing;
                    if (item_sz.cy > max_extent) max_extent = item_sz.cy;
                } else {
                    total_size += item_sz.cy + spacing;
                    if (item_sz.cx > max_extent) max_extent = item_sz.cx;
                }
            }
        }
        
        if (total_size > 0) {
            total_size -= spacing; // Remove extra spacing after last item
        }
        
        return horizontal ? Size(total_size, max_extent) : Size(max_extent, total_size);
    }
    
    // Get item count
    int GetCtrlCount() const { return items.GetCount(); }
};

// MenuBar class
class MenuBar : public Bar {
public:
    MenuBar() : Bar(true) {}
    
    // Add a menu
    MenuBar& Add(String menu_name) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(menu_name + " ▼");
        btn->SetStyle(Button::DROPDOWN);
        return Add(btn);
    }
    
    // Add menu with action
    MenuBar& Add(String menu_name, std::function<void()> action) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(menu_name + " ▼");
        btn->SetStyle(Button::DROPDOWN);
        btn->WhenAction = action;
        return Add(btn);
    }
    
    // Set as menu bar (special styling)
    MenuBar& SetMenuBarStyle() {
        // In a real implementation, this would apply menu bar styling
        return *this;
    }
};

// ToolBar class
class ToolBar : public Bar {
private:
    bool flat_style;
    bool show_text;
    
public:
    ToolBar() : Bar(true), flat_style(true), show_text(true) {}
    
    // Add a tool button
    ToolBar& Add(String text, std::function<void()> handler) {
        auto btn = std::make_shared<Button>();
        
        if (show_text) {
            btn->SetLabel(text);
        } else {
            // In a real implementation, this might use an icon
            btn->SetLabel(text); // Simplified
        }
        
        btn->WhenAction = handler;
        
        if (flat_style) {
            // In a real implementation, this would apply flat button styling
        }
        
        return Add(btn);
    }
    
    // Add a tool button with image
    ToolBar& Add(const Image& img, std::function<void()> handler, String tooltip = String()) {
        auto btn = std::make_shared<Button>();
        
        // In a real implementation, this would set the image on the button
        btn->SetLabel("Btn"); // Simplified
        btn->WhenAction = handler;
        if (!tooltip.IsEmpty()) {
            btn->SetToolTip(tooltip);
        }
        
        return Add(btn);
    }
    
    // Set toolbar style
    ToolBar& Flat(bool b = true) { flat_style = b; return *this; }
    ToolBar& ShowText(bool b = true) { show_text = b; return *this; }
    
    // Add a control that isn't a button
    ToolBar& AddCtrl(const std::shared_ptr<Ctrl>& ctrl) {
        return Add(ctrl);
    }
};

// PopupBar - for context menus and popup menus
class PopupBar : public Bar {
public:
    PopupBar() : Bar(true) {}
    
    // Add an item that can be checked
    PopupBar& Add(String text, std::function<void()> handler, bool checked = false) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(checked ? "✓ " + text : "  " + text);
        btn->WhenAction = handler;
        return Add(btn);
    }
    
    // Add a checkable item
    PopupBar& AddCheck(String text, std::function<void(bool)> handler, bool initial_state = false) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(initial_state ? "✓ " + text : "  " + text);
        btn->WhenAction = [handler]() { handler(true); }; // Simplified
        return Add(btn);
    }
    
    // Add radio item (mutually exclusive with other radio items)
    PopupBar& AddRadio(String text, std::function<void()> handler, bool selected = false) {
        auto btn = std::make_shared<Button>();
        btn->SetLabel(selected ? "● " + text : "○ " + text);
        btn->WhenAction = handler;
        return Add(btn);
    }
    
    // Show the popup bar at a specific position
    void Popup(const Point& pt) {
        // In a real implementation, this would show the bar as a popup menu
    }
    
    // Show the popup bar relative to a control
    void Popup(Ctrl& ctrl) {
        Rect r = ctrl.GetScreenRect();
        Popup(Point(r.left, r.bottom));
    }
};

// Helper class for building bars
class BarBuilder {
private:
    std::unique_ptr<Bar> bar;
    
public:
    BarBuilder(bool horizontal = true) {
        bar = std::make_unique<Bar>(horizontal);
    }
    
    BarBuilder& Item(String text, std::function<void()> handler) {
        bar->Add(text, handler);
        return *this;
    }
    
    BarBuilder& Separator() {
        bar->AddSeparator();
        return *this;
    }
    
    BarBuilder& Label(String text) {
        bar->AddLabel(text);
        return *this;
    }
    
    Bar& GetBar() { return *bar; }
    
    std::unique_ptr<Bar> Build() {
        return std::move(bar);
    }
};

// Convenience functions
inline BarBuilder MakeBar(bool horizontal = true) {
    return BarBuilder(horizontal);
}

#endif