# Task: Command Dispatcher
# Status: DONE

## Objective
Implement a hierarchical command dispatcher using and improving the `CommandLineArguments` class from `Core/Util.h`.

## Requirements
- Update `CommandLineArguments` to support subcommands efficiently.
- Add `bool Parse(const Vector<String>& arguments)` to `CommandLineArguments` to allow parsing partial or offset argument vectors.
- Implement a hierarchical dispatcher where each level uses its own `CommandLineArguments` instance.
- Support nested subcommands (e.g., `maestro plan list`).
- Ensure help menus are specific to the current command/subcommand level.
- Refactor `main.cpp` to use this structured approach.
