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

}
