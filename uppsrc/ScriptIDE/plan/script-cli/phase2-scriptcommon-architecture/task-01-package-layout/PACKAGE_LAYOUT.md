# Package Layout: ScriptCommon

This document defines the file structure and module boundaries for the new `ScriptCommon` package.

## Directory Structure
Target: `uppsrc/ScriptCommon/` (Flat file layout, following U++ conventions).

## Module Definitions

### 1. Runtime Module
**Files**: `RunManager.h`, `RunManager.cpp`, `Linter.h`, `Linter.cpp`
**Responsibility**:
- Centralize `PyVM` execution logic.
- Handle script compilation, IR management, and breakpoint hits (callback-based).
- Provide static code analysis (Linter).

### 2. Environment Module
**Files**: `PathManager.h`, `PathManager.cpp`
**Responsibility**:
- Manage search paths (`sys.path`).
- Sync environment variables to the Python runtime.

### 3. Config Module
**Files**: `IDESettings.h`
**Responsibility**:
- Define the `IDESettings` data structure.
- Handle binary and JSON serialization.
- NO dependencies on `Draw` or `CtrlCore` (uses `String/int` for appearance options).

### 4. Plugin Core Module
**Files**: `PluginCoreInterfaces.h`, `PluginRegistry.h`, `PluginRegistry.cpp`
**Responsibility**:
- Define `IBasePlugin` and `IBaseDocumentHost` (non-GUI base interfaces).
- Manage global plugin discovery and enablement.
- *Note*: GUI-specific hooks stay in `ScriptIDE/PluginInterfaces.h`.

### 5. Session/Workspace Module
**Files**: `DocumentModel.h`, `DocumentModel.cpp`, `WorkspaceModel.h`, `WorkspaceModel.cpp`
**Responsibility**:
- Track open files (buffers), dirty states, and file paths.
- Manage project root and file system change notifications.
- Decouple "Open Files" logic from the `PythonIDE` main window.

## Migration Mapping

| Current ScriptIDE File | New ScriptCommon File | Split Action |
| :--- | :--- | :--- |
| `RunManager.cpp/h` | `RunManager.cpp/h` | Move as-is. |
| `Linter.cpp/h` | `Linter.cpp/h` | Move as-is. |
| `PathManager.cpp/h` | `PathManager.cpp/h` | Move as-is. |
| `IDESettings.h` | `IDESettings.h` | Move as-is. |
| `PluginInterfaces.h` | `PluginCoreInterfaces.h` | Extract non-GUI base. |
| `PluginManager.cpp/h` | `PluginRegistry.cpp/h` | Extract registry logic. |
| `PythonIDE.cpp/h` | `WorkspaceModel.cpp/h` | Extract `open_files` array. |
| `PythonIDE.cpp/h` | `DocumentModel.cpp/h` | Extract `FileInfo` and dirty logic. |

## Header Policy
- `ScriptCommon.h`: Master header including all sub-headers.
- NO GUI packages in `uses` section of `ScriptCommon.upp`.
