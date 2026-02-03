# Task: AI Discussion (discuss)
# Status: IN_PROGRESS

## Objective
Implement an interactive AI discussion command that supports project context and autonomous tool execution.

## Requirements
- Link with `AI/Engine` (done via `Maestro` package).
- Use `CliMaestroEngine` to wrap `gemini` or `qwen` CLI.
- Implement an interactive loop (`discuss` mode) or single prompt execution.
- Inject project context (summary of tracks/phases/tasks).
- Handle tool calls from the AI (e.g., `update_task_status`).
- Support `-s SESSION` to resume or save interactions.
