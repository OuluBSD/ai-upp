#ifndef _ScriptIDE_PreferencesPage_h_
#define _ScriptIDE_PreferencesPage_h_

class PythonIDE;

struct IDEContext {
    PythonIDE* main_window = nullptr;
};

class PreferencesPage : public ParentCtrl {
public:
    virtual void Load(const IDESettings& cfg) = 0;
    virtual void Save(IDESettings& cfg) const = 0;
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) = 0;
    virtual void SetDefaults() = 0;
    virtual bool IsModified() const = 0;
};

#endif
