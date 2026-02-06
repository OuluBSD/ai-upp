# MaestroHub User Flow Analysis

## 1. The "Fix-It" Loop (Log to Issue to Fix)
1. **Detection**: Open `Intelligence Hub` -> `Log Analyzer`. Scan logs and select a finding.
2. **Issue Creation**: Click "Create Issue" in Log Analyzer. The `IssueCreateDialog` opens with pre-filled context.
3. **Triage**: Open `Issue Tracker`. Click "Triage Wizard". 
4. **AI Proposal**: AI Assistant analyzes the issue and proposes a fix (visible in the Triage UI).
5. **Acceptance**: User clicks "Accept Fix". This triggers an `Evidence Collection` task.
6. **Verification**: User opens `Evidence Locker` to review the verification report (PDF/RichText).

**Friction Analysis**: 
- *Current*: Switching between Intelligence Hub and Issue Tracker is 1 click (tabs). 
- *Optimization*: Added "Quick Triage" button directly in Log Analyzer finding detail.

## 2. The "Discovery" Flow (New Repository Onboarding)
1. **Entry**: Open MaestroHub. Select a root directory.
2. **Initialization**: If not a Maestro project, the `InitDialog` appears.
3. **Scan**: User switches to `Workspace` -> `Pipeline` to monitor the initial repository scan.
4. **Deep Dive**: User opens `Intelligence Hub` -> `TU Browser`.
5. **Understanding**: User searches for a symbol (e.g., "Main") to see the dependency graph and "included-by" relationships.

## 3. Context Switching (Work Session vs. Triage)
1. **Scenario**: User is middle of a `Work Session` (Work Pane) but a critical issue is reported.
2. **Action**: User clicks `Issue Tracker` tab.
3. **State Preservation**: The `Work Pane` preserves the active session ID. The AI Assistant's context stack shows the jump.
4. **Return**: User clicks back to `Work Pane`. The context is restored via the `assistant->UpdateContext` hook.

## 4. The "Strategy" Flow (Playbook Application)
1. **Problem**: A complex conversion task requires a specific architectural pattern.
2. **Action**: User opens `Strategy Playbooks`.
3. **Selection**: User browses "Migration" category and selects "U++ Porting Strategy".
4. **Application**: Click "Apply to Current Task". The AI Assistant receives the playbook principles as system constraints.