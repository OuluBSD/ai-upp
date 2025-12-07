# AI-UPP CLI Module

This module implements the command metadata layer and CLI scaffold for AI-UPP, providing a structured way to interact with TheIDE functionality through command-line interface.

## Architecture Overview

The CLI module consists of several components:

1. **Command Metadata (`../metadata/commands_v1.json`)**: Defines the available commands and their parameters in a machine-readable JSON format.

2. **Command Representation (`Command.h`)**: C++ structures that represent commands, arguments, and invocation results.

3. **JSON Loader (`JsonLoader.h/cpp`)**: Loads and parses the command metadata from the JSON file.

4. **Command Registry (`CommandRegistry.h/cpp`)**: Manages the collection of commands and provides lookup functionality.

5. **IDE Host (`IdeHost.h/cpp`)**: Hosts a real `Ide` instance from TheIDE in a headless/minimal-GUI configuration, allowing CLI access to full IDE functionality.

6. **IDE Session (`IdeSession.h/cpp`)**: An abstraction layer that decouples command execution from direct IDE internals, serving as a thin adapter around the real `Ide` instance.

7. **Command Executor (`CommandExecutor.h/cpp`)**: Validates arguments and dispatches commands through the `IdeSession` interface.

8. **CLI Frontend (`main.cpp`)**: Parses command-line arguments and executes commands after initializing the `IdeHost`.

## IDE Session Abstraction

The `IdeSession` interface provides a clean separation between the command execution layer and TheIDE internals. This allows for:

- Testing without full IDE initialization
- Headless operation for build automation
- Potential remote IDE control
- Easier maintenance and refactoring

## IDE Host Architecture

The `IdeHost` component creates and initializes a real `Ide` instance following patterns from TheIDE's `main.cpp`, but without starting the GUI event loop. It enables CLI programs to access full TheIDE functionality while running in a headless mode.

## Command Integration Status

All commands are now backed by real TheIDE functionality through the `IdeSession`:

- ✅ `open_file`: Opens files through the real `Ide::EditFile` method
- ✅ `save_file`: Saves files through the real `Ide::SaveFile` method
- ✅ `set_main_package`: Sets main package through `Ide::SetMain` method
- ✅ `build_project`: Performs actual builds using `Ide::PackageBuild` with real logs and error capture
- ✅ `clean_project`: Performs actual cleaning using `Ide::PackageClean`
- ✅ `goto_line`: Performs actual navigation using `Ide::GotoPos`
- ✅ `find_in_files`: Performs actual file searches using custom headless implementation
- ✅ `search_code`: Initiates code search/indexing through `Ide::TriggerIndexer`
- ✅ `show_console`: Retrieves actual console output from TheIDE console system
- ✅ `show_errors`: Retrieves actual error list from TheIDE error tracking system

## Building the CLI

To build the `theide-cli` binary, use the Ultimate++ build system:

```
// Build using TheIDE or command line build tools
```

## Using the CLI

Basic usage pattern:

```bash
theide-cli <command_name> [--arg value ...]
```

### Examples:

```bash
# List all available commands
theide-cli list_commands

# Open a file
theide-cli open_file --path /path/to/file.cpp

# Build a project
theide-cli build_project --name MyApp --config Debug

# Set main package
theide-cli set_main_package --package MyApp

# Search for text in files
theide-cli find_in_files --pattern "TODO" --directory /src

# Search code across the project
theide-cli search_code --query "MyFunction"

# Show console output
theide-cli show_console

# Show errors
theide-cli show_errors
```

## Current Status

This version fully integrates all 10 commands with real TheIDE functionality through the `IdeHost` and `IdeSession` architecture. The CLI now runs a real `Ide` instance in headless mode, enabling access to all of TheIDE's core capabilities from the command line.

## Available Commands

- `open_file`: Open a file in the editor (FULLY INTEGRATED)
- `save_file`: Save the current or specified file (FULLY INTEGRATED)
- `find_in_files`: Search for patterns in files (FULLY INTEGRATED)
- `build_project`: Build a project with specified configuration (FULLY INTEGRATED)
- `clean_project`: Clean project build artifacts (FULLY INTEGRATED)
- `goto_line`: Navigate to a specific line in a file (FULLY INTEGRATED)
- `search_code`: Search code across the project (FULLY INTEGRATED)
- `show_console`: Retrieve console output (FULLY INTEGRATED)
- `show_errors`: Retrieve error list (FULLY INTEGRATED)
- `set_main_package`: Set the main package for the project (FULLY INTEGRATED)
- `list_commands`: List all available commands (special built-in command)

## Implementation Notes

- All handlers now connect to real TheIDE through the `IdeSession` interface
- The `IdeHost` properly initializes TheIDE in headless mode before command execution
- Error handling captures real IDE errors and exceptions
- Build operations return actual build logs and error information
- The architecture maintains clean separation while providing access to full IDE functionality