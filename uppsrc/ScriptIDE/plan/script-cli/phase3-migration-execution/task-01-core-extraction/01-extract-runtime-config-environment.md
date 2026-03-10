# Task: Plan Extraction of Runtime, Config, and Environment to ScriptCommon

## Goal
Define the exact, low-risk migration sequence for moving non-GUI classes from `ScriptIDE` to `ScriptCommon`.

## Background / Rationale
Extraction must preserve behavior while avoiding merge conflicts with ongoing ScriptIDE work.

## Scope
- Plan movement of `RunManager`, `Linter`, `PathManager`, and `IDESettings`.
- Plan include/path updates and namespace consistency.
- Plan staged compile checkpoints.

## Non-goals
- Moving plugin GUI classes.
- Functional redesign of runtime behavior.

## Dependencies
- `ScriptCommon` file layout from phase2.
- Inventory and boundary audit from phase1.

## Concrete Investigation Steps
1. Create move order with smallest dependency fanout first.
2. For each file, define:
   - source path
   - target path
   - include changes
   - expected dependents to update
3. Define checkpoint builds after each move group.
4. Define rollback strategy per step.

## Affected Subsystems
- `uppsrc/ScriptIDE/RunManager.*`
- `uppsrc/ScriptIDE/Linter.*`
- `uppsrc/ScriptIDE/PathManager.*`
- `uppsrc/ScriptIDE/IDESettings.h`
- New `uppsrc/ScriptCommon/*`

## Implementation Direction
Proposed step order:
1. Move `IDESettings.h` to `ScriptCommon/IDESettings.h`.
2. Move `PathManager.*` to `ScriptCommon/`.
3. Move `Linter.*` and `RunManager.*` to `ScriptCommon/`.
4. Switch ScriptIDE includes to `#include <ScriptCommon/ScriptCommon.h>` and local adapter headers.
5. Remove migrated files from `ScriptIDE.upp`, add to `ScriptCommon.upp`.

## Risks
- Include churn may pull `ScriptIDE.h` into common code accidentally.
- Ongoing parallel edits in ScriptIDE may conflict during extraction.

## Acceptance Criteria
- [ ] Migration order is explicit and checkpointed.
- [ ] Each moved file has a target path and include-update plan.
- [ ] No moved file requires GUI headers after extraction.
