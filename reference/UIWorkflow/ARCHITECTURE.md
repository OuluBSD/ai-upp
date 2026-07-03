# UI Workflow Architecture

## Executive Summary

UI Workflow is a systematic approach to capturing, documenting, and validating user interaction flows in ai-upp. It bridges the gap between abstract UI specifications (`.form` files and layout definitions) and concrete mockup snapshots that represent specific application states. The system enables agent-driven UI work to maintain fidelity over time by making intended user flows explicit through a three-layer architecture: UI specifications at the base, mockup snapshots as partial state overlays, and workflows as connections between snapshots. Mechanical usability-load metrics provide deterministic feedback on interaction complexity without requiring AI evaluation, while optional AI advisory notes can augment human decision-making.

## Source Paths Inspected

Investigation conducted across the repository identified the following categories of relevant code:

**Existing U++ Form and Layout Infrastructure:**
- `uppsrc/Form/Examples/` — various `.form` example files (Calc-Buttons.form, ComplexFormTest.form, HelloWorld.form, etc.)
- `uppsrc/FormEditor/` — form editor package with FormEdit.lay layout
- `uppsrc/CtrlLib/Ctrl.lay`, `uppsrc/CodeEditor/CodeEditor.lay`, `uppsrc/AnimEdit/NewSpriteDlg.lay` — layout files using `.lay` format
- `uppsrc/Docking/Docking.lay` — docking system layout

**Docking and Window Management:**
- `uppsrc/Docking/` — comprehensive docking framework with DockWindow, DockPane, DockMenu, DockableCtrl, DockTabBar, DockCont classes
- `uppsrc/Docking/Docking.upp` — main docking package definition

**Visual State and Design Infrastructure:**
- `uppsrc/VisualStateModel/VisualStateModel.upp` — existing visual state model package
- `uppsrc/AnimEdit/` — animation and state editor with StateMachineEditorCtrl, TimelineCtrl, ToolBar usage
- `uppsrc/IDE/Designers/` — IDE designer infrastructure for various file types
- `uppsrc/IDE/Designer.h`, `uppsrc/IDE/AI/Designer.h` — designer base classes

**UI Control Infrastructure:**
- `uppsrc/art/RedBar/`, `uppsrc/art/BlueBar/` — ToolBar and MenuBar styling packages
- References to ArrayCtrl, TreeCtrl, MenuBar, ToolBar, Canvas, Paint throughout
- CtrlLayout macro used extensively in editors and dialogs

## Problem Statement

Agent-driven UI development in ai-upp risks degradation over time without concrete representations of intended workflows. While the repository has mature form files, layout editors, and docking systems, there is no unified model for:

1. Capturing partial snapshots of UI state at specific interaction points
2. Explicitly linking mockup snapshots to base specifications
3. Tracking user flows and step-by-step interaction sequences
4. Measuring interaction complexity and cognitive load
5. Verifying that workflows maintain coverage and minimize user friction

Existing approaches store UI definitions (`.form`, `.lay` files) but lack a layer to represent "this is the state when the user has opened menu X and clicked item Y." They also lack mechanical assessment of whether a workflow's step sequence imposes excessive cognitive or physical burden.

The UI Workflow system must integrate with existing form/layout infrastructure rather than replace it, staying consistent with U++ idioms and the repository's editor ecosystem.

## Core Concepts

**UI Specification** — A formal or semi-formal description of a user interface. In ai-upp, this is typically a `.form` file or `.lay` layout definition, but may also be prose description, wireframe, or linked external document. Serves as the baseline from which mockup snapshots derive.

**Form File** — A `.form` file in U++ format, as found in `uppsrc/Form/Examples/` or defined within packages. Declarative definition of controls, layout, data binding, and behavior. The ground truth for a given UI component.

**Mockup Snapshot** — A partial, point-in-time representation of application state overlaid on a UI specification. May depict "main menu open," "dialog with validation error," or "sidebar collapsed." Mockups are intentionally incomplete; they capture only the state details relevant to the interaction being studied.

**Overlay Element** — An annotation, label, hotspot, or temporary control rendered on top of a base form or specification to indicate interaction state or user focus. Examples include highlighted buttons, floating labels, error badges, or transient tooltips.

**Anchor** — A named location or element within the base UI specification that serves as a stable reference point for overlay elements. Allows mockup snapshots to be robust to minor layout changes in the base form.

