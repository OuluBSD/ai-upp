# Current Task

Current baseline:

- initial `ScriptTranspiler` package scaffold exists
- result object with warnings/errors is stable
- Hearts `main.py` transpiles through `ScriptWebHost`
- generated JS passes `node --check`
- Hearts entry now survives a Node harness `start()` run with browser-host shims

Next steps:

- document the supported Python subset explicitly
- decide whether imported Python modules are transpiled recursively or replaced by browser host shims
- continue reducing Python/JavaScript semantic gaps like constructors and container behavior
