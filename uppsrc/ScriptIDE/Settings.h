#ifndef _ScriptIDE_Settings_h_
#define _ScriptIDE_Settings_h_

struct PythonIDESettings {
    Font editor_font = Courier(14);
    Font console_font = Courier(14);
    bool show_line_numbers = true;
    bool show_spaces = false;
    String work_directory;

    void Serialize(Stream& s);
};

class SettingsDlg : public WithSettingsLayout<TopWindow> {
public:
    typedef SettingsDlg CLASSNAME;
    SettingsDlg();
    
    void Set(const PythonIDESettings& s);
    void Get(PythonIDESettings& s);
};

#endif
