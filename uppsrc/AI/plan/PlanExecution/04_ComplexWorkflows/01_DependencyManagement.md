# Task: Dependency Management
# Status: TODO

## Objective
Implement dependency tracking for WorkGraph tasks.

## Requirements
- Allow tasks to define `depends_on` (list of Task IDs).
- Visualize dependencies in the Product pane (arrows or indentation).
- Prevent enactment of blocked tasks.
- Auto-enact dependent tasks when prerequisites are met.
