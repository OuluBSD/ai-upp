#ifndef _ScriptIDE_PreferencesWindow_h_
#define _ScriptIDE_PreferencesWindow_h_

class PythonIDE;

struct IDEContext {
    PythonIDE* main_window = nullptr;
};

class PreferencesWindow : public WithPreferencesLayout<TopWindow> {
public:
    typedef PreferencesWindow CLASSNAME;

    PreferencesWindow(IDEContext& ctx, IDESettings& settings);

    void AddPage(const String& id, const String& title, Image icon, PreferencesPage* page);

private:
    void OnNavSelection();
    void OnOK();
    void OnCancel();
    void OnApply();
    void OnResetDefaults();
    
    void MarkModified();

    IDEContext& ctx;
    IDESettings& settings;
    IDESettings old_settings;

    struct PageEntry : Moveable<PageEntry> {
        String id;
        String title;
        Image icon;
        PreferencesPage* page;
    };
    Array<PageEntry> pages;
};

#endif
