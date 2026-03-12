#include "ScriptIDE.h"

namespace Upp {

CustomFileTabs::CustomFileTabs()
{
    new_tab_button.SetImage(CtrlImg::Plus());
    new_tab_button.NoWantFocus();
    new_tab_button << [=] { WhenNewTab(); };

    menu_button.SetImage(CtrlImg::menu_window());
    menu_button.NoWantFocus();
    menu_button << [=] {
        MenuBar bar;
        WhenTabMenu(bar);
        bar.Execute();
    };

    this->Ctrl::Add(new_tab_button);
    this->Ctrl::Add(menu_button);
}

void CustomFileTabs::Layout()
{
    FileTabs::Layout();

    Size sz = GetSize();
    int btn_sz = 24;
    int y_off = (sz.cy - btn_sz) / 2;

    // New tab button at left
    new_tab_button.SetRect(2, y_off, btn_sz, btn_sz);

    // Menu button at right
    menu_button.SetRect(sz.cx - btn_sz - 2, y_off, btn_sz, btn_sz);
}

Rect CustomFileTabs::GetNewTabRect() const
{
    Size sz = GetSize();
    int btn_sz = 24;
    int y_off = (sz.cy - btn_sz) / 2;
    return Rect(2, y_off, btn_sz + 2, y_off + btn_sz);
}

Rect CustomFileTabs::GetMenuButtonRect() const
{
    Size sz = GetSize();
    int btn_sz = 24;
    int y_off = (sz.cy - btn_sz) / 2;
    return Rect(sz.cx - btn_sz - 2, y_off, sz.cx - 2, y_off + btn_sz);
}

}
