# Task: Agent Subprocess Management
# Status: TODO

## Objective
Implement the logic to launch and manage AI coding agent processes within a PTY (Pseudo-Terminal).

## Requirements
- Use `LocalProcess` or similar U++ primitives to spawn agent processes.
- Allocate a PTY for the child process to ensure it behaves as if in a real terminal.
- Implement lifecycle management (start, stop, restart, monitoring).
- Capture process exit codes and signals.
