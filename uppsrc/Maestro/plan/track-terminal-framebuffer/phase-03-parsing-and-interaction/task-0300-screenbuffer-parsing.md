# Task: Screenbuffer Parsing to Structured Classes
# Status: TODO

## Objective
Implement a parser that analyzes the `TerminalBuffer` and extracts structured information.

## Requirements
- Identify common patterns in coding agent output (file paths, diffs, tool calls).
- Map the spatial 2D grid of the buffer to logical objects (e.g., `CodeSnippet`, `CommandResult`).
- Support "watching" specific regions of the screen for updates.
- Export parsed data into a format suitable for other AI agents to consume.
