# Current Task

Current baseline:

- standalone `SkylarkApp` executable is in place
- localhost configuration and command-line bootstrap are working
- `/api/status`, `/api/bootstrap`, `/api/transpile-entry.js`, and `/fs/**` are exposed
- browser bootstrap includes `.gamestate`, `.form`, layout geometry, and transpiled entry JS metadata
- the host page now includes a sprite layer, `cardgame_view` browser shim, and a Hearts-specific `hearts.logic` shim
- browser runtime assets now live in `static/`
- `SmokeTest.sh` uses Playwright CLI to wait for a running page with 52 sprites before capturing evidence
- container labels no longer destroy nested trick zones; `trick_*` slots now survive `set_label("trick_area", ...)` and mid-trick cards render in the center area again

Next steps:

- broaden the runtime/import model beyond the current Hearts-specific shim
- continue tightening browser presentation details like hand stacking and animation polish
- replace the remaining dedicated runtime route wording/docs to match the static-asset design
