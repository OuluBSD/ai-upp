# Task: GDB Crash Test

# Status: DONE

# Description
Develop a comprehensive Python test script that verifies the GDB integration using the new Accessibility framework.

# Objectives
- [x] Create `share/py/maestro/tests/test_gdb_crash.py`.
- [x] Create and compile `crash.cpp` as a test target.
- [x] Enhance `DebugWorkspace` with `TargetBinary` input and `Continue` button.
- [x] Script logic:
    - Mocks AI responses.
    - Navigates to "Execution Console".
    - Sets the target binary to `./crash`.
    - Runs the debugger.
    - Continues until the crash is hit.
    - Verifies that `main` and `crash_me` appear in the call stack using `find_all`.

# Requirements
- Full end-to-end verification of the GUI + Logic + System Integration.

# Implementation Details
- `DebugWorkspace` now has a `LayoutId("TargetBinary")` for reliable automation.
- `TreeCtrl::Access` enables full inspection of the call stack from Python.
- `builtin_find_all` in Python bindings allows verifying multiple frames at once.