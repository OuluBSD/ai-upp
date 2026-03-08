# Phase 6: Validation & Evidence
# Status: TODO

## Scenarios 91-100

91. **UX Evaluation Init**: Open 'UXEvaluationFactory' and run the first UI test.
    - **Input**: `SmallGuiAppForAccounting` binary.
    - **Output**: Screenshot baseline.
92. **UX Diffing**: Trigger a UI change (e.g., change a label) and run the test again.
    - **Output**: Pixel-diff in 'DiffView'.
93. **UX Approval**: Approve the new UI state to update the baseline.
94. **Evidence Collection**: Gather build logs, screenshots, and task MDs into 'EvidencePane'.
95. **Evidence Integrity**: Verify hashes of all collected artifacts.
96. **PDF Export**: Generate the "Accounting App Compliance Report" (PDF).
    - **Input**: Evidence items.
    - **Output**: `Accounting_App_Report.pdf`.
97. **Audit Trail Review**: Open 'AuditTrailCorrelator' and browse the full project history.
98. **Tutorial Completion**: Run the 'TutorialPane' to its final step "Project Delivery".
99. **Fleet Dashboard Update**: Verify the app is correctly listed with its final metadata.
100. **Final Shutdown**: Exit MaestroHub and verify all background processes (geckodriver, etc.) are clean.

## Summary
- **Input**: Working application and project artifacts.
- **Output**: A certified, documented, and visually verified project package.
