# Task: I/O Redirection and Piping
# Status: TODO

## Objective
Establish a robust bidirectional data flow between the AI coding agent's PTY and the terminal emulator.

## Requirements
- Connect the agent process's stdout/stderr to the `TerminalEmulator`.
- Buffer and process incoming byte streams as they arrive.
- Implement an async loop for reading from the agent and updating the `TerminalBuffer`.
- Map user keyboard events from the framework back to the agent's stdin.
