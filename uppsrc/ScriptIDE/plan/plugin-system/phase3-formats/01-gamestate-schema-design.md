# Task: `.gamestate` Schema Design

## Goal
Design the JSON schema for `.gamestate` files, which act as the main entry point for a game session or plugin-driven project.

## Background / Rationale
To bridge ScriptIDE, the Python VM, and custom plugin views (like a game board), we need a configuration file that dictates how these pieces connect. The `.gamestate` format fulfills this role, specifying the entry script, metadata, and required layouts.

## Scope
- Defining the JSON schema for `.gamestate`.
- Defining path resolution rules for relative paths.
- Defining required vs. optional fields.

## Non-goals
- Implementing the JSON parser (we use U++ Core JSON support).

## Dependencies
- `uppsrc/Core` JSON capabilities.

## Concrete Investigation Steps
1. Review U++ JSON parsing features to ensure the proposed schema is easily deserialized.
2. Determine how relative paths within the `.gamestate` file should be resolved (e.g., relative to the file itself or the workspace root).
3. Draft a sample `.gamestate` file for the Hearts project.

## Affected Subsystems
- Plugin System / File Type Handlers
- `.gamestate` parser logic

## Implementation Direction
Provide a formal schema and an example JSON document. E.g.:
```yaml
entry_script: "main.py"
entry_function: "start_game"
layout: "table.form"
metadata:
  players: 4
  ruleset: "standard"
```

## Risks
- Path resolution discrepancies could lead to broken game launches on different machines.

## Acceptance Criteria
- [ ] Documented JSON schema for `.gamestate`.
- [ ] Defined path resolution rules.
- [ ] Example `.gamestate` file provided.
