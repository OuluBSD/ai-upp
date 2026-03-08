# Scenarios 81-100: Build, Ops & UX Evaluation
# Status: TODO

## Objective
Verify build toolchain integration, ops automation, and the visual UX testing framework.

## Build & Ops Scenarios (81-90)
81. **Toolchain Scan**: Open 'BuildMethodsDialog' and verify it detects system compilers (GCC/Clang/MSVC).
82. **Custom Toolchain**: Add a custom toolchain path and verify it appears in the list.
83. **Ops Runner - Run**: Select an 'Ops' script (e.g., `build.py`) and click 'Run'. Verify output appears in the console.
84. **Environment Variables**: Edit environment variables in the 'BuildMethods' dialog and verify they are passed to the process.
85. **Process Termination**: Start a long-running 'Ops' process and verify it can be terminated via the 'Stop' button.
86. **Quota Monitoring**: Monitor the 'QuotaIndicator' during multiple AI-assisted build fixes.
87. **Standard Locations**: Verify that common build tool locations are scanned by default.
88. **Build Configuration Sync**: Change build flags in 'Config' and verify they are reflected in the 'Ops' command line.
89. **Maintenance - Purge Cache**: Use 'MaintenancePane' to purge the build cache and verify disk space is reclaimed.
90. **Maintenance - Core Sync**: Use the 'Sync Core' feature and verify it pulls latest U++ headers correctly.

## Product & UX Scenarios (91-100)
91. **UX Factory - Run Test**: Select a UI test in 'UXEvaluationFactory' and click 'Run'.
92. **UX Diffing**: Verify the 'DiffView' (Image) correctly highlights pixels that deviate from the baseline.
93. **UX Approval**: Click 'Approve' on a deviating UI state and verify the baseline is updated.
94. **Workflow Graph Interaction**: Open 'ProductPane', select a workflow, and click nodes to view details.
95. **Enact Step**: Right-click a workflow node and select 'Enact'. Verify it triggers the assistant.
96. **Tutorial Progress**: Open 'TutorialPane', click 'Next' through 5 steps, and verify the UI state changes (auto-navigation).
97. **Tutorial - Prev**: Verify 'Prev' correctly reverts to the previous tutorial state.
98. **Tutorial Completion**: Reach the end of the tutorial and verify the 'Congratulations' state.
99. **Fleet Dashboard**: Load multiple project paths and verify the 'ProjectGrid' (TreeArrayCtrl) displays them.
100. **Automation Queue**: Verify that background automation tasks are listed in the 'AutomationQueue'.

## Verification Method
- Process ID (PID) monitoring.
- Image pixel comparison for UX Diff.
- Tutorial step index tracking.
