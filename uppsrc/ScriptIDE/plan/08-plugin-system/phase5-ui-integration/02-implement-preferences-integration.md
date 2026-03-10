# Task: Implement Preferences Integration

## Goal
Add a "Plugins" page to the Preferences window to allow users to enable/disable plugins.

## Strategy
1. **New Preference Page**: Create `PluginsPreferencePage`.
2. **UI**: Show a list of discovered plugins with checkboxes for enablement.
3. **Persistence**: Store the enabled/disabled state in `PythonIDESettings`.
4. **Immediate Effect**: Apply changes immediately (or on "Apply") by calling `PluginManager::EnablePlugin` / `DisablePlugin`.

## Success Criteria
- [ ] Plugins list visible in Preferences.
- [ ] Enable/disable state persists across restarts.
- [ ] UI reflects changes (panes appear/disappear) when settings are applied.
