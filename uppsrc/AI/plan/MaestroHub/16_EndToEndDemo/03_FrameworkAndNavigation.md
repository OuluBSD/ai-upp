# Scenarios 1-10: Framework & Navigation
# Status: TODO

## Objective
Verify the core UI framework stability, global navigation, and visibility of primary components.

## Scenarios
1.  **Startup Splash**: Verify MaestroHub starts and displays the 'WelcomeDialog'.
2.  **Global Layout**: Verify presence of MenuBar, ToolBar, StatusBar, and the three main tab areas (Left, Center, Bottom).
3.  **Menu Traversal**: Automate a full sweep of the 'App' and 'Main' menus, ensuring no crashes on click.
4.  **Tab Switching (Mouse)**: Click through every tab in 'CenterTabs' (Dashboard, Intelligence, Playbook, etc.) and verify focus.
5.  **Keyboard Shortcuts (Ctrl+Tab)**: Verify cycling through tabs via standard keyboard shortcuts.
6.  **Assistant Toggle (F11)**: Toggle the 'MaestroAssistant' sidebar and verify that the 'CenterPane' resizes correctly.
7.  **History Navigation (Alt+Left/Right)**: Visit three different tabs, use 'Back' and 'Next' buttons/shortcuts to verify navigation history.
8.  **Status Bar Metrics**: Verify that the 'Quota Indicator' (AI usage) is visible and updates after a mock AI call.
9.  **Assistant Context Sync**: Change the active task in the Plan tree and verify the Assistant's header updates its context (Track/Phase/Task).
10. **Clean Shutdown**: Trigger 'Exit' and verify the process terminates gracefully without memory corruption logs.

## Verification Method
- Use `dump_ui` to find `LayoutId("Main")`, `MenuBar`, and `TabCtrl` slaves.
- Use `send_key` for shortcuts.
- Inspect `MaestroQA.log` for any `LogInternal` errors.
