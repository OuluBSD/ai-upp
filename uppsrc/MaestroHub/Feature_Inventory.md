# Maestro Feature Inventory & Mapping

## The "Meta Pipeline" Sequence

| Phase | CLI Command | GUI Component | Parity Status |
| :--- | :--- | :--- | :--- |
| 1. Bootstrap | `init` | `InitDialog` | Full |
| 2. Modeling | `runbook` | `ProductPane` / `RunbookEditor` | Full |
| 3. Programming | `workflow` | `ProductPane` / `StateEditor` | Full |
| 4. Intelligence | `repo` | `TechnologyPane` (RepoView) | Full |
| 5. Roadmapping | `plan`, `track`, `phase`, `task` | `TechnologyPane` (PlanView) | Full |
| 6. Orchestration | `make` | `TechnologyPane` (Build Actions) | Partial |
| 7. Execution | `run` | - | **ORPHANED** |

## Supporting Ecosystem

| Category | CLI Command | GUI Component | Parity Status |
| :--- | :--- | :--- | :--- |
| Intelligence | `tu` | `TUBrowser` | Partial |
| Intelligence | `log` | `LogAnalyzer` | Full |
| Maintenance | `issues` | `IssuesPane` / `TriageDialog` | Full |
| Maintenance | `solutions` | `IssueDialogs` | Full |
| Interaction | `discuss`, `ai` | `MaintenancePane` (AIChat) | Full |
| Operations | `ops`, `settings` | `OpsRunner` / `ConfigDialog` | Full |
| Optimization | `cache`, `track-cache` | - | **ORPHANED** |
| Education | `tutorial` | - | **ORPHANED** |
| Advanced | `evidence`, `playbook`, `convert` | - | **ORPHANED** |

## High-Value Features to Implement
1. **`run` Execution Console**: Dedicated workspace for binary execution, device targeting (ADB), and structured observability.
2. **`tutorial` Onboarding**: Interactive paging system within the GUI to guide new users.
3. **`evidence` Packager**: Visual audit trail collector for PR verification.

## Orphaned Features (CLI Only)
1. **Evidence Collection**: No GUI for `maestro evidence`. High value for audit trails.
2. **Playbook Management**: No GUI for managing instruction sets (`maestro playbook`).
3. **Conversion Orchestrator**: The core batch-processing engine (`maestro convert`) has no visual cockpit yet.
4. **Regression Replay**: No UI to trigger or monitor `regression-replay`.

## Orphaned Features (GUI Only)
1. **Interactive WorkGraph**: Graph visualization has no direct CLI equivalent beyond simple DOT exports.
2. **Real-time Log Stream**: The breadcrumb timeline in `WorkPane` is much denser than `maestro work list`.

## Mapping to User Tasks
- **Task: Batch Porting** -> `convert` (Orphaned) + `WorkGraph` (GUI).
- **Task: System Health** -> `ops doctor` (GUI).
- **Task: Issue Resolution** -> `issues` (GUI) + `log` (GUI).
- **Task: Rule Design** -> `runbook` (GUI) + `playbook` (Orphaned).