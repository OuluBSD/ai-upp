# Maestro — Consolidated Feature & Property List (English)

This is a **feature/property inventory** (not user scenarios), compiled from everything discussed.

---

## A) Core Product Architecture

### A1. Layered lifecycle
- Initialization -> narrative modeling -> formal workflow -> repository intelligence -> planning -> build -> run/debug.
- Supports both greenfield and legacy/reverse-engineering flows.

### A2. Multi-surface operation
- CLI-first foundation with GUI parity where relevant.
- Human-facing and AI-facing control paths coexist.

### A3. Persistent traceability
- Conversations, sessions, logs, and intermediate artifacts are storable and resumable.
- Strong cross-linking between plan items, issues, work sessions, and evidence.

---

## B) `init` — Project Bootstrap & Storage Topology

### Features
- Repository bootstrap and Maestro metadata initialization.
- Forced re-initialization with preservation intent.
- Monorepo subproject isolation.

### Properties
- Storage layout strategies:
  - `docs/maestro/`
  - `.maestro/`
  - distributed project-adjacent layout
- Mechanical layout migration/conversion between storage modes.
- Deterministic metadata handling during migration.

---

## C) `runbook` — Narrative Modeling Network

### Features
- First-class runbook CRUD + step operations.
- Freeform-to-structured runbook resolution.
- AI discussion-assisted runbook authoring.
- Diagram rendering (UML/PUML style).
- Archive/restore lifecycle.

### Properties
- Runbooks can model:
  - GUI flows
  - CLI procedures
  - Web flows
  - TUI/ncurses flows
  - Network protocol chains
  - DBus message sequences
- Supports fragmented/conflicting inputs and later coherence synthesis.
- Subcategory/lifecycle partitioning (draft, resolve, discuss, render, archive).
- Can represent advanced technical pipelines (e.g., VR 6DOF sensor fusion distribution patterns).

---

## D) `workflow` — Formal State/Logic Graph

### Features
- Runbook-to-workflow conversion.
- AI-assisted workflow synthesis and conflict review.
- Visualization and state transition analysis.
- Reverse engineering from code -> workflow.
- Back-projection workflow -> runbook.

### Properties
- High-level programming abstraction.
- Conflict detection hooks with user policy steering.
- Supports hybrid simulation with runbook layer (user-flow + machine-flow).

---

## E) `repo` — Repository Intelligence Hub

### Features
- Multi-ecosystem project scanning.
- Assembly/package hierarchy introspection.
- Reference integrity validation.
- Roadmap/plan import visibility.
- Hub-style distribution integration (pull/publish orientation).

### Properties
- Broad project-type intent (code + asset-centric projects).
- Current practical emphasis: Ultimate++ and Gradle.
- Scan self-awareness:
  - support confidence
  - unresolved/ambiguous areas
- Conceptual split:
  - repo = package topology
  - TU = file/symbol topology
- Package-manager-like reasoning layer (dependency/resolution style).

---

## F) `plan` — Track/Phase/Task Roadmapping

### Features
- Hierarchical plan entities:
  - track
  - phase
  - task
- Detailed metadata editing per level.
- AI discussion scoped to selected entity.
- Priority scoring and dependency modeling.

### Properties
- AI execution granularity typically phase-first, task-decomposition as needed.
- Supports saved/resumable planning conversations.
- Enables plan pivots based on execution outcomes.
- Dependency semantics via produce/require tags.
- Circular dependency detection expected.

---

## G) `make` — Build Orchestration

### Features
- Compiler-aware build orchestration for TU-compatible output.
- Internalized practical support where external build paths are insufficient.
- Toolchain add/remove/auto-detect.
- Cross-compilation targeting.
- Build trace of used source/assets.
- Run handoff trigger into execution workspace.

### Properties
- Designed to support TU extraction constraints.
- Supports host/target divergence and multi-arch builds.
- Current known support focus: U++ and Gradle.

---

## H) `run` — Execution & Debug Workspace

