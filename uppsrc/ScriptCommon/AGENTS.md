# ScriptCommon

Scope
- Applies to `uppsrc/ScriptCommon` and its sub-tree.

Purpose
- Headless shared script/runtime core for ScriptIDE (GUI), ScriptCLI (CLI/MCP), and future TUI frontends.

Rules
- Do not depend on `CtrlCore`, `CtrlLib`, `CodeEditor`, `RichEdit`, `Docking`, or other GUI packages.
- Keep APIs frontend-agnostic.
- All implementation files include `"ScriptCommon.h"` first.

Package Notes
- Keep `AGENTS.md` first in `ScriptCommon.upp` file list.
