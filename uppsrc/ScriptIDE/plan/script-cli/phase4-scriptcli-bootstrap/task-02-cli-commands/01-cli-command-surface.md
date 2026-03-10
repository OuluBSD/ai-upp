# Task: Define ScriptCLI Command Surface for Headless Workflows

## Goal
Specify the first command set for plugin development/testing workflows.

## Background / Rationale
A clear CLI command contract is needed before implementation to prevent churn and ambiguous automation behavior.

## Scope
- Command names, arguments, output format, exit codes.
- Mapping commands to ScriptCommon services.

## Non-goals
- TUI UX design.
- Long-term command catalog.

## Dependencies
- ScriptCLI package layout.
- ScriptCommon runtime/plugin APIs.

## Concrete Investigation Steps
1. Define minimal command set for immediate use:
   - `run`
   - `lint`
   - `plugin list`
   - `plugin test`
   - `mcp serve`
2. Define `--format=text|json` output mode.
3. Define deterministic exit codes for CI.
4. Define command-to-service mapping.

## Affected Subsystems
- Future `uppsrc/ScriptCLI/*`
- Future headless test harnesses

## Implementation Direction
Initial command proposal:
- `scriptcli run <file>`
- `scriptcli lint <file>`
- `scriptcli plugin list`
- `scriptcli plugin test <plugin-id> [--case <name>]`
- `scriptcli mcp serve [--port <n>] [--workspace <path>] [--transport tcp|stdio]`

Exit code proposal:
- `0`: success
- `1`: command/usage error
- `2`: runtime/lint/plugin failure
- `3`: infrastructure/config error

## Risks
- Ambiguous plugin test command scope.
- JSON output schema drift without versioning.

## Acceptance Criteria
- [ ] Command syntax is documented.
- [ ] Output and exit-code contract is documented.
- [ ] Every command maps to a ScriptCommon service boundary, including MCP host mode.