### Features
- Execution of built binaries/targets.
- Device-aware targeting (e.g., ADB-connected Android devices).
- Remote run/debug (e.g., SSH-linked contexts).
- Debug toolkit:
  - call stack
  - locals
  - expression eval
  - breakpoints
  - step controls
  - optional disassembly
- Unified runtime observability panel.

### Properties
- Separate domain from build, but tightly handshaked from make.
- Framework-aware logging preference (framework logs vs stdout/stderr).
- Extensible telemetry/log representation:
  - structured payloads (JSON)
  - thread/context columns
  - filters
  - foldable detail
  - chart-like diagnostics
- Runtime profiling orientation (latency, frame timings, resource hotspots).

---

## I) `log` — Build/Runtime Log Intelligence

### Features
- Unified log view across compile/runtime.
- Parsed compiler diagnostics fields:
  - file
  - line
  - column
  - message
- Historical and structured search.
- Structured diff across runs.
- Log import (legacy sources).
- Promote findings to issue or immediate task.

### Properties
- Supports rule-based auto-issue creation from log patterns.
- Shared renderer with build-time log surfaces.
- Readability-focused (color, hierarchy, clarity).

---

## J) `cache` — AI Cache Layer

### Features
- Inspect temporary AI artifacts.
- Prompt input traceability.
- Intermediate context storage visibility.

### Properties
- Metadata-rich cache (timestamps, ownership/process, context linkage).
- Useful for auditing and diagnosing AI pipeline behavior.

---

## K) `track-cache` — Internal Utility-AI Cache

### Features
- Dedicated cache channel for internal utility AI operations.

### Properties
- Separates:
  - project-progress AI work
  - internal preprocessing/tool AI work
- Diagnostic lens for internal correctness, hygiene, and anomalies.

---

## L) `ops` — Operations Automation & Simulation

### Features
- Deterministic runbook simulation runs.
- Workflow simulation runs.
- Hybrid runbook/workflow simulation pathing.
- Full intermediate-step capture.
- Simulation result surfacing (validation + improvement feedback).

### Properties
- Pre-implementation discovery layer (requirements/protocol gaps).
- Can reduce downstream model complexity/cost by clarifying system behavior early.
- Enables early optimization and design hardening before coding.

---

## M) `discuss` — User-Facing AI Conversation Management

### Features
- Thread list/filter by type/topic/date.
- Rich rendering:
  - tool traces
  - highlighted code
  - colorized diffs
- Cross-reference display (linked track/phase/task/work context).

### Properties
- Scope: user-facing conversational threads (not all automation internals).
- Shared UI model with work-chat experiences.
- Optional internal AI debug-thread exposure via explicit developer mode.

---

## N) `settings` — Configuration Control Surface

### Features
- Named settings profiles.
- Global + project-scoped settings.
- First-run wizard.
- Registry-style low-level editor (path/value tree).

### Properties
- “Wizard + advanced registry view first” strategy.
- Extensible schema acknowledging evolving requirements.

---

## O) `issues` — Repository-Native Issue Tracking

### Features
- GitHub-like issue lifecycle in repository-native mode.
- Add/triage/analyze/decide/fix/resolve/ignore flows.
- Link issues to tasks/work execution.
- Category views (build/runtime/convention/ux/product/features/etc.).
- Dependency links between issues.

### Properties
- Supports co-location of issue data and code history in Git.
- Log/build/runtime findings can be converted to issues quickly.
- AI-assisted issue drafting:
  - text synthesis
  - references
  - type/severity proposals
- Reproducibility metadata expected (args/settings/steps).

---

## P) `solutions` — Known-Fix Pattern Library

### Features
- Known issue pattern registry.
- Pattern match from new failures to known fixes.
- Automatic or semi-automatic remediation hooks.

### Properties
- Works for both build and runtime failure classes.
- Especially effective for recurring AI-generated framework pitfalls.
- Reduces repetitive triage and accelerates stabilization.

---

## Q) `ai` — Backend/Model Orchestration Helpers

