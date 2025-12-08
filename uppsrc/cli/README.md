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
- `--json`: When present, all output is formatted as JSON objects (machine-friendly)
- `--script PATH`: Execute commands from a text script file
- `--json-script PATH`: Execute commands from a JSON script file (AI Driver mode)

## Machine-Readable Output (JSON Mode)

The CLI now supports a machine-friendly JSON output mode. When the `--json` flag is provided, all commands return structured JSON instead of human-readable text.

### Usage:
```bash
theide-cli --json find_definition --symbol MyClass
```

### Output Format:
```json
{
  "command": "find_definition",
  "status_code": 0,
  "message": "Definition found: MyClass.cpp:15",
  "payload": {
    "file": "MyClass.cpp",
    "line": 15,
    "symbol": "MyClass"
  }
}
```

The payload field contains structured data appropriate for each command, while status_code (0=success) and message provide human-readable summary information.

## Command Introspection (describe_command)

An AI process can query the command surface programmatically using the `describe_command` command:

### Usage:
```bash
theide-cli --json describe_command --name build_project
```

### Output:
```json
{
  "command": "describe_command",
  "status_code": 0,
  "message": "Command: build_project\nCategory: build\nDescription: Build a project with specified configuration...",
  "payload": {
    "name": "build_project",
    "category": "build",
    "description": "Build a project with specified configuration...",
    "inputs": [
      {
        "name": "name",
        "type": "string",
        "required": true,
        "description": "Project name to build",
        "default": ""
      },
      {
        "name": "config",
        "type": "string",
        "required": false,
        "description": "Build configuration (Debug/Release)",
        "default": "Debug"
      }
    ],
    "outputs": {
      "kind": "object",
      "fields": ["project", "config", "log", "success"]
    },
    "side_effects": {
      "modifies_files": false,
      "modifies_project": false,
      "requires_open_project": false,
      "requires_open_file": false
    },
    "context_notes": ""
  }
}
```

## JSON Script Mode (AI Driver)

The CLI supports JSON-based script execution designed for AI automation systems:

### JSON Script Format:
```json
{
  "commands": [
    {
      "name": "set_main_package",
      "args": { "package": "MyApp" }
    },
    {
      "name": "find_definition",
      "args": { "symbol": "MyClass" }
    },
    {
      "name": "build_project",
      "args": { "name": "MyApp", "config": "Debug" }
    }
  ],
  "stop_on_error": true
}
```

### Usage:
```bash
theide-cli --workspace-root . --json --json-script plan.json
```

Each command in the JSON array is executed sequentially. If `stop_on_error` is true, execution stops on the first error.

## Examples:

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

# Machine-readable output for symbol definition
theide-cli --workspace-root . --json find_definition --symbol MyClass

# Get build order in structured format
theide-cli --workspace-root . --json get_build_order

# Query command metadata programmatically
theide-cli --json describe_command --name build_project

