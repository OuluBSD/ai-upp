# Task: Keyboard Event Mapping and Injection
# Status: TODO

## Objective
Implement high-level actions that translate to character-level events sent to the agent's PTY.

## Requirements
- Define `KeyMap` to translate framework-level events to PTY-compatible character sequences (e.g., arrow keys, Ctrl+C).
- Implement a `type(string)` method for injecting character sequences at once.
- Handle special keys like Escape, Enter, Tab, and Backspace correctly.
- Ensure events are sent in a way that respects the agent's input buffer.
