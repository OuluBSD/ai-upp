# Task: Implement Plugin Manager

## Goal
Implement the central `PluginManager` responsible for the plugin lifecycle.

## Strategy
1. **Lifecycle Management**: Implement `LoadPlugin`, `EnablePlugin`, and `DisablePlugin`.
2. **State Tracking**: Keep track of which plugins are currently enabled.
3. **Integration with IDE**: Provide hooks for `PythonIDE` to query registered file handlers and panes.

## Implementation Details
- `PluginManager` should be a singleton or owned by `PythonIDE`.
- It will own the `PluginRegistry`.

## Success Criteria
- [ ] `PluginManager` can load and enable a mock plugin.
- [ ] `OnEnable` and `OnDisable` are called correctly.
- [ ] Disabling a plugin removes its contributions from the UI.
