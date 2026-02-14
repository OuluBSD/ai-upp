# Task: Constraint System Plumbing
# Status: TODO

## Objective
Integrate the Logic-based constraint system (`AI/Logic`, `AI/LogicGui`) into `MaestroCLI`. This enables formal verification of project state and UI invariants from the command line.

## Requirements
- Link `MaestroCLI` with `AI/Logic` and `AI/LogicGui`.
- Implement `maestro check constraints` command.
- Support loading project-specific `.ugui` files from `docs/maestro/plans/constraints/`.
- Provide a `LogicVisitor` that can scan the repository/files and populate facts for the Logic Engine.
- Implement "Headless Assertions" that use the Logic Engine to verify non-GUI invariants (e.g., "Every .h file must have a matching .cpp").
