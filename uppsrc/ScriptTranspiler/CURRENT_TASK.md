# Current Task

Current baseline:

- initial `ScriptTranspiler` package scaffold exists
- result object with warnings/errors is stable
- Hearts `main.py` transpiles through `ScriptWebHost`
- generated JS passes `node --check`

Next steps:

- document the supported Python subset explicitly
- decide whether imported Python modules are transpiled recursively or replaced by browser host shims
- add runtime helper strategy for Python builtins and module imports beyond the current prelude
