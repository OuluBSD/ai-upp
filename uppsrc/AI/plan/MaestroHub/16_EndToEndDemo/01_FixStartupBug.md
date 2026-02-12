# Task: Fix Standalone Startup Bug
# Status: TODO

## Objective
Ensure `MaestroHub` can start normally without requiring a test script argument. Currently, it fails or behaves incorrectly if no script is provided.

## Context
- `MaestroHub` is being transitioned from a test-harness-only tool to a standalone developer tool.
- The `main.cpp` logic likely enforces a test script argument or fails to initialize the default "Interactive" mode correctly.

## Requirements
- Modify `uppsrc/MaestroHub/main.cpp` (or relevant startup logic).
- Check argument count.
- If no arguments are provided, launch the main `MaestroHub` GUI in interactive mode immediately.
- If arguments are provided (e.g., test script), maintain existing automation behavior.

## Verification
- Run `bin/MaestroHub` (no args) -> Should show main window.
- Run `bin/MaestroHub test_script.py` -> Should execute test.
