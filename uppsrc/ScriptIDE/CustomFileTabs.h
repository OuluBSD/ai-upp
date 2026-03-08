#ifndef _ScriptIDE_CustomFileTabs_h_
#define _ScriptIDE_CustomFileTabs_h_

class CustomFileTabs : public FileTabs {
public:
    typedef CustomFileTabs CLASSNAME;

    CustomFileTabs();

    Event<> WhenNewTab;
    Event<Bar&> WhenTabMenu;

protected:
    virtual void Layout() override;

private:
    Button new_tab_button;
    Button menu_button;

    Rect GetNewTabRect() const;
    Rect GetMenuButtonRect() const;
};

#endif
