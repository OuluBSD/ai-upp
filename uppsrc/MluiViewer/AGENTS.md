AGENTS

Scope
- Applies to `uppsrc/MluiViewer`.

Purpose
- Lightweight MLUI viewer/debugger for JSON snapshots and simple input dispatch.

Conventions
- Keep this package minimal and dependency-light (`CtrlLib` only).
- Use JSON over plain TCP to talk to MLUI runtime (`--mlui-server__`).
