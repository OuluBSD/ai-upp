# Task: Create Reference Example Plugin

## Goal
Implement a fully functional reference plugin to validate the plugin system.

## Location
- Source: `reference/ScriptIdePlugin`

## Requirements
The reference plugin must demonstrate:
1. **Custom File Type**: Handle `.gamestate` files.
2. **Custom View**: Provide a `GameStateCtrl` (e.g., showing a simple 2D map or state list).
3. **VM Bindings**: Register `MoveUnit(id, x, y)` and `GetScore()` functions.
4. **Custom Execute**: Pressing F5 on a `.gamestate` file runs `scripts/init_game.py`.
5. **Custom Pane**: Provide a "Game Stats" dockable pane.
6. **Integration**: Show how to register the plugin via the system.

## Success Criteria
- [ ] Reference plugin builds successfully.
- [ ] Plugin is discoverable by ScriptIDE.
- [ ] All 6 requirements verified in the IDE.
