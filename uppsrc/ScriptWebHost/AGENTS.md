# ScriptWebHost

Standalone browser host for ScriptIDE `.gamestate` sessions.

## Rules

- Keep this package as a separate executable from `ScriptIDE`.
- Follow `uppsrc/Skylark` and `reference/Skylark*` patterns for HTTP routing and app lifecycle.
- Reuse `ScriptCommon` for non-GUI runtime logic when possible.
- Do not add `CtrlCore` or `CtrlLib` dependencies here unless a later task explicitly requires them.
- Treat this package as localhost-first infrastructure for development and testing.
