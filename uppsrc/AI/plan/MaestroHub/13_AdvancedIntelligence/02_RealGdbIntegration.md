# Task: Real GDB Integration

# Status: TODO

# Description
Replace the simulated `GdbService` stub with a real GDB/MI implementation.

# Objectives
- [ ] Implement `GdbClient` using `LocalProcess` to launch `gdb --interpreter=mi`.
- [ ] Implement asynchronous output parsing (GDB/MI records).
- [ ] Support core commands: `-exec-run`, `-exec-continue`, `-exec-step`, `-exec-next`.
- [ ] Implement breakpoint management: `-break-insert`, `-break-delete`.
- [ ] Wire the real service to the `DebugWorkspace` UI.

# Requirements
- Non-blocking communication.
- Capture and display GDB's `stderr` and console output.
- Support basic stack frame and variable inspection.
