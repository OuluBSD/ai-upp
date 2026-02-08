# Task: Real GDB Integration

# Status: DONE

# Description
Replace the simulated `GdbService` stub with a real GDB/MI implementation.

# Objectives
- [x] Implement `GdbClient` (in `GdbService`) using `LocalProcess` to launch `gdb --interpreter=mi`.
- [x] Implement asynchronous output parsing (GDB/MI records).
- [x] Support core commands: `-exec-run`, `-exec-continue`, `-exec-step`, `-exec-next`.
- [x] Implement breakpoint management: `-break-insert`.
- [x] Wire the real service to the `DebugWorkspace` UI.

# Requirements
- [x] Non-blocking communication via `PostCallback` polling.
- [x] Capture and display GDB output.
- [x] Parse `*stopped` records to update stack frames.