# Task: Conversion Orchestrator Integration

# Status: DONE

# Description
Bring the `convert` CLI power to the GUI, allowing orchestrated porting of legacy codebases (e.g., Python to C++) with AI assistance. This serves as the "Factory Floor" for large-scale code transformations.

# Objectives
- **Inventory Explorer**: Visualize the discovered conversion units (functions, classes, files).
- **Plan Management**: UI for reviewing and adjusting the conversion order/dependencies.
- **Orchestration Control**: Start, pause, and monitor batch AI transformation tasks.
- **Validation Cockpit**: Link to `SemanticIntegrity` checks and regression replays.

# Technical Tasks
- [x] Implement `ConversionPane` in `MaestroHub.h/cpp`.
- [x] Integrate `ConversionOrchestrator` core logic (Inventory, Plan, Run, Validate).
- [x] Visual transformation tree with unit status.
- [x] Progress log with QTF support for real-time monitoring.
- [ ] Future: Integrate `DiffCtrl` for side-by-side comparison (Requires bazaar/DiffCtrl).

# UI Requirements (WinXP Aesthetic)
- **Top**: Conversion Pipeline Status (Inventory -> Plan -> Run -> Validate).
- **Left**: Transformation Tree (Assemblies -> Packages -> Files).
- **Center**: Porting Workspace (Source/Target Diff placeholder, AI Rationale).
- **Bottom**: Batch Progress Log (detailed tail of AI backend output).
- Standard "Start Factory" / "Stop Line" iconography.
