# ScriptIDE Planning Conventions Summary

## Document Structure
All planning tasks must follow this header hierarchy:
1. # Task: [Title]
2. ## Goal
3. ## Background / Rationale
4. ## Scope
5. ## Non-goals
6. ## Dependencies
7. ## Concrete Investigation Steps
8. ## Affected Subsystems
9. ## Implementation Direction
10. ## Risks
11. ## Acceptance Criteria (with checkboxes)

## File Organization
- Tracks are numbered (e.g., `01-foundation`).
- Phases are subdirectories (e.g., `phase1-architecture`).
- Tasks are individual `.md` files inside phases.
- Results/Artifacts should be co-located or linked in `cookie.txt`.

## Detail Level
- Strategy must explain *how* the implementation will be approached.
- Implementation Direction should include code sketches or class diagrams where appropriate.
- Dependencies must link to other task files.

## Terminology
- **ByteVM**: Internal Python interpreter.
- **DockWindow**: Main UI container for panes.
- **DockableCtrl**: Base for side panes.
- **CustomFileTabs**: The tabbed area for documents.