### Features
- Multi-backend management (cloud + local + legacy modes).
- Connectivity/auth/session/quota checks.
- Backend test/status diagnostics.
- Strategy-based model routing by task complexity/cost.
- Remote mirror workers via SSH + Maestro presence.
- Git-sync-aware remote execution flow.

### Properties
- Capacity-aware scheduling and pre-flight warnings.
- Prevents long tasks from starting when quota exhaustion risk is high.
- Operational focus on reliability and cost governance.

---

## R) `work` — AI Task Execution Sessions

### Features
- Start work by any/track/phase/issue/task.
- Execution trace:
  - file edits
  - operation type (diff/overwrite/etc.)
  - shell/tool calls
- Dual view:
  - real-time conversation control
  - operational event listing/timeline
- Analyze/fix/subwork/resume flows.

### Properties
- Supports long autonomous windows and supervised workflows.
- “Checkpoint/breadcrumb” navigation model for long sessions.
- Built for practical task throughput with auditable action traces.

---

## S) `wsession` — Backend Session Observability

### Features
- Backend session list/show/tree/timeline/stats.
- Breadcrumb management (including structured breadcrumbs).

### Properties
- Multiple backend sessions may serve one task (escalation/retry pattern).
- Tracks backend continuation across repeated CLI invocations.
- Primary diagnostic locus for backend-level faults:
  - sandbox/permission issues
  - process/protocol anomalies
  - integration-layer errors

---

## T) `tu` — Translation Unit / AST Tooling

### Features
- TU build/index/info/query/references.
- AST print and transform operations.
- Convention transformation pipelines.
- LSP server integration entry point.
- Draft generation support.

### Properties
- Structural refactoring core:
  - symbol location -> full reference set -> atomic rename/change
- Works across variable/function/class/namespace layers.
- Preferred over text-editing for large fork/integration work.
- Can be AI-driven by direct tools or JSON-command mediation.
- Supports pre-conversion normalization before language/framework migration.

---

## U) `convert` — Cross-Project Conversion Pipelines

### Features
- Conversion pipeline lifecycle:
  - add/new
  - plan
  - run
  - status
  - show
  - reset
  - batch
- Bridges source and target Maestro contexts.

### Properties
- Often requires source-side Maestro bootstrap + TU availability.
- Order-sensitive with explicit strategy choices, e.g.:
  - always-green builds
  - temporary breakage tolerance
  - low-cost bulk conversion first
  - fallback for non-compiling legacy inputs
- Supports conversion from nontraditional artifacts in exploratory modes.

---

## V) `ux` — AI-Assisted UX Evaluation

### Features
- Blindfold-style UX eval runs.
- Postmortem-to-issues/workgraph conversion.
- Eval run listing and summaries.

### Properties
- Can use framework accessibility/state interfaces instead of computer vision.
- Supports AI-driven goal execution against GUI/CLI surfaces.
- Allows task-attached helper templates/context extensions for testability.
- Produces measurable UX quality signals (e.g., stuck rate, completion success).
- Enables pre-release regression suites with reduced manual testing load.

---

## W) `tutorial` — First-Run / Landing / README Layer

### Features
- Interactive tutorial navigation by name/number/page.
- Introductory command guidance path.

### Properties
- Onboarding entry point for both humans and AI agents.
- May include project-specific overlays:
  - conventions
  - code of conduct
  - structural orientation
  - high-level goals/messages
- Functions as CLI-native homepage/first-run/readme hybrid.

---

## X) Cross-Cutting Platform Properties

### X1. Auditability
- Strong preference for recorded steps, resumable threads, and trace links.

### X2. AI-Human Collaboration Model
- Conversational control and operational execution are distinct but interoperable surfaces.

### X3. Determinism vs Throughput Strategy
- Supports strict deterministic workflows and pragmatic high-throughput modes.

### X4. Cost-Aware Intelligence Allocation
- Model/backend selection is strategy-driven by complexity, confidence, and quota.

### X5. Legacy-to-Modern Migration Capability
- Reverse engineering, normalization, and staged conversion are first-class capabilities.
