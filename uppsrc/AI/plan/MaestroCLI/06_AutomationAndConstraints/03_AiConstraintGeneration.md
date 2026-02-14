# Task: AI Constraint Generation from Runbooks
# Status: TODO

## Objective
Automatically derive formal logic constraints from user-perspective runbooks using AI. This ensures that every manual scenario has a corresponding automated invariant check.

## Requirements
- Implement `maestro runbook derive-constraints <runbook_id>` command.
- Use Gemini to analyze runbook steps and "Expected" outcomes.
- Generate First-Order Logic (FOL) predicates corresponding to those outcomes.
- Map predicates to concrete UI paths or filesystem patterns (e.g., `Exists(Button("Save"))`).
- Output results to `.ugui` constraint files.
