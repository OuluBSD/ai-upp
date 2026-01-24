# Task: Automatic Plan Injection
# Status: DONE

## Objective
Automatically inject the current project plan (from `PlanParser`) into the system prompt of every new AI session.

## Requirements
- Use `PlanParser` to load the current state of tracks/phases/tasks.
- Format the plan as a concise Markdown summary.
- Prepend this summary to the first user message or system prompt.
- Ensure the AI understands its "Current Task" based on `cookie.txt`.
