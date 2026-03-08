# Maestro System Lifecycle Analysis

## 1. Project Lifecycle
| State | Trigger | Description |
| :--- | :--- | :--- |
| **Discovered** | `repo scan` | Codebase detected but no `docs/maestro` found. |
| **Initialized** | `init` | Metadata structure established. |
| **Monitored** | `log scan` / `ops` | Active observation of logs and health. |
| **Optimized** | `convert` / `tu build` | Deep intelligence cached and structural refactors applied. |
| **Archived** | User Action | Cold storage of sessions and breadcrumbs. |

## 2. Issue Lifecycle
| State | Trigger | Description |
| :--- | :--- | :--- |
| **Open** | `log scan` Finding | Detected but not yet analyzed. |
| **Reacted** | `issues triage` | Acknowledged by user. |
| **Analyzing** | `issues analyze` | AI is determining root cause and solutions. |
| **In-Work** | `work start` | AI is actively implementing a fix. |
| **Verified** | `regression-replay` | Fix tested and build passes. |
| **Closed** | User Action | Merged and resolved. |
| **Ignored** | `issues ignore` | False positive or won't fix. |

## 3. Runbook Lifecycle
| State | Trigger | Description |
| :--- | :--- | :--- |
| **Draft** | `runbook add` | Narrative idea or conversation fragment. |
| **Resolved** | `runbook resolve` | Structured into actionable command steps. |
| **Verified** | `ops simulation` | Steps tested in a dry-run or simulation. |
| **Published** | User Action | Moved to library for reuse across projects. |
| **Deprecated** | User Action | Outdated by new workflows or code changes. |

## 4. Work Session Lifecycle
| State | Trigger | Description |
| :--- | :--- | :--- |
| **Active** | `work start` | AI is running and communicating. |
| **Paused** | `work pause` | User intervention or awaiting context. |
| **Branched** | `work subwork` | Nested session spawned for sub-task. |
| **Finalized** | `work finish` | Goal met, results committed. |
| **Broken** | AI Loop / Error | Abnormal termination requiring debug. |
