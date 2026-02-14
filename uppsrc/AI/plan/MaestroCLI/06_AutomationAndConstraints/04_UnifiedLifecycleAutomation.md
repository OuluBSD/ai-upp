# Task: Unified Lifecycle Automation (Runbook-First)
# Status: TODO

## Objective
Implement a "Developer Emulation" script that follows the correct architectural sequence: Runbooks -> Constraints -> WorkGraphs.

## Requirements
- Create `script/drawing_app_lifecycle.py`.
- Sequence:
    1. `maestro init`
    2. `maestro runbook resolve` (Generate user scenarios).
    3. `maestro runbook derive-constraints` (Generate automated assertions).
    4. `maestro plan decompose` (Generate implementation plan).
    5. `maestro plan enact` (Set up tasks).
- Implement an iterative loop that handles build failures by updating constraints or plans.
- Emulate user feedback loop via `maestro discuss`.
