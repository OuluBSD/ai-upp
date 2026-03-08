# Task: Runbook Automation
# Status: DONE

## Objective
Allow the AI to semi-autonomously execute runbook steps.

## Requirements
- Add "Execute Step" button to Runbook detail view.
- When clicked, inject the step's command/action into the chat.
- Parse the AI's response to check if the step's "Expected" result was met.
- Auto-advance to the next step if successful (in Auto-Continue mode).