**Hotspot** — An interactive region within a mockup snapshot that represents a clickable, typable, or otherwise actionable element. Links to the next workflow step or external action. May differ from the base form's controls in affordance or availability during the snapshot's state.

**Workflow Catalog** — A collection of workflow definitions for a single application or module, stored in durable format (e.g., JSON, structured text). Serves as the reference for all captured workflows.

**Workflow** — A named, ordered sequence of workflow steps representing a complete user task or interaction pattern. Example: "Open file dialog, type filename, press Enter, handle success/error." Workflows are the primary unit of UI design documentation.

**Workflow Step** — A single transition point in a workflow, typically corresponding to one mockup snapshot. The step specifies which snapshot is active, what visible choices are available, and what action the user takes next.

**Visible Choice** — An option or control explicitly shown in a mockup snapshot that the user may interact with. Must be rendered as a hotspot or overlay. Invisible or unavailable options do not contribute to visible choice count.

**Alternative Action** — An optional or unexpected user behavior at a workflow step (e.g., "press Escape instead of clicking OK"). Workflows may document alternative actions explicitly to ensure coverage, or leave them implicit if deterministic penalties for ignoring them are acceptable.

**Step Coverage** — The proportion of logical user intentions captured by the workflow's steps. High coverage means the workflow documents most realistic paths; low coverage leaves significant workflows unexplored. Coverage is not necessarily 100% completeness; it is a measure of how much of the important design space is represented.

**Overlap Link** — An edge between two mockup snapshots indicating that they represent similar or overlapping states, or that one is a refinement of another. Used to identify redundancy and opportunities for consolidation.

**Target Travel Distance** — The visual distance (in pixels or normalized units) between the current focus/cursor location and the next interaction target. Measured within the mockup's viewport to assess physical effort required.

**Usability-Load Score** — A deterministic numerical metric quantifying the interaction complexity and cognitive burden of a single workflow step or entire workflow. Computed from visible-choice count, target travel distance, modality changes, mnemonic clarity, and label clarity. Higher score indicates higher load; thresholds trigger warnings.

**Deterministic Rule** — A mechanical, code-driven assessment with no dependence on AI evaluation. Examples: counting visible choices, measuring pixel distances, checking label uniqueness, verifying all hotspots have text. Deterministic rules are the foundation of pass/fail verification.

**AI Advisory Note** — An optional, human-readable comment generated by an AI system to highlight design concerns or suggest improvements. Examples: "Hotspot naming is ambiguous in this context," "Consider breaking this into two steps." AI notes are purely advisory; they do not cause pass/fail, and core verification never depends on them.

## Relationship To U++ Forms

ai-upp's existing form and layout system is mature:

- **`.form` files** define control trees, layout directives, and data bindings
- **`.lay` files** define CtrlLayout macros and visual positioning
- **FormEditor package** provides GUI tools to edit form definitions
- **Docking framework** manages window hierarchy, tabs, and persistence
- **CtrlLayout macro** bridges form declarations and C++ code

**UI Workflow builds orthogonally on top of forms:**

1. A workflow may reference a `.form` file as its base UI specification
2. Mockup snapshots do not modify the form file; they add overlay state on top
3. Workflows capture the interaction sequence; forms capture the static structure
4. Both forms and workflows benefit from shared metadata (element IDs, anchors, mnemonics)

**Integration points:**
- Mockup snapshots store references to form file paths or element IDs (anchors)
- Workflow editor may display a linked form as background reference
- Overlay hotspots may bind to existing form controls by name or ID
- Load metrics leverage form control properties (label text, icon presence, mnemonic availability)

## Mockup Snapshot Model

A mockup snapshot captures a specific application state without modification to the underlying form:

**Data Structure:**
- `id` — Unique identifier within the workflow catalog
- `name` — Human-readable title (e.g., "main-menu-open")
- `description` — Optional prose description of what this state represents
- `base_form_path` — Path or reference to the linked `.form` or UI spec file (optional)
- `base_form_element_id` — If the form is a component, the ID of the root element (optional)
- `overlays` — List of overlay elements (annotations, labels, hotspots, state indicators)
- `metadata` — Additional state flags (modal dialog, full-screen, docked window, etc.)

**Overlay Elements:**
Each overlay specifies:
- `type` — "label," "hotspot," "badge," "highlight," "transient-control" (example types)
- `anchor` — Reference to a form element or named location
- `bounds` — Optional pixel-space offset or size (if not anchored)
- `text` — Display text or content
- `mnemonic` — Keyboard shortcut or access key (if applicable)
- `state` — "enabled," "disabled," "hidden," "error," etc.

