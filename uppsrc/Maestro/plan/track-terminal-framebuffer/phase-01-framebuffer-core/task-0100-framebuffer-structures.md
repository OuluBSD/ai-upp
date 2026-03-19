# Task: Framebuffer Data Structures
# Status: TODO

## Objective
Implement a high-capacity terminal framebuffer that can store large amounts of character and attribute data without truncation.

## Requirements
- Define `TerminalBuffer` class to hold character cells (char, color, attributes).
- Support extremely large dimensions (e.g., thousands of columns and lines) to avoid "..." truncation.
- Implement efficient scrolling and region management.
- Provide thread-safe access to the buffer for concurrent reading (parser) and writing (emulator).
