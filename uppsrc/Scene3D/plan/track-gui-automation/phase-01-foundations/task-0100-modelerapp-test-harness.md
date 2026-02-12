# task-0100-modelerapp-test-harness

## Goal
Add a ModelerApp automation harness that runs ByteVM UI tests in-process.

## Scope
- Add `--test <script.py>` to ModelerApp.
- Open the main window and execute ByteVM automation script.
- Return non-zero exit on failure.

## Acceptance
- `bin/ModelerApp --test <script.py>` executes script and exits with status.
- Errors are surfaced in stdout/stderr.