**Partial Snapshots:**
Mockups need not be complete. A snapshot showing only the main menu open is valid, even if the toolbar, status bar, and most dialogs are not annotated. This is intentional: partial snapshots reduce maintenance burden and isolate the relevant state.

## Workflow Catalog Model

A workflow catalog is a durable, machine-readable collection of workflows:

**File Format:**
- Preferred: JSON with schema validation (human-readable, agent-friendly)
- Alternative: TOON or U++ native format if established patterns exist
- Must support diffs and mechanical agent edits

**Catalog Structure:**
```
catalog:
  name: "application-name or module-name"
  version: "1.0"
  base_specs: ["path/to/form1.form", "path/to/form2.form"]
  workflows: [
    {
      id: "workflow-1",
      name: "Open File",
      description: "Canonical flow to open a file",
      steps: [ ... ]
    },
    ...
  ]
  mockups: [
    {
      id: "mockup-1",
      name: "main-menu-open",
      base_form_path: "app-main.form",
      overlays: [ ... ]
    },
    ...
  ]
```

**Workflow Step Model:**
```
step:
  sequence: 1
  mockup_id: "mockup-1"
  description: "User sees main menu open"
  visible_choices: [
    { hotspot_id: "file-menu", label: "File", mnemonic: "Alt+F" },
    { hotspot_id: "edit-menu", label: "Edit", mnemonic: "Alt+E" },
    ...
  ]
  primary_action: { hotspot_id: "file-menu", action: "click" }
  alternative_actions: [
    { hotspot_id: "help-button", action: "click", description: "Get help" }
  ]
  next_step_on_success: 2
  next_step_on_alternative: [
    { condition: "help-requested", step: 5 },
    ...
  ]
  load_metrics: { ... }
```

## Mechanical Usability Load

Usability-load scoring is deterministic and based on structured data, never requiring AI evaluation at the core level.

**Load Sources (with deterministic rules):**

1. **Visible-Choice Overload:**
   - Count distinct visible choices at a step
   - Rule: score increases by 1 point per choice above a threshold (e.g., 7 choices = 7, 10 choices = 10)
   - Rationale: cognitive load increases with decision complexity

2. **Target Travel Distance:**
   - Compute distance in pixels from last cursor/focus location to next target
   - Rule: distance >= 200px → +2 load points; >= 400px → +4 points; etc.
   - Rationale: large mouse movements increase physical effort

3. **Modality Shifts:**
   - Count changes in interaction mode (menu → dialog → CLI input → form field)
   - Rule: +3 points per modality change within a workflow
   - Rationale: context switching imposes cognitive load

4. **Label Clarity:**
   - Check for missing labels or duplicated labels on visible choices
   - Rule: missing label → +2 points; duplicate label → +1 point per duplicate
   - Rationale: unclear targets increase error risk

5. **Mnemonic Coverage:**
   - For menu and toolbar choices, count availability and clarity of shortcuts
   - Rule: choice without mnemonic in context where others have one → +1 point
   - Rationale: inconsistent shortcuts increase memory burden

6. **Required Memory:**
   - Flag targets that are not visible in the current mockup but required for next step
   - Rule: +2 points per required-but-invisible target
   - Rationale: working-memory load for targets not in view

**Step Load Score:**
- Aggregate all applicable rules for a single step
- Example: 8 visible choices (8) + 300px travel (2) + 1 modality shift (3) + 1 unclear label (2) = 15 points

**Workflow Load Score:**
- Sum step scores or compute mean, depending on use case
- Report critical hotspots (steps exceeding threshold, e.g., 12 points)

**Thresholds and Advisory:**
- Score 0–6: Low load (ideal)
- Score 7–12: Medium load (acceptable, monitor)
- Score 13+: High load (consider redesign)

**AI Advisory (Optional):**
After deterministic scoring, optional AI review may suggest:
- "Consider grouping these 12 menu items into submenus"
- "The 400px travel distance from toolbar to bottom panel suggests a split view"
- But such suggestions do not affect the core score or pass/fail status.

## Workflow Verification

Verification ensures workflows are complete, consistent, and correctly linked to specifications.

