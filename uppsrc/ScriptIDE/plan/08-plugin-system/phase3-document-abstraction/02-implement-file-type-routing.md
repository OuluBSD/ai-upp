# Task: Implement File Type Routing

## Goal
Route file opening requests to the appropriate plugin based on file extension.

## Strategy
1. **Extension Mapping**: Update `PluginRegistry` to store mappings from extensions (e.g., `.gamestate`) to `IDocumentViewFactory`.
2. **Opening Logic**:
   - When a file is opened, check its extension.
   - If a plugin is registered for that extension, call its factory to create the `Ctrl`.
   - Fall back to `CodeEditor` for `.py` or unknown types.

## Success Criteria
- [ ] Opening a `.py` file still uses `CodeEditor`.
- [ ] Opening a registered extension (e.g., `.gamestate`) creates the plugin's `Ctrl`.
- [ ] Error handling for failed factory calls.
