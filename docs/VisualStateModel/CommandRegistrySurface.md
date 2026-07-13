# VisualStateModel Command Registry Surface

This document records the preferred M09 command surface for VisualStateModel
automation. The generic protocol is documented in `docs/CommandRegistry.md`;
this file records the VSM-specific commands and migration status.

## Preferred Host

Use `bin\VisualStateCommandRegistry.exe` for commandized VSM workflows:

```text
bin\VisualStateCommandRegistry.exe --list --json
bin\VisualStateCommandRegistry.exe --describe <command> --json
bin\VisualStateCommandRegistry.exe --run <command> [--arg key=value]... --json
```

The GUI wrapper stays target-driven:

```text
bin\CommandRegistryGui.exe --headless-smoke bin\VisualStateCommandRegistry.exe --smoke-command <command>
```

Do not hardcode `bin\VisualStateCommandRegistry.exe` into the GUI wrapper. The
target application is always supplied as `<target-app>` or `--app <target-app>`.

## Output Contract

The preferred automation result is the registry JSON envelope:

```json
{
  "ok": true,
  "code": 0,
  "message": "VisualStateSessionValidate completed",
  "value": {
    "app": "path-to-child-executable",
    "args": [],
    "exit_code": 0,
    "stdout": "child mixed stdout"
  }
}
```

Most wrapped VSM tools still produce mixed human-readable stdout. M09 preserves
that output in `value.stdout`; it does not claim the child stdout is normalized
JSON. Prefer the registry envelope for automation status and use child stdout for
diagnostic detail until a specific workflow is moved to a shared service or gets
a clean JSON/JSONL output mode.

## Preferred Commands

| Command | Status | Wrapped executable | Arguments | Notes |
| --- | --- | --- | --- | --- |
| `vsm.model-smoke` | preferred smoke | `VisualStateModelTest` | optional `bin_dir` | M08 command. |
| `vsm.import-sequence` | preferred smoke | `VisualStateImportSequence` | optional `bin_dir` | Currently synthetic/default mode only through registry. |
| `vsm.end-to-end` | preferred smoke | `VisualStateEndToEndSample` | optional `bin_dir` | Heavier synthetic pipeline. |
| `vsm.logic-self-test` | preferred validation | `VisualStateLogicCompare --self-test` | optional `bin_dir` | M08/M07 logic self-test. |
| `vsm.m07-smoke-pipeline` | preferred pipeline | registered child commands | optional `bin_dir` | In-process registry pipeline over process-backed children. |
| `vsm.session-validate` | preferred validation | `VisualStateSessionValidate [session_dir]` | optional `bin_dir`, optional `session_dir` | No `session_dir` runs synthetic self-check. |
| `vsm.session-diff` | preferred validation | `VisualStateSessionDiff [session_a session_b]` | optional `bin_dir`, optional paired `session_a`, `session_b` | Paired args are validated by the host. |
| `vsm.annotation-validate` | preferred validation | `VisualStateAnnotationValidate [annotation_file]` | optional `bin_dir`, optional `annotation_file` | No file runs synthetic self-check. |
| `vsm.groundtruth-init` | preferred template generation | `VisualStateGroundTruthInit [session_dir output_template_path]` | optional `bin_dir`, optional paired `session_dir`, `output_template_path` | Paired args are validated by the host. |

## Wrapped But Not Yet Preferred

These tools are good commandization candidates, but M09 has not exposed them yet:

