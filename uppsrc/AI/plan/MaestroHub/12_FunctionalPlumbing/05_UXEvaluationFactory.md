# Task: AI-Assisted UX Evaluation Factory

# Status: TODO

# Description
Implement the "Blindfold-style" UX evaluation system where AI agents test the GUI/CLI for friction and "stuck" states.

# Objectives
- Implement a `UXEvalPane` for configuring and running evaluation sessions.
- Support "Goal Execution": define a goal (e.g., "Create a new issue and fix it") and let AI attempt it.
- Use accessibility/state interfaces instead of computer vision for faster, deterministic evaluation.
- Produce "UX Quality Signals": stuck rate, completion success, click-depth.

# Deliverables
- Integration of eval results into the `Issue Tracker` as "UX Friction" findings.
