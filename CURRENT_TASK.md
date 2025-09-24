Repo-wide CURRENT TASK

MCP for OpenAI Codex – Vfs/AST Integration

Goals
- Provide robust Env APIs for AST-backed code graph operations used by uppsrc/ide/MCP.

Required Env APIs (to be implemented)
- Resolve node by location: EnvLocate(file, line, column) -> { id, kind, name, range }.
- Get node by id: EnvGet(id) -> node metadata + range.
- Get definition(s): EnvDefinition(id) -> [locations].
- Get references: EnvReferences(id, page?, limit?) -> { items, next_page_token }.
- Pretty print node code: EnvCode(id or range) -> string.
- Optional: EnvStatus() -> { initialized, last_update, stale_files }.

Where to implement
- uppsrc/Core2 (MetaEnvironment, VfsValue): ensure symbol table, ranges, and cross-refs are queryable.
- uppsrc/Vfs2: clean up and expose utilities for file→vfs mapping and source slices.
- uppsrc/ide/Vfs: IdeMetaEnvironment adapters that surface the above Env APIs to IDE clients.

Notes
- The AST is currently produced by ide/Builders/ScriptBuilder.cpp when the SCRIPT builder is selected and packages are compiled.
- Code paths are messy; plan a pass with AI assistance to refactor/narrow interfaces incrementally without breaking existing behavior.

Next
- Design the Env* API surface (headers in Core2/VfsValue.h and ide/Vfs/Ide.h) and add minimal stubs with TODOs.
- Inventory current data structures for declarations/definitions/references and decide on a stable node id scheme (USR-like).
- Once the Env APIs exist, wire uppsrc/ide/MCP/node.* handlers to call them and replace placeholders.