| Future command | Existing executable | Reason deferred |
| --- | --- | --- |
| `vsm.region-dump` | `VisualStateRegionDump` | Heavier image/artifact workflow; likely needs fixture and JSONL verification. |
| `vsm.layout-assign` | `VisualStateLayoutAssign` | Depends on session + `.form`; candidate for later shared service. |
| `vsm.logic-compare` | `VisualStateLogicCompare <session> <.form>` | Existing JSONL output should be preserved and documented before wrapping. |
| `vsm.logic-production` | `VisualStateLogicCompare --production-out <path> <session> <.form>` | Production/video-only semantics need explicit validation contract. |
| `vsm.replay-report` | `VisualStateReplayReport` | Report artifact workflow; should verify generated files. |
| `vsm.record-session` | `VisualStateRecordSession` | Session-generation workflow; heavier and closer to Phase 12/TexasHoldem source work. |
| `vsm.batch-report` | `VisualStateBatchReport` | Requires sessions root and artifact checks. |
| `vsm.cache-stats` | `VisualStateCacheStats` | Cache behavior should be verified with deterministic fixture. |
| `vsm.mjpeg-source` | `VisualStateMjpegSource` | Prototype/adapter status needs clearer success criteria. |

## Legacy Entrypoints

Existing `reference/VisualState*` executables remain authoritative for their
underlying behavior until a workflow is moved into a shared headless service.
The command host currently wraps selected tools; it does not replace their
implementation.

TexasHoldem source/session commands such as `--dump-source-contract-sample`,
`--validate-source-contract-sample`, `--record-session`, and `--replay-session`
remain outside the preferred VSM command host until the Phase 12 controllable
source contract decides the boundary.

## Verified M09 Commands

M09 close-out verified the full preferred command set, not only the four commands
added during M09. All preferred command runs returned outer exit code `0`, JSON
`ok=true`, JSON `code=0`, and process-backed child `value.exit_code=0`.

```text
bin\VisualStateCommandRegistry.exe --list --json
bin\VisualStateCommandRegistry.exe --describe vsm.model-smoke --json
bin\VisualStateCommandRegistry.exe --describe vsm.import-sequence --json
bin\VisualStateCommandRegistry.exe --describe vsm.end-to-end --json
bin\VisualStateCommandRegistry.exe --describe vsm.logic-self-test --json
bin\VisualStateCommandRegistry.exe --describe vsm.m07-smoke-pipeline --json
bin\VisualStateCommandRegistry.exe --describe vsm.session-validate --json
bin\VisualStateCommandRegistry.exe --describe vsm.session-diff --json
bin\VisualStateCommandRegistry.exe --describe vsm.annotation-validate --json
bin\VisualStateCommandRegistry.exe --describe vsm.groundtruth-init --json
bin\VisualStateCommandRegistry.exe --run vsm.model-smoke --json
bin\VisualStateCommandRegistry.exe --run vsm.import-sequence --json
bin\VisualStateCommandRegistry.exe --run vsm.end-to-end --json
bin\VisualStateCommandRegistry.exe --run vsm.logic-self-test --json
bin\VisualStateCommandRegistry.exe --run vsm.m07-smoke-pipeline --json
bin\VisualStateCommandRegistry.exe --run vsm.session-validate --json
bin\VisualStateCommandRegistry.exe --run vsm.session-diff --json
bin\VisualStateCommandRegistry.exe --run vsm.annotation-validate --json
bin\VisualStateCommandRegistry.exe --run vsm.groundtruth-init --json
bin\CommandRegistryGui.exe --headless-smoke bin\VisualStateCommandRegistry.exe --smoke-command vsm.session-validate
```

Negative diagnostics are also structured:

```text
bin\VisualStateCommandRegistry.exe --run vsm.no-such-command --json
  => exit 127, ok=false, code=127
bin\VisualStateCommandRegistry.exe --run vsm.session-diff --arg session_a=C:\missing --json
  => exit 2, ok=false, code=2
bin\VisualStateCommandRegistry.exe --run vsm.groundtruth-init --arg session_dir=C:\missing --json
  => exit 2, ok=false, code=2
bin\VisualStateCommandRegistry.exe --run vsm.session-validate --arg bin_dir=C:\missing --json
  => exit 127, ok=false, code=127
```

`0149` extracted shared service logic only for the `VisualStateRegionDump`
session-region timeline path. Other preferred commands remain process-backed
wrappers over the authoritative legacy executables.