**Deterministic Checks:**
1. **Dangling References:** All mockup IDs, hotspot IDs, and form paths referenced in workflows must exist
2. **Unreachable Steps:** No workflow step should be orphaned (not reachable from any entry point)
3. **Cycle Detection:** Workflows may have loops, but report them explicitly for review
4. **Label Uniqueness:** Within a single step, hotspot labels must be unique or clearly disambiguated
5. **Anchor Resolution:** All overlay anchors must resolve to valid form elements or explicitly defined coordinates
6. **Coverage Gaps:** Identify mockups that are created but not used in any workflow (dead code)

**Optional Verification (AI or Human Review):**
- Is the step description clear and testable?
- Does the workflow represent a realistic user task?
- Are alternative actions documented or explicitly acknowledged as uncovered?

**Verification Output:**
```
verification_report:
  workflow_id: "workflow-1"
  status: "PASS" | "WARN" | "FAIL"
  errors: [ ... ]  // deterministic failures
  warnings: [ ... ] // e.g., high load, low coverage
  advisory_notes: [ ... ] // optional AI feedback
```

## Editor Strategy

UI Workflow editing has two phases:

**Phase 1 (Headless Core):**
- Implement headless data model, file I/O, and metric calculation
- Enable CLI tools and mechanical verification
- Allow agents and scripts to read/write catalogs without GUI

**Phase 2 (GUI Editors):**
- Add mockup snapshot editor (canvas + overlay property panel)
- Add workflow graph editor (node-and-edge visualization)
- Add load report view (bar charts, step-by-step breakdown)
- Integrate with existing Docking framework for window management
- Optional integration with FormEditor or TheIDE to display linked forms

**Design Patterns:**
- Reuse CtrlLayout macro for editor dialogs
- Follow Docking framework conventions (DockableCtrl, DockPane, persistence)
- Use ArrayCtrl or custom canvas for list views and visualization
- Leverage existing VisualStateModel patterns if applicable

## Agent Development Loop

Agents interact with the UI Workflow system primarily through:

1. **Read Workflow Catalog:** Agent loads a catalog from disk (JSON or similar)
2. **Propose Modifications:** Agent suggests new mockups, steps, or alternative actions
3. **Mechanical Verification:** Deterministic checks validate the proposal (no AI loop)
4. **Write Catalog:** Agent writes changes back to file in durable format
5. **Optional Feedback:** Human or AI review examines optional advisory notes

**Contracts:**
- Agents must preserve all deterministic rule violations; they are hard stops
- Agents may ignore optional AI advisory notes if they prefer alternative solutions
- Agents should maintain version and attribution metadata in catalogs

## Proposed Package Boundaries

**UIWorkflowCore** (`uppsrc/UIWorkflowCore`)
- Headless data model for mockup snapshots, workflows, and catalogs
- File I/O (JSON parsing, serialization)
- Deterministic metric calculation and verification
- No GUI dependencies; minimal external dependencies
- Exports: Mockup, Workflow, Catalog, VerificationReport classes and functions

**UIWorkflow** (`uppsrc/UIWorkflow`)
- GUI package for editing and visualizing workflows
- Depends on UIWorkflowCore, CtrlLib, Docking, and optionally FormEditor
- Provides DockableCtrl subclasses for mockup editor, workflow graph editor, load report view
- Integrates with TheIDE Designer infrastructure if applicable
- Exports: MockupEditorCtrl, WorkflowGraphCtrl, LoadReportCtrl, and supporting classes

**UIWorkflowTests** (`uppsrc/UIWorkflowTests`)
- Headless unit tests, integration tests, and sample catalogs
- No GUI code; depends on UIWorkflowCore only
- Provides example workflows and selftest harness
- Validates file I/O, metric calculation, and verification logic

**Integration Points (Optional):**
- **FormEditor:** Mockup editor may import form definitions for overlay anchoring
- **TheIDE:** UI Workflow editors may register with IDE Designer registry
- **VisualStateModel:** Catalog structure may align with existing visual-state metadata if beneficial

## File Formats

**Mockup Snapshot (JSON):**
```json
{
  "id": "mockup-main-menu-open",
  "name": "Main Menu Open",
  "description": "User has clicked File menu",
  "base_form_path": "app-main.form",
  "overlays": [
    {
      "type": "hotspot",
      "anchor": "file-menu",
      "label": "File",
      "mnemonic": "Alt+F",
      "state": "highlighted"
    }
  ],
  "metadata": { "modal": false }
}
```

