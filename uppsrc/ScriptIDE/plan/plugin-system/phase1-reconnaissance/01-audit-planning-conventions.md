# Task: Audit Existing Planning Conventions

## Goal
Inspect and summarize the existing ScriptIDE planning conventions to ensure new tasks align with the project's standards.

## Background / Rationale
To maintain a cohesive and structured approach to project management, any new planning documents must mimic the established patterns in `uppsrc/ScriptIDE/plan/`. This ensures consistency for contributors and automated agents.

## Scope
- Reviewing markdown files under `uppsrc/ScriptIDE/plan/`.
- Cataloging task structures, phase organization, naming schemes, and granularity.

## Non-goals
- Modifying existing planning files.
- Implementing any features.

## Dependencies
- Access to the `uppsrc/ScriptIDE/plan/` directory.

## Concrete Investigation Steps
1. Read existing `.md` files in the plan directory.
2. Note the heading structure (e.g., Goal, Background, Strategy, Success Criteria).
3. Identify how phases and tasks are named (e.g., `01-foundation`, `phase1-architecture`).
4. Document the expected level of detail and terminology used.

## Affected Subsystems
- Documentation / Planning Tree

## Implementation Direction
N/A - This is purely an investigative planning task.

## Risks
- Misinterpreting the existing conventions leading to disjointed new planning tasks.

## Acceptance Criteria
- [ ] A summary document or mental model is created outlining the required heading structure.
- [ ] Phase and task naming conventions are clearly documented.
- [ ] Granularity and dependency linking styles are understood and applied to future tasks.
