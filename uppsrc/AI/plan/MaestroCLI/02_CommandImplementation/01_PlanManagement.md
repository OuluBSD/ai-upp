# Task: Plan Management
# Status: TODO

## Objective
Implement the `plan` subcommand to list, show, and manage plans using `PlanParser`, matching the Python implementation's surface area.

## Requirements
- Link `MaestroCLI` with `Maestro` package.
- Use `PlanParser` to load plans from `uppsrc/AI/plan`.
- Implement `plan list (ls)`: Show all plans and their status.
- Implement `plan show (sh) <name>`: Show details of a specific plan.
- Implement `plan add (a) <title>`: Create a new plan file.
- Implement `plan remove (rm) <name>`: Delete a plan.
- Implement `plan add-item (ai) <name> <text>`: Add an item to a plan.
- Implement `plan remove-item (ri) <name> <index>`: Remove an item.
- Implement `plan discuss (d) <name> [prompt]`: AI-assisted plan editing.
