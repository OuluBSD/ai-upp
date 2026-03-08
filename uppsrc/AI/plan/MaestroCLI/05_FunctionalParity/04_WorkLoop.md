# Task: Implement Autonomous Work Loop
# Status: DONE

## Objective
Implement the autonomous AI worker loop for task execution.

## Requirements
- Port `maestro/work/` logic.
- Implement task selection based on WorkGraph and dependencies.
- Support "any" work mode where AI prioritizes the frontier.
- Integrate with WorkSession for deep breadcrumb logging.
- Support subwork session management for complex branching tasks.
