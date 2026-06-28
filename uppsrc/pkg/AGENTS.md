AGENTS

Scope
- Applies to `uppsrc/pkg` and its sub-tree.

Purpose
- `pkg` is the ai-upp package manager console tool.
- Keep it console-only and reuse `ide/Core` for package/workspace parsing.

Conventions
- Prefer small, direct helpers over a second build-graph implementation.
- Keep user-facing output in English.
- Treat `--pretend` and `--ask` flows as first-class CLI behavior even while build execution remains mocked.

