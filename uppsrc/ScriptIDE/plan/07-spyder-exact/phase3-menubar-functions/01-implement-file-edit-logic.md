# Task: Implement File and Edit Menu Logic

## Goal
Replace the menubar stubs for **File** and **Edit** categories with real functional logic, ensuring 100% connectivity to the IDE's core services.

## Strategy
1.  **File Logic**: Implement multi-file management (tab-aware), saving, loading, and recently-used files.
2.  **Edit Logic**: Connect to `CodeEditor` methods for standard text operations and implement Python-specific logic (commenting, casing).
3.  **Connectivity**: Link to `IDESettings` where applicable (e.g., Tab width, EOL mode).

## Implementation Details

### File Menu
- `New file`: Create a new tab with an "untitled" buffer.
- `Open`: Use `FileSel` to load a `.py` file into a new or existing tab.
- `Save/Save As`: Persist current editor content.
- `Open Recent`: Implement a `Vector<String>` in `settings.application` to track history.
- `Restart`: Re-initialize the IDE process (or clear all state).

### Edit Menu
- `Undo/Redo`: Direct map to `code_editor.Undo() / Redo()`.
- `Comment/Uncomment`: Use `code_editor.ToggleLineComments()`.
- `Toggle UPPERCASE/lowercase`: Implement selection transformation.
- `Convert EOL`: Implement logic to transform `\n` to `\r\n` or `\r`.

## Success Criteria
- [ ] User can open multiple files in tabs.
- [ ] Recent files list persists across sessions.
- [ ] Comment/Uncomment works via Ctrl+1.
- [ ] UPPER/lower case toggles correctly transform selected text.
- [ ] Save All correctly prompts for all modified buffers.
