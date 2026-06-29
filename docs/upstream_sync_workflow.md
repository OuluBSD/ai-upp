# Upstream Synchronization Workflow

This document outlines the procedure for the Gemini AI agent to synchronize commits from the upstream repository (`ultimatepp/ultimatepp`) into our compatibility-broken fork.

## Architecture Context
The original `uppsrc/ide` package is being split to support independent console and GUI applications.
- **Original Upstream:** Monolithic `uppsrc/ide`
- **Our Fork:** 
  - `uppsrc/ide`: A minimal shim/shell package that keeps manifest and entrypoint glue in one place.
  - `uppsrc/ide/Core`: Shared headless IDE logic, including package/workspace/build metadata, console helpers, and reusable parser/build-graph behavior.
  - `uppsrc/ide/MainCtrl` and related GUI packages: GUI controller, editor, and IDE UI implementation.
  - `uppsrc/pkg` reuses `uppsrc/ide/Core` for headless package, workspace, and build graph behavior instead of duplicating package parsing logic.
  - Keep `uppsrc/ide/Core` aligned with `pkg`, since `pkg` depends on it as the canonical headless parser and build-graph layer.

## 1. State and Mapping Data
Synchronization relies on two JSON files:
- `metadata/upstream_sync.json`: Tracks the current sync state, the remote URL, and the last processed commit.
- `metadata/upstream_mapping.json`: Defines how upstream files map semantically onto the local split tree. These mappings are broad by design and should be updated whenever files move between the shim, `Core`, and GUI packages.

## 2. Synchronization Process (Agent Instructions)
When instructed to sync upstream commits, the agent should perform the following steps:

1. **Read Metadata:** Load both JSON files to determine the `last_synced_commit` and the current mapping rules.
2. **Fetch Commits:** Run `git fetch upstream` and identify new commits between `last_synced_commit` and `upstream/master`.
3. **Analyze Diff:** For each new commit, generate the diff and analyze which upstream files were modified.
4. **Apply Mappings:** Use `metadata/upstream_mapping.json` to determine the target local files and structural destinations for the modifications.
   - Mappings are semantic, not mechanical. Code changes over time, so the agent must use semantic understanding (e.g., matching function names, logic blocks) to correctly port the upstream diff into our local structural equivalents.
   - Favor the split destinations when upstream changes move logic into `ide/Core`, `ide/MainCtrl`, or the `ide` shim.
5. **Port Logic:** Intelligently port the logic changes to the mapped destinations. This is NOT a simple patch application, but a manual merge of concepts since the local codebase has a different architecture.
6. **Verify:** Build the project (both GUI and Console targets) to ensure changes did not break the build. Run available tests.
7. **Commit & Update State:** Commit the successfully ported changes locally, and update the `last_synced_commit` hash in `metadata/upstream_sync.json`. 

## 3. Maintaining Mappings
As the local architecture evolves, `metadata/upstream_mapping.json` must be kept up to date. If an upstream file is split into new components, add the corresponding entries so future sync agents can route the change semantically.
