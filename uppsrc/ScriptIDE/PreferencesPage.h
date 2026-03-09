#ifndef _ScriptIDE_PreferencesPage_h_
#define _ScriptIDE_PreferencesPage_h_

struct IDEContext; // Forward declaration

class PreferencesPage : public ParentCtrl {
public:
    virtual void Load(const IDESettings& cfg) = 0;
    virtual void Save(IDESettings& cfg) const = 0;
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) = 0;
    virtual void SetDefaults() = 0;
    virtual bool IsModified() const = 0;
};

#endif
