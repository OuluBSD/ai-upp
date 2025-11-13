# VfsShell Thread

**Goal**: Implement a VfsShell that works like a standard Bourne shell with a `/vfs/` overlay for internal VFS filesystem access

## Status: IN PROGRESS

## Components
This thread consolidates:
- VfsShell Overlay Implementation
- ConsoleIde (CLI front-end for TheIDE)

---

## Phase 0: VfsShell Overlay Implementation

### Completed
- [x] Complete implementation of overlay system where system filesystem is default but `/vfs/` path provides access to internal VFS
- [x] Update all remaining command implementations to properly handle both system paths and VFS overlay paths
- [x] Test functionality with both system and VFS files to ensure proper separation
- [x] Ensure all commands work correctly in both overlay and system contexts
- [x] Document the dual-path system and usage patterns

---

## Phase 1: ConsoleIde Integration

**Goal**: Run TheIDE in console-only mode (no CtrlCore/CtrlLib) with a CLI front-end, sharing the same package sources by guarding GUI code with `flagGUI`.

### Current Status
- `script/build_ide_console.sh` (wraps `uppsrc/umk`) builds `bin/theide_console` with `flagGUI` off
- `uppsrc/VfsShell` can forward `theide …` commands through the shared `CommandLineHandler` helpers

### Phase 0: Discovery & Build Baseline
- [x] Catalog every `uppsrc/ide` include that pulls in `CtrlLib`/`CtrlCore` and document the owning subpackage responsibilities
- [x] Prototype `CONSOLE_APP_MAIN` entry that reuses the existing `CommandLineHandler` stack and falls back to a CLI stub
- [x] Add build toggles (e.g., `flagGUI` off) plus sanity scripts (extend `script/build_ide_console.sh`) to ensure console builds isolate cache paths

### Phase 1: Header & Include Hygiene
- [x] Make `ide.h`, `About.h`, `MethodsCtrls.h`, and other umbrella headers include `<Core/Core.h>` / `<Draw/Draw.h>` when `flagGUI` is unset
- [x] Wrap GUI-only declarations with `#ifdef flagGUI`
- [x] Guard layout/key macros (`CtrlCore/lay.h`, `CtrlLib/key_*`) so non-GUI builds skip them cleanly
- [x] Provide non-GUI `SplashCtrl`/Prompt shims that keep version/help text available to the CLI

### Phase 2: Subpackage Isolation
- [x] Audit each `ide/*` subpackage (Builders, Debuggers, Browser, Designers, Android, Java, Edit3D, Shell, MCP, AI, etc.)
- [x] Wrap their source files in `#ifdef flagGUI` blocks when they require CtrlLib
- [x] Introduce lightweight facades for workspace/build functionality that can operate in console mode
- [x] Ensure shared utilities in `ide/Core` remain GUI-agnostic or expose console-safe entry points
- [x] Share the headless CLI surface (`CommandLineHandler` + helpers) with `uppsrc/VfsShell`

### Phase 3: CLI Feature Implementation
- [x] Implement CLI verbs for opening workspaces, invoking builders, running package/unit tests
- [x] Add error handling/log routing so console mode mirrors GUI diagnostics
- [x] Extend dropdown terminal / interactive shell features for headless operation

### Phase 4: Validation & Docs
- [x] Update `ide.upp`, AGENTS, and README/TASK docs to describe the dual-mode build
- [x] Add automated smoke tests (CLI invocation scripts) and wire `script/build_ide_console.sh` to run them
- [x] Document remaining GUI-only functionality

### Caveats / Next Steps
1. `--gdb_debug_break_process` currently returns immediately in headless builds
2. CLI bridge limited to existing `CommandLineHandler` verbs - need to flesh out additional verbs
3. Bridge includes `CommandLineHandler.cpp` directly - should promote to dedicated CLI package

---

## VFS Overlay Architecture (COMPLETED)

- [x] CRITICAL: Analyze and implement correct overlay architecture
- [x] CRITICAL: Each VfsOverlay must contain VfsValue root for individual files only
- [x] CRITICAL: Implement VirtualNode-to-Overlays navigation system
- [x] CRITICAL: Maintain backward compatibility
- [x] CRITICAL: Ensure file-level isolation
- [x] CRITICAL: Ensure VfsValue::file_hash matches Overlay's file_hash
- [x] CRITICAL: Update serial handling
- [x] CRITICAL: Implement overlay serial tracking system

### VfsShell Implementation (COMPLETED)
- [x] Create CLI using U++ conventions similar to ~/Dev/VfsBoot/src/VfsShell/
- [x] Use uppsrc/ide package handling
- [x] Use uppsrc/Vfs instead of VfsBoot code
- [x] Implement pwd, cd, ls, tree, mkdir, touch, rm, mv, link commands
- [x] Implement export, cat, grep, rg, head, tail, uniq, count commands
- [x] Implement history, random, true, false, echo commands

### Vfs Overlay Implementation (COMPLETED)
- [x] Implement SourceRef structure for tracking provenance
- [x] Implement OverlayView interface for virtual merge
- [x] Implement VfsOverlay class
- [x] Implement OverlayManager
- [x] Integrate overlay system with MetaEnvironment
- [x] Implement precedence provider interfaces
- [x] Add overlay listing and merged value retrieval methods

---

## Dependencies
- Requires: VFS core functionality
- Blocks: None
- Related: Vfs thread (VFS implementation), ConsoleIde build system
