# Task: Implement Menubar Skeleton (Stubs)

## Goal
Implement the 100% complete menu bar structure as specified in `MENUBAR_SPEC.md`. Every menu, submenu, separator, and shortcut must be present.

## Strategy
1.  **Define Stub Methods**: Add placeholder methods to `PythonIDE.h` for every category (e.g., `StubFile`, `StubEdit`).
2.  **Exhaustive Menu Building**: Use `MENUBAR_SPEC.md` as a checklist to build out the `MainMenu` and its sub-menus.
3.  **Shortcut Registration**: Assign the exact shortcuts defined in the spec.

## Implementation Details

### PythonIDE.h
Add a generic `Todo(const String& msg)` method to show a "Not implemented" message.

### PythonIDE.cpp - MainMenu
```cpp
void PythonIDE::MainMenu(Bar& bar) {
    bar.Sub("File",      [=](Bar& b){ FileMenu(b); });
    bar.Sub("Edit",      [=](Bar& b){ EditMenu(b); });
    bar.Sub("Search",    [=](Bar& b){ SearchMenu(b); });
    bar.Sub("Source",    [=](Bar& b){ SourceMenu(b); });
    bar.Sub("Run",       [=](Bar& b){ RunMenu(b); });
    bar.Sub("Debug",     [=](Bar& b){ DebugMenu(b); });
    bar.Sub("Consoles",  [=](Bar& b){ ConsolesMenu(b); });
    bar.Sub("Projects",  [=](Bar& b){ ProjectsMenu(b); });
    bar.Sub("Tools",     [=](Bar& b){ ToolsMenu(b); });
    bar.Sub("Window",    [=](Bar& b){ WindowMenu(b); });
    bar.Sub("Help",      [=](Bar& b){ HelpMenu(b); });
}
```

## Success Criteria
- [ ] All top-level menus are visible.
- [ ] Sub-menus (like "Open recent", "Convert end-of-line") are present.
- [ ] Separators match the specification exactly.
- [ ] Shortcuts (Ctrl+N, Ctrl+O, etc.) are visible in the menu items.
- [ ] Clicking any item that is not yet implemented shows a "Todo" message.
