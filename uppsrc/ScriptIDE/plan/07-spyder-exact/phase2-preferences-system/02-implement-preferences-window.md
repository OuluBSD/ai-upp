# Task: Implement Preferences Window

## Goal
Implement the `PreferencesWindow` dialog with its navigation tree and dynamic page loading, as specified in `PREFERENCES_SPEC.md`.

## Strategy
1.  **Layout**: Create a `TopWindow` with a horizontal `Splitter`.
    -   Left side: `ArrayCtrl` for navigation (Icon + Title).
    -   Right side: `ParentCtrl` to host the current settings page.
2.  **Navigation Logic**: Clicking an item in the `ArrayCtrl` must hide the current page and show the selected page.
3.  **Bottom Actions**: Implement Reset to defaults, OK, Cancel, and Apply buttons.

## Implementation Details

### PreferencesWindow.h
```cpp
class PreferencesWindow : public TopWindow {
public:
    typedef PreferencesWindow CLASSNAME;

    Splitter main_split;
    ArrayCtrl nav;
    ParentCtrl page_host;
    // ... buttons ...

    void AddPage(const String& id, const String& title, Image icon, PreferencesPage& page);
    void OnNavSelection();
};
```

## Success Criteria
- [ ] Window opens with a split view.
- [ ] Navigation tree contains all 18 sections.
- [ ] Selecting a section correctly switches the visible page on the right.
- [ ] Bottom buttons (OK/Apply/Cancel) are functional.
