# Task: Implement Plugin Docking

## Goal
Allow plugins to contribute custom dockable panes to the IDE.

## Strategy
1. **Registry Support**: Store mappings from plugin names to `IDockPaneProvider`.
2. **Auto-Opening**:
   - When a plugin is enabled, if it provides a pane, create and dock it automatically.
   - Pains should participate in the existing `Docking` system.
3. **Menu Integration**: Add plugin panes to the "Window > Panes" menu.

## Success Criteria
- [ ] Plugin can show a custom `DockableCtrl`.
- [ ] Pane is restored correctly on application restart (layout serialization).
- [ ] Pane is hidden/removed when the plugin is disabled.
