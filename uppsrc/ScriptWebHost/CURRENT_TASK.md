# Current Task

Current baseline:

- standalone `SkylarkApp` executable is in place
- localhost configuration and command-line bootstrap are working
- `/api/status`, `/api/bootstrap`, `/api/transpile-entry.js`, and `/fs/**` are exposed
- browser bootstrap includes `.gamestate`, `.form`, layout geometry, and transpiled entry JS metadata
- the host page now includes a sprite layer and generic `cardgame_view` browser shim
- browser runtime assets now live in `static/`
- `SmokeTest.sh` uses Playwright CLI to wait for a running page with 52 sprites before capturing evidence
- container labels no longer destroy nested trick zones; `trick_*` slots now survive `set_label("trick_area", ...)` and mid-trick cards render in the center area again
- browser module loading now goes through `.gamestate` metadata and project-owned JS module assets instead of a hard-coded Hearts shim in `runtime.js`
- `reference/Solitaire` now provides a second browser-hosted `.gamestate` scaffold that runs without Hearts-specific logic modules
- browser modules now support both:
  - `kind: "js"` for project-owned JavaScript helpers
  - `kind: "py"` for host-transpiled Python helpers with explicit exports
- dotted imports like `import solitaire.bridge` now work for transpiled Python helper modules
- browser hand cards now support live pointer dragging with drop callbacks through optional `on_drag(card_id, zone_id)`

Next steps:

- add explicit browser drag regression coverage beside the autoplay/trick regression
- continue tightening browser presentation details like hand stacking and animation polish
- replace the remaining dedicated runtime route wording/docs to match the static-asset design
