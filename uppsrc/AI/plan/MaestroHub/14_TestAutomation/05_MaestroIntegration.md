# Task: Maestro Integration & Test Runner

# Status: DONE

# Description
Integrate the Python automation capabilities into `Maestro` and create a command to execute tests.

# Objectives
- [x] Add `Ctrl/Automation` and `ByteVM` dependencies to `Maestro` and `MaestroHub`.
    - `Maestro` package now depends on `ByteVM` always, and `Ctrl/Automation` when `flagGUI` is active.
- [x] Implement `maestro test` command:
    - Loads a Python script.
    - Registers UI automation bindings (`find`, `click`, `set`, etc.).
    - Runs the script using `PyVM`.
    - Reports success or caught exceptions.
- [x] Ensure command availability:
    - `maestro test` is available in `MaestroHub` and `MaestroCLI` (if built with GUI support).

# Implementation Details
- `TestCommand` handles the script loading and VM execution.
- Undefined references to `Tokenizer` were resolved by adding `Core/TextParsing` dependency to `ByteVM`.
- Headless build compatibility maintained by wrapping GUI-specific logic in `flagGUI`.
