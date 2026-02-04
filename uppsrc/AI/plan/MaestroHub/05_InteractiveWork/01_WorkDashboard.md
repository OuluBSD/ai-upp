# Task: Interactive Work Dashboard
# Status: TODO

## Objective
Create a dashboard for monitoring and guiding the autonomous AI worker.

## Requirements
- Display current `WorkItem` (Task/Issue) details.
- Show the AI's generated plan (steps).
- Stream execution logs/breadcrumbs in real-time.
- Provide "Approve", "Reject", "Modify Plan" controls.
- Integrate with `WorkManager::StartWorkSession` (but running in a background thread/GUI mode).
