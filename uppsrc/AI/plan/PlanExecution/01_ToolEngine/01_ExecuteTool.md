# Task: Tool Execution Engine
# Status: DONE

## Objective
Implement a robust engine to execute tools requested by the AI (e.g., `run_shell_command`, `write_file`).

## Requirements
- Handle `tool_use` events from `CliMaestroEngine`.
- Execute shell commands using `LocalProcess`.
- Provide `tool_result` back to the engine.
- Support for "Human-in-the-loop" approval.
