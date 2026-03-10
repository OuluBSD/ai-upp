# Task: Plugin Enable/Disable Impact

## Goal
Define the behavior and systemic impact of enabling or disabling a plugin at runtime in ScriptIDE.

## Background / Rationale
Users should be able to toggle plugins via the Preferences menu. Disabling a plugin must cleanly unregister its file handlers, panes, menus, and VM bindings without requiring an IDE restart.

## Scope
- Designing the toggle mechanism in the UI (Preferences page).
- Defining how `PluginManager` revokes file associations.
- Defining how open tabs related to a disabled plugin are handled (e.g., closed or converted to plain text).
- Defining how dockable panes are removed from the Window menu and hidden.
- Defining safe VM unbinding.

## Non-goals
- Live-reloading of modified plugin code (plugins are statically linked C++ for now).

## Dependencies
- `01-plugin-lifecycle-manifest.md`
- ScriptIDE Preferences UI.

## Concrete Investigation Steps
1. Trace the current `PreferencesPages.cpp` to see how settings are applied dynamically.
2. Determine how to safely iterate over open `CustomFileTabs` to find and close plugin-owned documents.
3. Investigate U++ `Docking` API to cleanly remove a registered pane.
4. Determine how to remove functions from the `ByteVM` globals.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginManager.cpp`
- `uppsrc/ScriptIDE/PythonIDE.cpp` (Tab and Pane management)
- `uppsrc/ByteVM/PyVM.cpp`

## Implementation Direction
Provide a clear sequence diagram or step-by-step description of the teardown process when a plugin's `enabled` state changes from true to false.

## Risks
- Dangling pointers in the UI or VM if a plugin is not fully deregistered.
- User data loss if a plugin tab is closed forcefully without prompting to save.

## Acceptance Criteria
- [ ] Documented impact of disabling a plugin on open panes and menus.
- [ ] Documented impact on open file tabs (save prompts, closure).
- [ ] Documented safe VM unbinding strategy.
- [ ] Documented Preferences UI integration plan.
