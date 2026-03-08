# Task: Operations Automation (ops)
# Status: DONE

## Objective
Port the operations automation and health check logic to C++.

## Requirements
- Implement `ops doctor`:
    - Check if project root is a valid Maestro repository.
    - Validate package dependency graph (detect missing or circular dependencies).
    - Check for `flagV1` convention violations.
    - Verify presence of required docs (`runbooks`, `workflows`, `issues`).
- Implement `ops run`:
    - Execute a deterministic runbook (no AI required).
    - Support parameter passing.
    - Record execution history in `docs/maestro/ops/runs/`.
- Implement `ops list` and `ops show`:
    - Browse and inspect operation run records.
