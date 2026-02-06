# MaestroHub Comprehensive Scenarios (Project: MegaFileUtil)

## 1. Repository & Intelligence (`init`, `repo`, `tu`, `log`)
- **Scenario A (First Scan)**: User initializes `MegaFileUtil` (a massive utility library). They use the `Pipeline` pane to watch the AI index 5,000 files.
- **Scenario B (Circular Dep Hunt)**: User uses the `TU Browser` to find why `FileParser.cpp` depends on `NetConfig.h`. They view the include-chain graph.
- **Scenario C (Log Mining)**: A crash log from MegaFileUtil is dropped into `Log Analyzer`. It identifies a null pointer in `StreamBuffer::Flush`.

## 2. Planning & Conversion (`plan`, `convert`, `evidence`, `playbook`)
- **Scenario A (Migration Plan)**: User wants to port MegaFileUtil from U++ 2018 to 2026. `Conversion Factory` generates a 50-step plan.
- **Scenario B (Batch Porting)**: User starts the "Transformation Units" run. They watch the `AI Rationale` tab as it explains why it's changing `Vector` to `RecyclerPool`.
- **Scenario C (Playbook Enforcement)**: User selects the "Security-First" playbook. The conversion tool automatically adds bounds checking to all array accesses in `MegaFileUtil`.

## 3. Operations & Triage (`issues`, `ops`, `settings`)
- **Scenario A (Triage Wizard)**: 10 issues are imported from the CLI. User runs the Triage Wizard; AI suggests "Ignore" for 3 duplicates and "Fix" for 7 others.
- **Scenario B (Ops Doctor)**: User runs `Ops Runner` -> "Check Directory Integrity". It finds orphaned `.maestro` folders and offers to prune them.
- **Scenario C (Model Switching)**: User switches from `Gemini Pro` to `Gemini Flash` in `Settings` to speed up the initial repository inventory.

## 4. Execution & Work (`work`, `runbook`, `workflow`, `debug`)
- **Scenario A (Manual Subwork)**: AI is stuck on a complex template error. User clicks "Subwork", creates a manual session, and fixes it themselves.
- **Scenario B (Runbook execution)**: User executes the "Release MegaFileUtil" runbook. The `Work Dashboard` guides them through: Build -> Test -> Sign -> Upload.
- **Scenario C (Debug Trace)**: A conversion task failed. User opens `Execution Console` to see the exact CLI command and environment variables that caused the fail.