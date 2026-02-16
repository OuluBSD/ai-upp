# Playbook Test Harness v1

# Status: done

## Overview

The Playbook Test Harness v1 provides a deterministic, scriptable testing framework for AI Playbooks. It focuses on:

- End-to-end execution of playbooks (especially "safe_cleanup_cycle")
- Validation of safety constraints (max_risk, max_actions, allow_apply)
- Observing structural effects (telemetry, evolution, diffs) in controlled test workspaces
- Making it easy to run these tests from the command line and CI

The harness avoids GUI dependencies and relies only on the AI-UPP CLI (`--json`), ensuring deterministic behavior.

## Test Workspaces

The harness includes dedicated test workspaces under `tests/playbooks/workspaces/`:

### ws_basic_cleanup

A minimal U++ workspace with:
- A couple of packages (e.g. "TestPkg")
- A .upp file with a few .cpp/.h files
- Some intentionally "messy" code:
  - Unused includes
  - Simple functions with trivial complexity

**Purpose**: Validate that "safe_cleanup_cycle" performs small, safe cleanups.

### ws_high_risk

A workspace constructed such that:
- Includes and dependencies are tangled
- There are potential changes that would be considered "risky"
- The playbook constraints should block actual apply when max_risk is low

**Purpose**: Test that playbook safety constraints prevent applying changes.

## Test Configuration

Tests are defined in `tests_v1.json` with the following structure:

```json
{
  "tests": [
    {
      "id": "basic_safe_cleanup_applies",
      "workspace": "tests/playbooks/workspaces/ws_basic_cleanup",
      "playbook_id": "safe_cleanup_cycle",
      "expect": {
        "status": "applied_or_simulated",
        "max_events_increase": 5,
        "min_events_increase": 1,
        "allow_apply": true
      }
    }
  ]
}
```

## Running the Tests

Execute the test harness with:

```bash
cd <repo-root>
tests/playbooks/run_playbook_tests.sh
```

The script will:
1. Iterate over tests in tests_v1.json
2. For each test:
   - Sets WORKSPACE_ROOT from the test metadata
   - Ensures `.aiupp` directory exists under workspace
   - Captures baseline evolution stats
   - Executes the playbook
   - Captures new evolution_summary
   - Validates expectations using jq
3. Prints a clear PASS/FAIL line per test
4. Returns appropriate exit code (0 for success, non-zero for failure)

## Expected Output

The script outputs clear PASS/FAIL results like:
- `PASS basic_safe_cleanup_applies`
- `FAIL high_risk_blocked_by_constraints: expected blocked_by_constraint but got applied`

## Extending the Harness

To add new tests:
1. Create a new test workspace if needed
2. Add a new test configuration to `tests_v1.json`
3. Update the harness script if new validation logic is required
4. Test the changes by running the harness

## Safety and Validation

The harness includes several validation mechanisms:
- Status validation to ensure the correct outcome (applied, blocked, etc.)
- Apply validation to verify no changes were applied when expected
- Event count validation to ensure evolution changes are within expected bounds
- Constraint validation to ensure max_risk and other limits are respected