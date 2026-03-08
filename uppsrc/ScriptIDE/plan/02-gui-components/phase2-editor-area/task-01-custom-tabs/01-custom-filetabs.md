# Task: Create Custom FileTabs with New/Menu Buttons

## Goal
Extend FileTabs to add "New Tab" button (left) and "Menu" button (right) in tab header.

## Create CustomFileTabs class

File: `uppsrc/ScriptIDE/CustomFileTabs.h`
```cpp
#ifndef _ScriptIDE_CustomFileTabs_h_
#define _ScriptIDE_CustomFileTabs_h_

class CustomFileTabs : public FileTabs {
public:
    typedef CustomFileTabs CLASSNAME;

    CustomFileTabs();

    Event<> WhenNewTab;
    Event<Bar&> WhenTabMenu;

protected:
    virtual void Paint(Draw& w) override;
    virtual void LeftDown(Point p, dword keyflags) override;
    virtual void Layout() override;

private:
    Button new_tab_button;
    Button menu_button;

    Rect GetNewTabRect() const;
    Rect GetMenuButtonRect() const;
};

#endif
```

File: `uppsrc/ScriptIDE/CustomFileTabs.cpp`
```cpp
#include "ScriptIDE.h"

CustomFileTabs::CustomFileTabs()
{
    new_tab_button.SetImage(CtrlImg::Plus());
    new_tab_button.NoWantFocus();
    new_tab_button <<= [=] { WhenNewTab(); };

    menu_button.SetImage(CtrlImg::MenuBars());
    menu_button.NoWantFocus();
    menu_button <<= [=] {
        MenuBar bar;
        WhenTabMenu(bar);
        bar.Execute();
    };

    Add(new_tab_button);
    Add(menu_button);
}

void CustomFileTabs::Layout()
{
    FileTabs::Layout();

    Size sz = GetSize();
    int btn_sz = 20;

    // New tab button at left
    new_tab_button.SetRect(2, 2, btn_sz, btn_sz);

    // Menu button at right
    menu_button.SetRect(sz.cx - btn_sz - 2, 2, btn_sz, btn_sz);
}

Rect CustomFileTabs::GetNewTabRect() const
{
    return Rect(2, 2, 22, 22);
}

Rect CustomFileTabs::GetMenuButtonRect() const
{
    Size sz = GetSize();
    return Rect(sz.cx - 22, 2, sz.cx - 2, 22);
}
```

## Integration in PythonIDE

In `PythonIDE.h`:
```cpp
#include "CustomFileTabs.h"

class PythonIDE : public TopWindow {
    // ...
    CustomFileTabs editor_tabs;

    void OnNewTab();
    void OnTabMenu(Bar& bar);
};
```

In `PythonIDE.cpp`:
```cpp
void PythonIDE::InitLayout()
{
    // ... existing code ...

    editor_tabs.WhenNewTab = THISBACK(OnNewTab);
    editor_tabs.WhenTabMenu = THISBACK(OnTabMenu);

    // Add tabs and label to editor area
    editor_area.Add(editor_tabs.BottomPos(25, 25).HSizePos());
}

void PythonIDE::OnNewTab()
{
    // Create new empty file
    editor_tabs.AddFile("<untitled>", ScriptIDEImg::IconFile());
}

void PythonIDE::OnTabMenu(Bar& bar)
{
    bar.Add("Close All", [=] { /* ... */ });
    bar.Add("Close Others", [=] { /* ... */ });
    bar.Separator();
    bar.Add("Tabs at Bottom", [=] {
        // Toggle tab position
        editor_tabs.SetAlign(editor_tabs.GetAlign() == AlignedFrame::BOTTOM ?
                             AlignedFrame::TOP : AlignedFrame::BOTTOM);
    });
}
```

## Files Created
- `uppsrc/ScriptIDE/CustomFileTabs.h`
- `uppsrc/ScriptIDE/CustomFileTabs.cpp`

## Files Modified
- `uppsrc/ScriptIDE/ScriptIDE.h` (add include)
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`
- `uppsrc/ScriptIDE/ScriptIDE.upp` (add CustomFileTabs files)

## Success Criteria
- "+" button visible at left of tabs
- "≡" menu button visible at right
- Clicking "+" creates new tab
- Menu button shows context menu
- Tabs can be placed at top or bottom
