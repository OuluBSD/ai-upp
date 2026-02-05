# Core Architecture Alignment

## 1. The Layered Lifecycle (A1)
- **Current Observation**: UI is tabbed but doesn't explicitly enforce the "init -> runbook -> workflow -> ..." sequence.
- **Proposed Refinement**: Add a "Pipeline Bar" at the top that breadcrumbs the current project phase.

## 2. Multi-Surface Operation (A2)
- **Current Observation**: Chat is a tab. Execution is a tab.
- **Proposed Refinement**: Move Chat to a collapsible side panel available across ALL tabs to support "interoperable control".

## 3. Persistent Traceability (A3)
- **Current Observation**: Logs and sessions are separate.
- **Proposed Refinement**: Global "Audit Trail" view that time-correlates log findings, AI thoughts, and code commits.
