# ai-upp roadmap/task -> Maestro import report

Date: 2026-01-05

## Source inventory
- Roadmap files: 6
- Task files (excluding README.md, TODO.md): 14
- Topic basenames (union): 15
- Paired roadmap+task topics: 5 (animedit, GameEngine, PacketRouter, uppstd, uppts)
- Roadmap-only topics: 1 (uxide)
- Task-only topics: 9 (EonPatternIntegration, GameEngineDocumentation, GameEngine_updated, HalIntegration, LibgdxComparison, ShaderEditor, Vfs, VfsShell, stdsrc)

## Maestro results (post-import)
- Tracks: 15
- Phases: 109 (from `maestro phase list`)
- Tasks: 481 (from `maestro task list`)

## Verification commands run
- `python /home/sblo/Dev/Maestro/maestro.py track list`
- `python /home/sblo/Dev/Maestro/maestro.py phase list`
- `python /home/sblo/Dev/Maestro/maestro.py task list`
- Per-track spot checks (phase + task lists):
  - `python /home/sblo/Dev/Maestro/maestro.py phase list animedit`
  - `python /home/sblo/Dev/Maestro/maestro.py task list v0-foundations`
  - `python /home/sblo/Dev/Maestro/maestro.py phase list gameengine`
  - `python /home/sblo/Dev/Maestro/maestro.py task list gameengine-p1`
  - `python /home/sblo/Dev/Maestro/maestro.py phase list packetrouter`
  - `python /home/sblo/Dev/Maestro/maestro.py task list packetrouter-core`
  - `python /home/sblo/Dev/Maestro/maestro.py phase list uppstd`
  - `python /home/sblo/Dev/Maestro/maestro.py task list uppstd-p1`
  - `python /home/sblo/Dev/Maestro/maestro.py phase list uppts`
  - `python /home/sblo/Dev/Maestro/maestro.py task list project`
  - `python /home/sblo/Dev/Maestro/maestro.py phase list uxide`
  - `python /home/sblo/Dev/Maestro/maestro.py task list version`

Note: `maestro track show <id>` fails with `Error: docs/todo.md not found.`

## Legacy cleanup
- Removed tracks: `roadmap-root`, `task-root`, `task-notes`
- Removed phases: `roadmap-root-root`, `task-root-root`, `task-notes-root`
- Archived previous JSON artifacts to: `docs/maestro/_import_backup_20260105`

## Ambiguities / parsing notes
- Some roadmap files use repeated phase-like headings (e.g., animedit), resulting in duplicated phase variants (e.g., `v0`, `v01`, `v0-foundations`). Review and prune if needed.
- Some sections are narrative with "Goal" or "Deliverables" lists rather than checkboxes; these were converted into tasks for `uxide`.
- Phase IDs with punctuation (e.g., "I/O") required explicit IDs; repaired manually during import.
