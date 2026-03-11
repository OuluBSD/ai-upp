# Headless Plugin Testing Strategy

## Overview
This document defines how ScriptIDE plugins are tested in headless mode using `ScriptCLI` and `ScriptCommon`. This ensures that plugin logic (Python bindings, file processing, state management) can be verified without a GUI environment.

## Directory Layout
Plugin tests are located in `autotest/Plugins/`:

```
autotest/Plugins/<PluginID>/
├── fixtures/           # Static files used as input for tests (e.g., .py, .gamestate)
├── tests/              # Python test scripts
│   ├── test_logic.py   # Verifies VM bindings and logic
│   └── test_files.py    # Verifies file type handling
└── golden/             # Expected output for deterministic assertions
    ├── test_logic.golden
    └── test_files.golden
```

## Testing Harness
The `HeadlessPluginContext` (implemented in `ScriptCommon`) provides the minimal environment required by plugins:
- Access to `PyVM`.
- Stubbed logging.
- No GUI/Ctrl dependency.

## Test Execution
Tests are invoked via `ScriptCLI`:

```bash
bin/ScriptCLI plugin test <PluginID>
```

### Execution Steps:
1. **Load**: The harness loads the plugin instance.
2. **Initialize**: `plugin->Init(headless_context)` is called.
3. **Register**: The plugin registers its providers (Bindings, Execute, etc.) to the registry.
4. **Run**:
   - For logic tests: The harness executes Python scripts from the `tests/` directory.
   - For provider tests: The harness calls the registered providers directly with fixture paths.
5. **Verify**: Standard output/error and VM state are compared against golden files.

## Deterministic Assertions
To ensure CI stability:
- All volatile state (memory addresses, timestamps) must be masked in output.
- `PyVM` state can be serialized to JSON for deep comparison.

## CI Integration
Headless plugin tests are part of the standard `autotest` suite:
- `script/test.py` will include a stage for `ScriptCLI plugin test` for all enabled plugins.
- Failure in any plugin test blocks the merge.
