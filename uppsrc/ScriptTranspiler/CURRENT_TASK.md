# Current Task

Current baseline:

- initial `ScriptTranspiler` package scaffold exists
- result object with warnings/errors is stable
- Hearts `main.py` transpiles through `ScriptWebHost`
- generated JS passes `node --check`
- Hearts entry now survives a Node harness `start()` run with browser-host shims
- Solitaire browser scaffold also transpiles and runs through `ScriptWebHost`
- supported subset is now documented in `SUPPORTED_SUBSET.md`
- browser-side helper modules are now project-owned assets declared via `.gamestate` metadata rather than baked into `runtime.js`

Next steps:

- decide whether imported Python modules are transpiled recursively or replaced by browser host shims
- continue reducing Python/JavaScript semantic gaps like constructors and container behavior
- add targeted coverage only when Hearts or Solitaire require a new Python pattern
