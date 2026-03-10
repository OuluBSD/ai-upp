# Custom Tab Save/Load & Dirty State Behavior

## Dirty State Tracking
Custom documents (e.g., visual layout editors) must notify the IDE when they are modified.
1. **Signal**: The `IDocumentHost` implementation emits a `WhenDirty` event (or equivalent callback).
2. **IDE Update**: `PythonIDE` catches this signal, marks the corresponding `FileInfo` as dirty, and updates the tab title (e.g., prepends `*`).
3. **Check**: The IDE calls `IDocumentHost::IsModified()` during "Save All" or before closing.

## Save Workflow
When the user triggers a "Save" action:
1. `PythonIDE` identifies the active `IDocumentHost`.
2. It calls `host->Save()`.
3. The plugin implementation serializes its internal state (e.g., to JSON) and writes it to the path returned by `host->GetPath()`.
4. Upon success, the plugin implementation clears its internal dirty flag, which triggers an IDE title update.

## Closure Protection
If a user attempts to close a tab or the IDE while `IsModified()` is true:
1. A standard U++ `PromptYesNoCancel` dialog is shown.
2. **Yes**: `Save()` is called. If successful, the tab closes.
3. **No**: The tab closes without saving.
4. **Cancel**: The closure is aborted.

## External Modification
Plugins should ideally monitor their underlying file for external changes. If a change is detected, the `IDocumentHost` can prompt the user to "Reload" or "Keep internal version," leveraging the `Load()` method.
