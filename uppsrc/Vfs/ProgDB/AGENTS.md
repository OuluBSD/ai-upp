AGENTS

Scope
- Applies to `uppsrc/Vfs/ProgDB`.

Purpose
- Program database package for git-friendly, line-oriented program records.
- Keeps translation units, namespaces, symbols, functions, tags, comments, semantic relations, and target bindings in a filesystem-native model.
- Hosts the semantic model needed by FOG-style generator/meta-programming, where generators produce validated program nodes rather than raw text.
- Must remain headless and reusable by `uppsrc/ade`, TheIDE integrations, importers, exporters, and tests.

Relations
- Uses `Vfs/Core` for VFS integration boundaries where needed.
- May use `Core` TOON helpers (`ParseTOON`, `AsTOON`) for human-diffable sparse records and interchange.
- Future SQL compatibility should be adapter-based and must not make SQL storage the primary source of truth.

Rules
- Keep GUI code out of this package.
- Preserve stable node identity across rename operations.
- Treat derived indexes as rebuildable; source node records are authoritative.
- Do not model every small function-body AST node as a `VfsValue`; use compact ProgDB records or a custom `VfsValueExt` carrier when Vfs integration is needed.
- Do not assume TOON is the default for dense records; prefer schema-specific line formats when token order can remove repeated keys and qualifiers.
- Treat FOG-style meta-programming as semantic generation: preserve provenance from generated nodes back to generator declarations, generator calls, output channels, and source lines.
- Prefer adopting FOG thesis internals over FOG syntax: potential/actual records, meta-type syntax predicates, staged parsing, declaration composition, utility levels, usage dependency graphs, and source mapping.
- The future ProgDB language may use Python-like indent/dedent scopes and syntactic type/name separation to avoid C++-style semantic parser coupling.
- Use `ASSERT` for invariants instead of silent fallback.
- Keep `.cpp` files including only `ProgDB.h` first.

.upp Notes
- List `AGENTS.md` first in `ProgDB.upp`.
