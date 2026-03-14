# Current Task

Current baseline:

- standalone `SkylarkApp` executable is in place
- localhost configuration and command-line bootstrap are working
- `/api/status`, `/api/bootstrap`, `/api/transpile-entry.js`, `/runtime.js`, and `/fs/**` are exposed
- browser bootstrap includes `.gamestate`, `.form`, layout geometry, and transpiled entry JS metadata
- the host page now includes a sprite layer, `cardgame_view` browser shim, and a Hearts-specific `hearts.logic` shim

Next steps:

- replace inline runtime assets with static files
- broaden the runtime/import model beyond the current Hearts-specific shim
- add end-to-end browser verification instead of relying on HTTP and Node harness checks
