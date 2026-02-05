# AI-Assisted UX Evaluation Monitor Design (WinXP Style)

## 1. "UX Performance Monitor" Window
- **Style**: Standard window with a "Task Pane" on the left (common in WinXP Explorer).
- **Task Pane**: Common actions: "Run New Eval", "Export Report", "Clear History".
- **Main View**: Tabbed interface.

## 2. Tab 1: "Evaluation History" (ListView)
- **Control**: ArrayCtrl with columns: [Run ID, Date, Goal, Status, Score].
- **Icons**: Green checkmark for success, Yellow triangle for friction, Red cross for failure.
- **Context Menu**: "Replay Session", "Create Issue from Failure", "Compare with Run...".

## 3. Tab 2: "Step-by-Step Replay"
- **Style**: Master-detail.
- **Top (Event List)**: ArrayCtrl showing AI-User actions:
  - `00:01`: Clicked [Analyze] Button.
  - `00:05`: Entered text "Refactor regex".
  - `00:10`: STUCK - Error dialog appeared.
- **Bottom (Context/Screenshot)**: Static placeholder area for a "Screenshot" or visual state dump of the GUI at that step.

## 4. Tab 3: "Quality Metrics" (Dashboard)
- **Style**: GroupBox-heavy overview.
- **GroupBox "Completion Stats"**:
  - Progress bar showing "Success Rate".
  - Label: "Average Time-to-Value: 45s".
- **GroupBox "Friction Hotspots"**:
  - ArrayCtrl listing the most "stuck" UI components (e.g. "Runbook Editor - Step 3").

## 5. "New Evaluation" Wizard
- **Style**: Classic 3-step wizard.
- **Step 1**: Select User Persona (DropList: Junior, Senior, Architect).
- **Step 2**: Define Goal (EditString: "Fix a build error using only the mouse").
- **Step 3**: Resource limits (EditIntSpin: "Max steps", "Timeout").