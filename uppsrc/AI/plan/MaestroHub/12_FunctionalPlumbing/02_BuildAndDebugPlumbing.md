# Task: Build & Debug System Plumbing

# Status: DONE

# Description
Integrate real build and debug capabilities into the Cockpit, leveraging existing U++ IDE infrastructure where possible.

# Objectives
- [x] Implement "Build Method" (Toolchain) management GUI (`BuildMethodsDialog`).
- [x] Reference `uppsrc/ide/Builders` and `~/upp/*.var` files for implementation (`ToolchainManager`).
- [x] Establish a "Debugger Service" that can be used by the Cockpit GUI but remains detached for `MaestroCLI` usage (`DebuggerService`, `GdbService`).
- [x] Populate `DebugWorkspace` with real data from the debugger backend (`DebugWorkspace` connected to `GdbService` events).

# Requirements
- [x] Maintain upstream compatibility for `uppsrc/ide`.
- [x] `MaestroCLI` must remain functional without GUI flags.
- [x] Re-use `ide` logic for `*.var` file parsing and toolchain detection.
