# ScriptIDE Implementation Plan

This directory contains the implementation plan for ScriptIDE, a Spyder-like Python IDE built on Ultimate++ using ByteVM.

## Migration Notice (2026-03-10)

ScriptIDE planning is now in architectural split mode:
- New shared non-GUI core: `uppsrc/ScriptCommon`
- New headless frontend: `uppsrc/ScriptCLI` (CLI + MCP host)
- `uppsrc/ScriptIDE` remains GUI frontend and adapter layer

Legacy tasks that reference non-GUI files under `uppsrc/ScriptIDE` should be interpreted via the `plan/script-cli/` migration mapping.
See also: `MIGRATION_MAP_SCRIPTCOMMON.md`.

## Plan Structure

```
plan/
├── cookie.txt                    # Current and upcoming tasks tracker
├── README.md                     # This file
├── 01-foundation/                # Track 1: Core infrastructure
│   ├── phase1-package-setup/
│   ├── phase2-ui-foundation/
│   └── phase3-bytevm-debug/
├── 02-gui-components/            # Track 2: GUI implementation
│   ├── phase1-main-window/
│   ├── phase2-editor-area/
│   ├── phase3-right-panels/
│   ├── phase4-left-sidebar/
│   └── phase5-status-menu-toolbar/
├── 03-python-integration/        # Track 3: Python execution
│   ├── phase1-console/
│   ├── phase2-execution/
│   └── phase3-debugging/
└── 04-features/                  # Track 4: IDE features
    ├── phase1-variable-explorer/
    ├── phase2-file-management/
    ├── phase3-help-system/
    └── phase4-settings/
└── plugin-system/                # Track: Plugin System and Hearts game
    ├── phase1-reconnaissance/
    ├── phase2-architecture/
    ├── phase3-formats/
    ├── phase4-editor-hosting/
    ├── phase5-plugin-registration/
    ├── phase6-vm-bridge/
    ├── phase7-formeditor-adaptation/
    ├── phase8-hearts-migration/
    ├── phase9-asset-pipeline/
    ├── phase10-reference-example/
    └── phase11-testing-acceptance/
└── script-cli/                   # Track: ScriptCommon/ScriptCLI split + MCP
    ├── phase1-discovery/
    ├── phase2-scriptcommon-architecture/
    ├── phase3-migration-execution/
    ├── phase4-scriptcli-bootstrap/
    ├── phase5-test-headless/
    └── phase6-mcp-server/
```

## Implementation Order

1. **Track 1: Foundation** - Set up package, basic UI, ByteVM debugging
2. **Track 2: GUI Components** - Build main window layout and all panels
3. **Track 3: Python Integration** - Console, execution, debugging
4. **Track 4: Features** - Variable explorer, files, help, settings

## Conventions

- Each phase contains task directories with markdown files
- `cookie.txt` is updated as tasks progress
- Tasks are atomic and testable
