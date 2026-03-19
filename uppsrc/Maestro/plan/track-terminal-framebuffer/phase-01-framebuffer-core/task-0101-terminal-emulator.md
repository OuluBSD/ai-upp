# Task: Terminal Emulator Logic
# Status: TODO

## Objective
Implement a virtual terminal emulator that processes ANSI/VT100 escape sequences and updates the `TerminalBuffer`.

## Requirements
- Parse standard VT100/Xterm escape sequences (cursor movement, colors, clearing).
- Handle specialized sequences used by AI coding agents (e.g., status line updates).
- Implement a state machine for sequence parsing.
- Ensure the emulator correctly handles the "infinite" width/height of the buffer.
