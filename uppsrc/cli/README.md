# AI-UPP CLI Module

This module implements the command metadata layer and CLI scaffold for AI-UPP, providing a structured way to interact with IDE functionality through command-line interface.

## Architecture Overview

The CLI module consists of several components:

1. **Command Metadata (`../metadata/commands_v1.json`)**: Defines the available commands and their parameters in a machine-readable JSON format.

2. **Command Representation (`Command.h`)**: C++ structures that represent commands, arguments, and invocation results.

3. **JSON Loader (`JsonLoader.h/cpp`)**: Loads and parses the command metadata from the JSON file.

4. **Command Registry (`CommandRegistry.h/cpp`)**: Manages the collection of commands and provides lookup functionality.

5. **CLI Core (`clicore` package)**: A new NOGUI package that implements IDE functionality without any GUI dependencies. Contains core logic for workspace management, building, searching, etc.

6. **IDE Session (`IdeSession.h/cpp`)**: An abstraction layer that decouples command execution from IDE internals, serving as a thin adapter around the `CoreIde` from the CLI core.

7. **Command Executor (`CommandExecutor.h/cpp`)**: Validates arguments and dispatches commands through the `IdeSession` interface.

8. **CLI Frontend (`main.cpp`)**: Parses command-line arguments and executes commands after initializing the `IdeSession`.

## CLI Core Architecture (New Design)

The new architecture introduces the `clicore` package to address the critical issue that the original `ide` package is tightly coupled to GUI (GTK/Win32/Cocoa) and requires GUI build flags. The `clicore` package:

- **NOGUI**: Built without any GUI dependencies
- **Forked Logic**: Contains relevant non-GUI logic from `uppsrc/ide` copied and sanitized
- **Clean API**: Provides a pure C++ API that matches what `IdeSession` needs
- **Incremental Migration**: Structured so that more functionality from `uppsrc/ide` can be ported into it incrementally

## Architecture Flow

```
CLI → CommandExecutor → IdeSession → CoreIde → internal CLI-only services
```

**Why this approach?** Previous attempts to host a real `Ide` instance from the `ide` package failed because:
- The `ide` package is tightly coupled to GUI and requires GUI build flags everywhere
- Including `<ide/ide.h>` in a NOGUI CLI package pulls in GUI backends and breaks the build
- The `IdeHost` approach that tried to run a headless `Ide` still required GUI libraries at build time

**Solution**: We FORK relevant non-GUI logic from `uppsrc/ide` into our own `clicore` package, gradually de-GUI-fying it while preserving functionality.

## Command Integration Status

All commands now work through the CLI Core (clicore) without any GUI dependencies:

- ✅ `open_file`: File opening through `CoreFileOps` in CLI Core
- ✅ `save_file`: File saving through `CoreFileOps` in CLI Core
- ✅ `set_main_package`: Package management through `CoreWorkspace` in CLI Core
- ✅ `build_project`: Build operations through `CoreBuild` in CLI Core (simulated for now)
- ✅ `clean_project`: Clean operations through `CoreBuild` in CLI Core (simulated for now)
- ✅ `goto_line`: Line navigation validation (to be enhanced in future)
- ✅ `find_in_files`: File searches through `CoreSearch` in CLI Core
- ✅ `search_code`: Code search (stubbed, to be enhanced in future)
- ✅ `show_console`: Console output through `CoreConsole` in CLI Core
- ✅ `show_errors`: Error output through `CoreConsole` in CLI Core
- ✅ `list_commands`: Special built-in command (unchanged)

## Building the CLI

The CLI can now be built in NOGUI mode without any GUI dependencies:

```
// Build using TheIDE or command line build tools in NOGUI configuration
```

## Using the CLI

Basic usage pattern:

```bash
theide-cli <command_name> [--arg value ...]
```

### Global Arguments

The CLI supports global arguments that affect the entire session:

- `--workspace-root PATH`: Sets the workspace root directory for the session (required for package-aware commands)

### Examples:

```bash
# List all available commands
theide-cli list_commands

# Open a file
theide-cli open_file --path /path/to/file.cpp

# Build a project with workspace root
theide-cli --workspace-root /path/to/workspace build_project --name MyApp --config Debug

# Set main package in a workspace
theide-cli --workspace-root . set_main_package --package MyApp

# Search for text in files (uses main package directory if no directory specified)
theide-cli --workspace-root . find_in_files --pattern "TODO"

# Search code across the main package
theide-cli --workspace-root . search_code --query "MyFunction"

# Show console output
theide-cli show_console

# Show errors
theide-cli show_errors
```

## Current Status

This version implements a clean NOGUI architecture where all 10 commands connect to the `clicore` package instead of the GUI-dependent `ide` package. The CLI now builds cleanly in NOGUI mode and provides core IDE functionality through a forked core.

## Available Commands

- `open_file`: Open a file in the editor (FULLY INTEGRATED with CLI Core)
- `save_file`: Save the current or specified file (FULLY INTEGRATED with CLI Core)
- `find_in_files`: Search for patterns in files (FULLY INTEGRATED with CLI Core)
- `build_project`: Build a project with specified configuration (SIMULATED with CLI Core)
- `clean_project`: Clean project build artifacts (SIMULATED with CLI Core)
- `goto_line`: Navigate to a specific line in a file (PARTIALLY INTEGRATED)
- `search_code`: Search code across the project (STUBBED in CLI Core)
- `show_console`: Retrieve console output (FULLY INTEGRATED with CLI Core)
- `show_errors`: Retrieve error list (FULLY INTEGRATED with CLI Core)
- `set_main_package`: Set the main package for the project (FULLY INTEGRATED with CLI Core)
- `list_commands`: List all available commands (special built-in command)

## Implementation Notes

- All handlers now connect to the CLI Core (`clicore`) through the `IdeSession` interface
- The `IdeSession` has been refactored to use `CoreIde` instead of `Ide`/`IdeHost`
- NO GUI dependencies: The `ide` package is NO LONGER used directly
- The `clicore` package is built NOGUI and can be used in headless environments
- Build operations are currently simulated but provide the structure for integration with real build system
- The architecture maintains clean separation while providing a path for more TheIDE functionality to be ported