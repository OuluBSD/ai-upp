# Task: Implement VM Binding Bridge

## Goal
Allow plugins to register C++ functions that can be called from Python code running in ByteVM.

## Strategy
1. **Define Binding Context**: A plugin should receive a reference to the `PyVM` instance.
2. **Registration API**: Provide a way for plugins to add entries to the global or local dictionary of the VM.
3. **Bridge Logic**:
   - Plugins implement `IPythonBindingProvider`.
   - When a document is opened or a plugin is enabled, the bridge calls the provider to register functions.

## Implementation Details
- Use `PyValue::Function` wrappers to expose C++ callbacks.
- Ensure thread safety and lifetime management of bound objects.

## Success Criteria
- [ ] Python script can call a C++ function registered by a plugin.
- [ ] Functions can accept and return `PyValue` objects.
- [ ] Bindings are cleaned up when the plugin is disabled.