# Execute JSON script for automation
theide-cli --workspace-root . --json --json-script automation_plan.json
```

## Current Status

This version implements a clean NOGUI architecture where all commands connect to the `clicore` package instead of the GUI-dependent `ide` package. The CLI now builds cleanly in NOGUI mode and provides core IDE functionality through a forked core.

The new AI Driver Integration Layer v1 provides:
- Machine-readable JSON output with structured payloads
- Command introspection for programmatic discovery
- JSON script mode for AI-driven automation
- Deterministic and stable output for machine consumption

## Available Commands

- `open_file`: Open a file in the editor (FULLY INTEGRATED with CLI Core)
- `save_file`: Save the current or specified file (FULLY INTEGRATED with CLI Core)
- `find_in_files`: Search for patterns in files (FULLY INTEGRATED with CLI Core)
- `build_project`: Build a project with specified configuration (SIMULATED with CLI Core)
- `clean_project`: Clean project build artifacts (SIMULATED with CLI Core)
- `goto_line`: Navigate to a specific line in a file (NOW FULLY INTEGRATED with CoreEditor)
- `search_code`: Search code across the project (STUBBED in CLI Core)
- `show_console`: Retrieve console output (FULLY INTEGRATED with CLI Core)
- `show_errors`: Retrieve error list (FULLY INTEGRATED with CLI Core)
- `set_main_package`: Set the main package for the project (FULLY INTEGRATED with CLI Core)
- `list_commands`: List all available commands (special built-in command)
- `describe_command`: Describe command metadata and schema (special built-in command)

## Editor Commands

The CLI now includes several editor-specific commands that operate on in-memory buffers:

- `insert_text`: Insert text at a specific position in a file
  - Example: `theide-cli --workspace-root . insert_text --path src/main.cpp --pos 0 --text "// Auto-generated header\n"`
- `erase_range`: Erase a range of text from a file
  - Example: `theide-cli --workspace-root . erase_range --path src/main.cpp --pos 10 --count 20`
- `replace_all`: Replace all occurrences of a pattern with replacement text in a file
  - Example: `theide-cli --workspace-root . replace_all --path src/main.cpp --pattern "TODO" --replacement "DONE" --case_sensitive false`
- `undo`: Undo the last edit operation in a file
  - Example: `theide-cli --workspace-root . undo --path src/main.cpp`
- `redo`: Redo the last undone edit operation in a file
  - Example: `theide-cli --workspace-root . redo --path src/main.cpp`

## Headless Editor Core (CoreEditor)

The `clicore` package now includes a new CoreEditor subsystem that provides in-memory text buffer management with basic editing capabilities:

- **In-Memory Buffers**: Files opened via CLI are now loaded into in-memory representations for efficient editing
- **Basic Editing**: Supports insert, erase, replace operations on text buffers
- **Navigation**: Provides goto-line functionality for efficient navigation
- **Search/Replace**: Implements find-first and replace-all operations in buffers
- **Undo/Redo**: Includes basic undo/redo functionality for safe editing
- **Multiple Files**: The CoreIde now manages multiple open files using CoreEditor instances
- **CLI Integration**: All file operations now work with in-memory buffers instead of direct filesystem access

The CoreEditor is designed as a headless editor core that maintains basic editing capabilities without syntax highlighting, code completion, or other advanced editor features. This provides a foundation for AI-driven editing operations while remaining lightweight and CLI-compatible.

## Script Mode (Batch Commands)

The CLI now supports a script mode for executing multiple commands from a file:

- Use `--script PATH` flag to run a sequence of commands from a text file
- Each line in the script file contains a single command with its arguments
- Lines starting with `#` are treated as comments and ignored
- Empty lines are also ignored
- The script execution stops on the first error (v1 behavior)

Example script file (`commands.txt`):
```
set_main_package --name MyApp
open_file --path src/main.cpp
replace_all --path src/main.cpp --pattern TODO --replacement DONE --case_sensitive false
save_file --path src/main.cpp
build_project --name MyApp --config Debug
```

Usage: `theide-cli --workspace-root . --script commands.txt`

## CoreAssist: Headless Symbol Indexer

A new subsystem `CoreAssist` has been added to provide symbol analysis capabilities to the CLI. It implements a basic but extensible symbol indexer for C++ files in the workspace:

- **Symbol Extraction**: Heuristically extracts function names, class names, struct names, enum names, global variables, and #include references
- **Indexing**: Can index an entire workspace to create symbol-location mappings
- **Navigation**: Supports go-to-definition and find-usages operations
- **Heuristic Scanning**: Uses regex-free pattern matching and simple parsing for acceptable performance and accuracy
- **NOGUI Compatible**: Completely free of GUI dependencies and suitable for headless environments
- **Architecture**: Designed for future deeper parsing integration (v2) while providing immediate value with v1 heuristics

### Symbol Analysis Commands

Two new commands have been added to leverage CoreAssist functionality:

- `find_definition`: Finds the definition location of a specified symbol
  - Example: `theide-cli --workspace-root . find_definition --symbol MyClass`
  - Returns: file path and line number where the symbol is defined

- `find_usages`: Finds all usages of a specified symbol in the workspace
  - Example: `theide-cli --workspace-root . find_usages --symbol MyFunction`
  - Returns: list of file:line locations where the symbol is used

### CLI Usage

```bash
# Find where a symbol is defined (after workspace has been indexed)
theide-cli --workspace-root /path/to/workspace find_definition --symbol SymbolName

# Find all usages of a symbol
theide-cli --workspace-root /path/to/workspace find_usages --symbol SymbolName
```

**Note**: The indexer is heuristic-based (v1), so it may have limitations with complex C++ constructs. The architecture is prepared for more accurate parsing in future versions.

## CoreGraph: Workspace Dependency Graph

A new subsystem `CoreGraph` has been added to model the dependency relationships between packages in a workspace and provide graph-based analysis capabilities:

- **Package Dependency Graph**: Models uses-relationships between packages as directed edges in a dependency graph
- **Topological Sort**: Computes the build order in which packages should be built without violating dependency constraints
- **Cycle Detection**: Identifies circular dependencies that would prevent successful builds
- **Impact Analysis**: Determines which packages are affected by changes to a specific file based on the dependency graph

