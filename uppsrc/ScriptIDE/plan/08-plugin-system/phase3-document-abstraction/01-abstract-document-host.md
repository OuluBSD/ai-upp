# Task: Abstract Document Host

## Goal
Modify the editor area to host arbitrary `Ctrl`s instead of just `CodeEditor`.

## Background
Currently, the `PythonIDE` assumes all documents are Python files edited in a `CodeEditor`. To support plugins, we need an abstraction that can host either a `CodeEditor` or a plugin-provided `Ctrl`.

## Strategy
1. **Define IDocumentHost**: An interface or base class for anything that can be shown in the editor area tabs.
2. **Refactor Editor Area**:
   - Change `active_editor` to be a pointer/reference to `IDocumentHost`.
   - Ensure `editor_tabs` can associate a key with an `IDocumentHost` instance.
3. **Implement Base Hosts**:
   - `SourceDocumentHost`: Wraps the existing `CodeEditor`.
   - `PluginDocumentHost`: A generic container for plugin-provided `Ctrl`s.

## Success Criteria
- [ ] Editor area can display a non-text `Ctrl` (e.g., a simple `Label` as a test).
- [ ] Switching tabs correctly switches the hosted `Ctrl`.
- [ ] Standard editor operations (Save, Cut, Copy) are delegated to the host.
