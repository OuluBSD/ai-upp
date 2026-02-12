# Phase 1: Project Bootstrap
# Status: TODO

## Scenarios 1-15

1.  **Hub Startup**: Verify MaestroHub opens and displays the splash/welcome.
2.  **Layout Verification**: Check for all core frames (Menu, Toolbar, Status).
3.  **Project Init (AI Driven)**: Open 'InitDialog', target `./tmp/SmallGuiAppForAccounting`.
    - **Input**: Empty dir.
    - **Action**: Click 'Init'.
    - **Verify**: "Prelaunch: Scaffolding Project" task appears in AI Trace.
    - **Mock AI**: Return file content for `main.cpp` and `.upp`.
    - **Output**: Physical files created by AI logic.
4.  **Session Creation**: Create "Sprint-0-Bootstrap" session.
    - **Output**: `session.json`.
5.  **Root Directory Verification**: Verify the file browser shows the accounting app root.
6.  **Package Discovery**: Verify 'FleetDashboard' detects the new project.
7.  **Maintenance Setup**: Run 'Sync Core' to ensure U++ headers are available.
8.  **Assistant Toggle**: Toggle the assistant to prepare for AI guidance.
9.  **Navigation History**: Click between 'Dashboard' and 'Sessions' to verify history works.
10. **Build Method Selection**: Open 'BuildMethods' and select a valid local toolchain.
    - **Input**: System compilers.
    - **Output**: `build_method.json`.
11. **Toolchain Var Config**: Set `UPP_MAIN` to the new project.
12. **Breadcrumb Check**: Verify breadcrumbs show `SmallGuiAppForAccounting`.
13. **Tab Focus Persistence**: Verify that clicking 'Intelligence' tab stays focused.
14. **Status Bar Health**: Check that 'Quota Indicator' is initialized.
15. **Initial Save**: Trigger a global save and verify `.maestro/maestro.json`.

## Summary
- **Input**: None (Bootstrap).
- **Output**: A valid, configured U++ project session ready for planning.
