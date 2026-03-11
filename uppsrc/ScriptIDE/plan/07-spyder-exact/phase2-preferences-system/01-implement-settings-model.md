# Task: Implement IDESettings Data Model

## Migration Note (ScriptCommon Split)
`IDESettings` schema ownership is moving to `uppsrc/ScriptCommon/IDESettings.h`. ScriptIDE keeps page widgets and apply/adaptation logic.

## Goal
Implement the master configuration object `IDESettings` as specified in `PREFERENCES_SPEC.md`. This object will serve as the single source of truth for the IDE's state.

## Strategy
1.  **Define Sub-structs**: Create typed structs for each of the 18 sections (Appearance, Application, Python interpreter, etc.).
2.  **Global IDESettings**: Aggregate all sub-structs into a master `IDESettings` class.
3.  **Persistence**: Implement `Serialize` methods for all structs to support loading/storing to disk.

## Implementation Details

### IDESettings.h
```cpp
struct AppearanceSettings {
    String interface_theme;
    String syntax_theme;
    String monospace_font_face;
    int    monospace_font_size = 10;
    // ...
    void Serialize(Stream& s);
};

// ... 17 more structs ...

struct IDESettings {
    AppearanceSettings appearance;
    ApplicationSettings application;
    // ...
    void Serialize(Stream& s);
};
```

## Success Criteria
- [ ] `IDESettings` contains all 18 sections from the spec.
- [ ] Default values are assigned according to the spec.
- [ ] `Serialize` handles all fields.
- [ ] Settings can be saved to and loaded from `ide_settings.bin`.
