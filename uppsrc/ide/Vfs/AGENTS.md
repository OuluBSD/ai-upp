AGENTS

Scope
- Applies to `uppsrc/ide/Vfs`.

Purpose
- Virtual filesystem (VFS) and meta-environment integration for TheIDE, plus code generation and editor controls for meta code and ECS scripting.

Package Overview
- Manifest: `Vfs.upp`.
- Core: `Vfs.{h,cpp}`, `Ide.h`.
- Code model: `ClangTypeResolver.{h,cpp}` (works with `ide/clang`), `CodeGenerator.{h,cpp}`.
- Meta UI: `MetaCodeCtrl.*`, `MetaEnvTree.*`, `MetaIndexerCtrl.*`, `MetaTempTask.*`, `EditorCtrl.*`.
- Environment: `Env.{h,cpp}`.
- DSL: `EcsLang.{h,cpp}` (see `uppsrc/Eon/AGENTS.md` for ECS/dataflow concepts).

Extension Points
- Add new resolvers or generators in `CodeGenerator.*` and `ClangTypeResolver.*`.
- Extend meta editors and views by adding new `*Ctrl` implementations.
- Keep code-model logic decoupled from UI where possible.

.upp File Notes
- Ensure `AGENTS.md` is first in `Vfs.upp` `file` section.

