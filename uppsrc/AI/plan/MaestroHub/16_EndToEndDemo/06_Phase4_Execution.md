# Phase 4: Build & Execution
# Status: TODO

## Scenarios 56-75

56. **Build Trigger**: Use 'OpsRunner' to execute `umk` (build).
    - **Input**: Source code.
    - **Output**: `build.log`.
57. **Toolchain Validation**: Verify that the selected toolchain is correctly invoked.
58. **Console Output**: Monitor the 'Internal Console' for real-time build output.
59. **Build Error Detection**: (Intentional) Introduce a syntax error and verify it appears in 'LogAnalyzer'.
    - **Output**: Finding in scan list.
60. **Log Finding Navigation**: Click the error and verify it jumps to the line in 'RepoView'.
61. **Success Build**: Fix the error and verify the build completes.
    - **Output**: `SmallGuiAppForAccounting` binary.
62. **Ops Runner Execution**: Run the compiled binary via 'OpsRunner'.
    - **Input**: Binary.
    - **Output**: Runtime `stdout`.
63. **Stop Process**: Terminate the running app via the 'Stop' button in Ops.
64. **Environment Check**: Verify that environment variables were correctly passed to the app.
65. **Debugger Launch**: Open 'DebugWorkspace' and start the app under the debugger.
66. **Breakpoint Setting**: Set a breakpoint in `Account::Deposit`.
67. **Step Over**: Perform 'Step Over' in the debugger and verify 'Locals' update.
68. **Call Stack Verification**: Verify the 'CallStack' tree displays the correct frames.
69. **Debugger Stop**: Gracefully stop the debugger session.
70. **Build Cache Check**: Use 'MaintenancePane' to view the size of the build cache.
71. **Quota Usage Check**: Verify how much AI quota was used during implement/build cycles.
72. **Ops History**: Verify the 'OpsRunner' history shows previous build/run commands.
73. **Process Trace**: View the execution trace in the 'Internal Console'.
74. **Build Method Switch**: Switch to "Release" mode and rebuild.
75. **Binary Verification**: Verify the release binary exists and is smaller.

## Summary
- **Input**: C++ Source code.
- **Output**: A compiled and executable application with associated logs.
