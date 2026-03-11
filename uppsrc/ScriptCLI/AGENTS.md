# ScriptCLI

Scope
- Applies to `uppsrc/ScriptCLI` and its sub-tree.

Purpose
- Headless CLI frontend over ScriptCommon services (and later MCP host mode).

Rules
- No GUI package dependencies (`CtrlCore`, `CtrlLib`, etc.).
- All implementation files include `"ScriptCLI.h"` first.
- Keep APIs/commands automation-friendly (stable exit codes and parsable output).

Package Notes
- Keep `AGENTS.md` first in `ScriptCLI.upp` file list.
