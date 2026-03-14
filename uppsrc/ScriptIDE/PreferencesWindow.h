#ifndef _ScriptIDE_PreferencesWindow_h_
#define _ScriptIDE_PreferencesWindow_h_

class PreferencesWindow : public WithPreferencesLayout<TopWindow> {
public:
    typedef PreferencesWindow CLASSNAME;

    PreferencesWindow(IDEContext& ctx, IDESettings& settings);
    void RefreshPluginPages();

    void AddPage(const String& id, const String& title, Image icon, PreferencesPage* page);
    void AddPage(const String& category, const String& id, const String& title, Image icon, PreferencesPage* page);
    void AddPage(const String& id, const String& title, Image icon, IPluginPreferencesPage* page);
    void AddPage(const String& category, const String& id, const String& title, Image icon, IPluginPreferencesPage* page);

private:
    static String ConfigPath();
    void OnNavSelection();
    void OnOK();
    void OnCancel();
    void OnApply();
    void OnResetDefaults();
    void PopulatePages();
    void ClearPages();
    
    void MarkModified();

    IDEContext& ctx;
    IDESettings& settings;
    IDESettings old_settings;

    struct PageEntry : Moveable<PageEntry> {
        String id;
        String category;
        String title;
        Image icon;
        PreferencesPage* page = nullptr;
        IPluginPreferencesPage* plugin_page = nullptr;
    };
    Array<PageEntry> pages;
};

#endif
