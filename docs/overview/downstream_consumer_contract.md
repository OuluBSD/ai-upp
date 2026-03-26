# ai-upp Downstream Consumer Contract (PKR/Card-Game Line)

## 1. Scope

This is not a full ai-upp explanation.
This document defines the downstream contract surfaces used by PKR/card-game workflows.

## 2. Required surfaces

Legend:
- Observed: directly evidenced in current files.
- Inferred: strongly implied from current integration.

### ScriptIDE core and plugin surfaces (Observed)
- `uppsrc/ScriptIDE/ScriptIDE.upp`
- `uppsrc/ScriptIDE/HOWTO_PLUGINS.md`
- plugin-related interfaces/manager files under `uppsrc/ScriptIDE/*`

Why required:
- PKR wrappers and CardIDE integration rely on ScriptIDE host and plugin contract behavior.

### ByteVM / Python VM runtime (Observed)
- `ScriptIDE.upp` dependency on `ByteVM`
- ByteVM usage across ScriptIDE/card-game integration paths

Why required:
- PKR GameEditor/GameEditorCLI/card scripts depend on Python/ByteVM execution semantics.

### Card/game support surfaces (Observed)
- `game/GameRules/*`
- `uppsrc/ScriptIDE/reference/Hearts`
- `uppsrc/ScriptIDE/reference/Solitaire`

Why required:
- Provides shared rule-layer and reference-project patterns used as baseline integration models.

### Build interfaces (Observed)
- `script/build.py` in ai-upp used as delegated builder by PKR.
- Support for `--search-root`, `--add-root`, and `--output-dir` in build tooling.

Why required:
- PKR build orchestration depends on ai-upp build-script behavior and CLI interface stability.

### Assets/runtime paths (Observed)
- PKR references ai-upp runtime assets from `share/imgs/cards/default` and `share/gfx/gui/...` via relative paths.

Why required:
- PKR runtime visuals and gamestate configs currently assume these paths/assets exist and remain stable.

## 3. Structural inconsistencies

### Hearts/Solitaire placement complexity (Observed)
- Reference projects live under `uppsrc/ScriptIDE/reference/*` while ai-upp also has broader `reference/` areas.
- Result: onboarding ambiguity about canonical location and ownership.

### Potential `pysrc/game` style boundary need (Inferred)
- Current reference/project placement mixes framework reference and runtime-oriented concerns.
- A clearer downstream-facing structure could reduce coupling confusion.

### Reference folder overloading (Observed/Inferred)
- “Reference” is used for both demos and practically consumed integration baselines.
- This dual role can mislead downstream consumers about stability guarantees.

### ScriptIDE plan overlap (Observed)
- Multiple parallel plan subtrees (plugin-system variants, script-cli split, separate-window, spyder, transpile tracks) reduce clarity of canonical direction.

## 4. Recommended contract simplifications (no implementation yet)

1. Publish one canonical downstream contract index for ScriptIDE/ByteVM/GameRules/build surfaces.
2. Mark canonical reference-project location explicitly (and what is demo-only).
3. Define asset-path stability policy for downstream repos (allowed moves, deprecation window, migration guidance).
4. Add explicit “supported for downstream” labels to key interfaces and scripts.
5. Consolidate overlapping ScriptIDE plan branches into a status-indexed canonical track.
6. Provide a minimal downstream smoke checklist that sibling repos can run after ai-upp changes.

## 5. Boundary guardrail reminder

Downstream consumers (like PKR) may depend on these surfaces, but ai-upp remains public/generic:
- keep domain-private poker/commercial logic out of ai-upp,
- require abstraction and de-identification before accepting downstream-driven upstream changes.

