# Scenarios 11-20: Session & Workspace Management
# Status: TODO

## Objective
Verify project initialization, directory switching, and session persistence.

## Scenarios
1.  **Project Init (Maestro)**: Open 'InitDialog', select `./tmp/SmallGuiAppForAccounting`, and click 'Init'.
2.  **Directory Browsing**: In the 'Sessions' pane, verify the directory tree correctly lists the newly created project.
3.  **New Work Session**: Click 'New Session' in the toolbar, enter "Feature-Auth", and verify it appears in the 'WorkSessions' list.
4.  **Session Persistence**: Close MaestroHub, restart, and verify the "Feature-Auth" session is still selected and the root directory is preserved.
5.  **Breadcrumb Navigation**: Deep dive into subdirectories in the file browser and use breadcrumbs to jump back to root.
6.  **Filter - Status**: Apply "In Progress" filter to sessions and verify only active sessions are visible.
7.  **Filter - Search**: Search for "Auth" in the session list and verify substring matching.
8.  **Context View**: Select a session and verify the 'ContextView' (RichText) displays the session's metadata and last activity.
9.  **Session Deletion**: Create a temporary session "Trash", delete it via context menu, and verify it's removed from disk/DB.
10. **Multiple Roots**: Add a second directory to the 'Dirs' list and verify switching between two disparate project roots.

## Verification Method
- Semantic check on `TreeCtrl` labels.
- Verify file existence in `./tmp/` after 'Init'.
- Inspect `docs/maestro/sessions.json` for persistence validation.