**Workflow Catalog (JSON):**
```json
{
  "name": "application",
  "version": "1.0",
  "workflows": [
    {
      "id": "open-file",
      "name": "Open File",
      "steps": [
        {
          "sequence": 1,
          "mockup_id": "mockup-main-menu-open",
          "visible_choices": [ ... ],
          "load_metrics": { "score": 4 }
        }
      ]
    }
  ],
  "mockups": [ ... ]
}
```

Alternative formats (TOON, XML) may be supported if existing ai-upp conventions mandate them, but JSON is preferred for agent editability and tooling.

## Build And Test Strategy

**Compilation:**
- UIWorkflowCore and UIWorkflow packages compile via standard U++ build (`bin/build.exe`)
- Headers expose public APIs for data model and file I/O
- Optional GUI code compiles only if CtrlLib and Docking are available

**Testing:**
- UIWorkflowTests package provides selftest binary via CONSOLE_APP_MAIN
- Selftest loads example catalogs, verifies metrics, and runs deterministic checks
- Automated tests cover file I/O round-trips, metric calculation, and coverage verification

**Regression Gates (Optional, Task 0014):**
- CI pipeline may run selftest binary to detect regressions
- Catalogs in reference/ may be version-controlled and diffed for breaking changes
- Example workflows serve as golden-standard regression tests

## Risks And Open Questions

**Risks:**
1. **Scope Creep:** UI Workflow could expand to replace form editors or become an alternative design tool; must stay focused on workflow capture and verification
2. **Agent Drift:** Without periodic validation, agent-edited catalogs may diverge from reality; mechanical verification mitigates but does not eliminate
3. **Incomplete Snapshots:** Partial mockups are a feature, but may tempt incomplete workflows; clear guidelines on coverage are needed
4. **Metric Gaming:** Designers may argue against load metrics; communication on why thresholds exist is essential
5. **Integration Burden:** Linking to form files introduces dependency; must gracefully handle missing or malformed forms

**Open Questions:**
1. Should mockup snapshots support visual screenshots (PNG) in addition to overlay markup? (Current design is overlay-based; screenshots may come later)
2. How should version control and diff visibility be maintained for large catalogs? (JSON is friendly to diffs; consider one-workflow-per-file if needed)
3. Should TheIDE or a separate application host the UI Workflow editors? (Plan is separate UIWorkflow package; integration point is optional)
4. How much of load-metric scoring should be customizable per project? (Design assumes fixed rules; per-project config may be added later)
5. Should workflows support branching based on user data (e.g., "if file is unsaved, show save dialog")? (Current model is control-flow-based; data-flow is future work)

## Recommended Implementation Tasks

The following tasks define the recommended sequence for building UI Workflow support in ai-upp. These tasks are already defined in `plan/UI-Workflow/`:

**Foundation Phase (0002–0005):**
- **Task 0002:** Specify UI Workflow file formats — Define stable JSON or TOON-based formats for catalogs and snapshots
- **Task 0003:** Add headless UI Workflow core data model — Implement headless mockup, workflow, and catalog classes
- **Task 0004:** Implement mockup form overlay model — Add overlay element types, anchoring, and binding logic
- **Task 0005:** Implement workflow catalog and step model — Complete catalog loading, step sequencing, and reference resolution

**Measurement Phase (0006–0008):**
- **Task 0006:** Implement deterministic usability-load metrics — Implement load-score calculation (visible choices, travel distance, modality, labels, mnemonics)
- **Task 0007:** Add workflow and mockup coverage verifier — Implement verification engine for dangling references, cycles, coverage gaps, and label uniqueness
- **Task 0008:** Add CLI mnemonic registry and scoring — Build registry of keyboard shortcuts and scoring logic for mnemonic clarity

**Editors Phase (0009–0011):**
- **Task 0009:** Add mockup editor skeleton — Create GUI skeleton with canvas, overlay property panel, and docking integration
- **Task 0010:** Add workflow graph editor skeleton — Implement node-and-edge visualization of workflow steps
- **Task 0011:** Add usability-load report view — Render load metrics as bar charts, step tables, and threshold warnings

**Agent Loop Phase (0012–0014):**
- **Task 0012:** Define agent UI workflow update contract — Specify API and serialization contracts for agent-driven modifications
- **Task 0013:** Add UI Workflow selftest and sample catalog — Create example workflows and automated selftest harness
- **Task 0014:** Add optional UI Workflow regression gate — Implement CI integration and version-control gates for workflow catalogs

This sequence ensures that headless verification and file I/O are in place before GUI tools are added, and that agents can operate independently of the GUI editors.
