# ProgDB Task: Core Implementation

## Theme
Implement the foundational ProgDB features including filesystem storage layout, potential/actual composition model, custom line format serialization, Python-style indent/dedent statement parser, relational query interface, and a test suite.

## Goals (Phase 1)
- [x] Expand `ProgNodeRecord` to support:
  - Utility levels (`pool`, `emit`, `utility`, `frozen`).
  - Merge policies (`prepend`, `append`, `replace`, `exclusive`).
  - Provenance tracking (source file, line number, generator ID, potential ID).
  - Potential vs Actual flag.
- [x] Add compact `ProgAstNode` representation for AST/statements.
- [x] Implement parser/generator for Python-style indent/dedent language.
- [x] Implement ProgDB custom line format reader/writer (PLF) for git-friendly storage.
- [x] Implement `ProgDatabase` class to manage:
  - Database folder structure (`units/`, `symbols/`, `functions/`, `comments/`, `tags/`, `indexes/`, `target-api-map/`).
  - Node saving & loading.
  - Relational indexes and queries (parent-child lookups, tag searches, relations traversal).
- [x] Create `upptst/ProgDBTest` suite validating:
  - PLF serialization parity.
  - Indent/dedent AST parser stages.
  - Potential/actual contribution composition.
  - Relational indexes on disk.
- [x] Update `ade` command line tool to talk to `ProgDatabase`.

## Progress
- [x] Completed all core functionality.
- [x] Compilation checks passed under native MSVC 2022 64-bit (`MSVS22x64`).
- [x] Automated test suite `ProgDBTest` runs and passes successfully.
- [x] Exposed all client-server commands inside the `ade` compiler executable.
