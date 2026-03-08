# Task: UX Evaluation Factory

# Status: DONE

# Description
Establish a specialized environment for "Snapshot & Compare" UI testing, allowing the AI to verify visual fidelity against baselines.

# Objectives
- [x] Implement "Snapshot & Compare" logic (Simulated for now, extensible to real capture).
- [x] Create `UXEvaluationFactory` pane/dialog to manage test cases and baselines.
- [x] Integrate with `ProductPane` (via Menu).

# Implementation Details
- **UXEvaluationFactory**:
    - Manage list of visual tests.
    - Display Baseline vs Current vs Diff images.
    - "Approve" workflow to update baselines.
- **UI:** 3-pane image comparison view.