### Graph Operations

The CoreGraph provides several key operations to understand and work with package dependencies:

- **Build Order**: Computes the topological sort of packages, providing the order in which packages should be built
- **Cycle Detection**: Identifies cycles in package dependencies that would cause build failures
- **Affected Packages**: Performs impact analysis to determine which packages depend (directly or transitively) on a specific file

### Graph Commands

Three new commands have been added to expose CoreGraph functionality through the CLI:

- `get_build_order`: Gets the topological order of packages for building
  - Example: `theide-cli --workspace-root . get_build_order`
  - Returns: List of packages in the order they should be built

- `detect_cycles`: Detects circular dependencies in package graph
  - Example: `theide-cli --workspace-root . detect_cycles`
  - Returns: List of detected cycles or confirmation that no cycles exist

- `affected_packages`: Gets packages affected by changes to a file
  - Example: `theide-cli --workspace-root . affected_packages --path src/util.cpp`
  - Returns: List of packages that transitively depend on the specified file

### CLI Usage

```bash
# Get the build order for packages in the workspace
theide-cli --workspace-root /path/to/workspace get_build_order

# Detect any circular dependencies in package relationships
theide-cli --workspace-root /path/to/workspace detect_cycles

# Find which packages would be affected by changes to a specific file
theide-cli --workspace-root /path/to/workspace affected_packages --path src/mypackage/MyFile.cpp
```

**Note**: These commands require a properly set workspace root and will analyze the dependency graph based on the `.upp` files in the workspace.

## CoreRefactor – Workspace Refactoring Engine

A new subsystem `CoreRefactor` has been added to provide workspace-aware refactoring capabilities to the CLI. It implements a first-generation refactoring engine focused on common code maintenance operations:

- **Symbol Renaming**: Renames symbols across the entire workspace, updating all references
- **Dead Include Removal**: Identifies and removes unused #include directives from files
- **Include Path Canonicalization**: Standardizes include paths to their canonical forms
- **Workspace-Aware**: Operations understand package structure and dependencies
- **Safe Editing**: All operations work on in-memory editors with proper overlap detection
- **NOGUI Compatible**: Fully compatible with headless environments

### Refactoring Commands

Three new commands have been added to expose CoreRefactor functionality through the CLI:

- `rename_symbol`: Renames a symbol throughout the workspace
  - Example: `theide-cli --workspace-root . rename_symbol --old MyOldClass --new MyNewClass`
  - Returns: Confirmation of successful rename operation

- `remove_dead_includes`: Removes unused include directives from a file
  - Example: `theide-cli --workspace-root . remove_dead_includes --path src/main.cpp`
  - Returns: Number of includes removed

- `canonicalize_includes`: Standardizes include paths in a file to canonical forms
  - Example: `theide-cli --workspace-root . canonicalize_includes --path src/main.cpp`
  - Returns: Number of includes canonicalized

### CLI Usage

```bash
# Rename a symbol throughout the entire workspace
theide-cli --workspace-root /path/to/workspace rename_symbol --old OldName --new NewName

# Remove unused include directives from a file
theide-cli --workspace-root /path/to/workspace remove_dead_includes --path src/File.cpp

# Canonicalize include paths in a file
theide-cli --workspace-root /path/to/workspace canonicalize_includes --path src/File.cpp
```

**Note**: These refactoring operations modify file content in the in-memory editor buffers and require saving to persist changes to disk.

## Implementation Notes

- All handlers now connect to the CLI Core (`clicore`) through the `IdeSession` interface
- The `IdeSession` has been refactored to use `CoreIde` instead of `Ide`/`IdeHost`
- NO GUI dependencies: The `ide` package is NO LONGER used directly
- The `clicore` package is built NOGUI and can be used in headless environments
- Build operations are currently simulated but provide the structure for integration with real build system
- The architecture maintains clean separation while providing a path for more TheIDE functionality to be ported

## AI/Automation Integration

The new AI Driver Integration Layer v1 is specifically designed to enable external AI systems to reliably drive the CLI engine:

- **Machine-Friendly**: Structured JSON output via `--json` flag
- **Introspectable**: Command metadata available via `describe_command`
- **Deterministic**: Clean, predictable output in machine mode
- **Embeddable**: Can be used as a subprocess driven by stdin/stdout
- **Scriptable**: JSON-based script execution for complex automation

This layer provides the protocol and plumbing for AI processes to interact with the engine without embedding any real ML models in the CLI itself.