# MaestroHub UX Analysis

## User Perspective: The Automation Orchestrator
### Expectations
- Clear overview of state across multiple projects.
- Robust process management (stopping, restarting, and mass-triaging).
- Low-friction reuse of runbooks and workflows.

### User Needs
1. **Fleet Management**: See status of multiple projects and active AI sessions at a glance.
2. **Batch Processing**: Ability to run scans or triages across several modules simultaneously.
3. **Template Management**: Easily design and verify a Runbook once, then deploy it automatically.
4. **Reliability Monitoring**: High-density logs and audit trails for automated AI actions.

## Research Findings: Current State Analysis

### 1. Multi-Project Management (Friction)
- **Current**: The UI is strictly single-root. Changing projects requires "App -> Select Root".
- **Friction**: Users cannot see the health of "Project B" while AI is working on "Project A".
- **Recommendation**: Implement a "Workspace Sidebar" or "Global Dashboard" that shows sparklines/status for all registered projects.

### 2. Process Monitoring & Orchestration (Friction)
- **Current**: Active sessions are hidden in the "Work" tab or "Sessions" history.
- **Friction**: High-volume automated usage (e.g., mass-triaging 100 issues) creates a "black box" feel. The user needs to see the *queue* of AI tasks.
- **Recommendation**: Create an "Automation Queue" view showing active, pending, and failed background processes with global stop/pause controls.

### 3. Reuse & Design Lifecycle
- **Current**: Runbook and Workflow editors are distinct dialogs.
- **Friction**: The link between "Designing a Runbook" and "Executing it on Project X" is purely manual enactment.
- **Recommendation**: Treat Runbooks as "Blueprints" in a central library. Add a "Deploy Runbook" wizard that targets one or more projects.

## Refined GUI Restructuring Strategy
1. **Consolidate Intelligence**: Merge Log Analyzer, TU Browser, and Repo View into a single "Codebase Intelligence" hub.
2. **Prioritize the Dashboard**: The home screen must show "Fleet Status" (All projects) + "Active AI Processors".
3. **Dedicated "Work" Cockpit**: Optimize the Work tab for high-density log streaming and "Human-in-the-loop" decision gates.
