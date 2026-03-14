# Current Task

Current baseline:

- standalone `SkylarkApp` executable is in place
- localhost configuration and command-line bootstrap are working
- `/api/status`, `/api/bootstrap`, and `/api/transpile-entry.js` are exposed
- browser bootstrap includes `.gamestate`, `.form`, layout geometry, and transpiled entry JS metadata

Next steps:

- move from bootstrap/transpile inspection to executable browser runtime wiring
- define the browser-side host API expected by transpiled game code
- add static client assets instead of keeping the renderer inline in route handlers
