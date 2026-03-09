# Task: Implement Preferences Page Stubs

## Goal
Implement the 18 specialized `PreferencesPage` classes as specified in `PREFERENCES_SPEC.md`.

## Strategy
1.  **Base Class**: Define `PreferencesPage` inheriting from `ParentCtrl` with virtual methods `Load`, `Save`, `Apply`, `SetDefaults`, and `IsModified`.
2.  **Page Implementation**: Create one class for each section (e.g., `AppearancePage`, `EditorPage`).
3.  **UI Content**: For this task, populate each page with the primary controls (checkboxes, dropdowns) mentioned in the spec.

## Implementation Details

### PreferencesPage.h
```cpp
class PreferencesPage : public ParentCtrl {
public:
    virtual void Load(const IDESettings& cfg) = 0;
    virtual void Save(IDESettings& cfg) const = 0;
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) = 0;
    virtual void SetDefaults() = 0;
    virtual bool IsModified() const = 0;
};
```

## Success Criteria
- [ ] 18 classes are implemented.
- [ ] Each page has a basic layout with at least the most critical controls.
- [ ] `Load` and `Save` methods are correctly wired to their respective sections in `IDESettings`.
