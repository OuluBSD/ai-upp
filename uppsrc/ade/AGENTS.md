AGENTS

Scope
- Applies to `uppsrc/ade`.

Purpose
- Command-line entry point for the agentic development environment.
- Should expose client commands for browsing, editing, searching, renaming, commenting, validating, importing, and exporting ProgDB content.

Architecture
- Split client and backend server.
- Backend should stay alive and keep ProgDB indexes and semantic parser state warm.
- Client commands should be scriptable and return structured output.

Rules
- Keep core program database logic in `Vfs/ProgDB`.
- Keep GUI dependencies out.
- Prefer headless diagnostics and stdout/stderr over dialogs.
- Use `LOG` for persistent diagnostics where needed.

.upp Notes
- List `AGENTS.md` first in `ade.upp`.
