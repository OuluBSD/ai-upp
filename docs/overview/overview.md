# ai-upp Repository Overview (Authoritative, PKR-Dependency Focus)

## 1. Repository role

For PKR, `ai-upp` is the public foundational platform repo:
- U++ framework/base tooling,
- ScriptIDE/ByteVM runtime and plugin interfaces,
- shared game infrastructure (including `game/GameRules` and card-related references/assets),
- common build/test substrate used by dependent repos.

Observed dependency impact:
- PKR build is delegated to `ai-upp/script/build.py`.
- PKR runtime references `ai-upp/share/...` assets and ScriptIDE integration points.

What belongs here:
- generic framework/runtime/editor/plugin infrastructure,
- generally known card-game/reference material suitable for public distribution.

What does not belong here:
- PKR-private/commercial poker logic, proprietary strategy models, closed product workflows.

## 2. PKR-prototype-relevant areas

High relevance (Observed):
- `uppsrc/ScriptIDE/*` (plugin system, run targets, card-game host integration).
- `uppsrc/ByteVM/*` dependency surface used by ScriptIDE/PKR packages.
- `game/GameRules/*` shared rules interfaces.
- `uppsrc/ScriptIDE/reference/Hearts` and `reference/Solitaire` as public reference projects.
- `share/` card/table assets consumed from PKR gamestate configs.
- `script/build.py` and test scripts used by downstream builds.

Medium relevance:
- ScriptIDE planning subtrees (`uppsrc/ScriptIDE/plan/*`) where architecture direction is documented.

Lower relevance / ballast for PKR prototype:
- massive unrelated framework trees, broad historical docs/book content, unrelated game projects (e.g., Umbrella), and repository-root operational residue files.

## 3. Dependencies and boundaries

Observed dependency edges:
- PKR depends directly on ai-upp build script and uppsrc roots.
- PKR references ai-upp assets by relative path in `pysrc/texasholdem/*.gamestate`.
- ScriptIDE exposes plugin and host extension surfaces (`HOWTO_PLUGINS.md`, plugin interfaces, CardGame plugin support).

Boundary risks:
- High risk of coupling drift: PKR runtime behavior can silently depend on ai-upp path/layout details.
- Medium risk of domain contamination if PKR-specific semantics are upstreamed without generic abstraction.

Boundary policy:
- ai-upp should export stable generic contracts only.
- PKR should consume via explicit interfaces, not implicit directory assumptions.

## 4. PLAN status snapshot

Observed:
- ai-upp has broad planning surfaces in root `plan/*` and especially `uppsrc/ScriptIDE/plan/*` (plugin-system, script-cli split, run targets, web/transpile tracks, etc.).

Likely active and PKR-relevant:
- plugin-system and script-cli split work that define integration and headless tooling boundaries.

Potentially stale/overlapping:
- many parallel ScriptIDE plans with overlapping themes (plugin-system variants, spyder variants, separate-window, py-js-transpile-web).
- difficult to infer single authoritative track without explicit status map.

## 5. Docs status snapshot

Useful:
- `uppsrc/ScriptIDE/HOWTO_PLUGINS.md` provides concrete extension contract guidance.
- `uppsrc/ScriptIDE/reference/README.md` clarifies Hearts/Solitaire reference intent.

Potentially stale/misaligned for PKR dependency consumers:
- root `README.md` is framework-centric and not sufficient as a dependency contract for downstream repos.
- many historical docs and reports are not clearly tagged by freshness.

## 6. Script status snapshot

Critical for dependency chain:
- `script/build.py` (downstream delegated builds).
- selected ScriptIDE-related tests (`script/test_scriptide_*`, `script/test_scriptcli_mcp.py`) as contract checks.

Likely secondary/noise from PKR perspective:
- broad unrelated scripts and environment-specific operational utilities.

## 7. File and data usage

Observed:
- PKR consumes ai-upp assets from `share/imgs/cards/default` and `share/gfx/gui/...` through gamestate configs.
- ai-upp `.gitignore` excludes many generated/runtime artifacts but allows core sources and relevant refs.

Uncertain:
- exact minimal asset subset required by PKR prototype is not enumerated in current docs.

## 8. Structural issues

Observed:
- extremely broad monorepo-style scope with mixed concerns and residual operational files at root.
- no concise downstream-consumer contract for ScriptIDE/ByteVM/GameRules surfaces.
- multiple overlapping plan trees reduce clarity on canonical roadmap.

Inferred:
- PKR dependency stability would improve if ai-upp published a narrow “supported downstream interface” map.

## 9. Recommended next analytical steps (no large implementation yet)

1. Publish an explicit downstream contract doc for ScriptIDE/ByteVM/GameRules/build interfaces.
2. Add stable path/version policy for shared assets consumed by PKR.
3. Consolidate overlapping ScriptIDE plan trees into one canonical status-indexed track.
4. Mark non-current plan branches as archived/superseded.
5. Add a dependency-consumer smoke checklist specifically for sibling repos (PKR/ConvNetCpp).
6. Document public-vs-private contribution rules to prevent PKR leakage.

## Special investigation: ai-upp (ScriptIDE/ByteVM/game focus)

Observed hard dependency surfaces for PKR:
- `uppsrc/ScriptIDE/ScriptIDE.upp` depends on `ByteVM` and contains plugin/run-target machinery.
- `uppsrc/ScriptIDE/HOWTO_PLUGINS.md` describes external plugin strategy suitable for PKR wrappers.
- `uppsrc/ScriptIDE/reference/Hearts` and `reference/Solitaire` exist as public reference game projects.
- `game/GameRules` provides generic card-game rule interfaces used as shared boundary layer.

Hearts/Solitaire placement assessment:
- Keeping Hearts/Solitaire in ai-upp is logically consistent as public reference/classic game examples.
- PKR should keep private poker-specific flows out of ai-upp and integrate via plugin/host interfaces.

Python-game placement concern:
- There are multiple “reference” loci (root `reference/`, `uppsrc/ScriptIDE/reference/*`) and plan documents discussing additional runtime/export modes.
- This is structurally workable but cognitively heavy; a single authoritative placement policy is missing.

What ai-upp should likely clean up later for PKR clarity:
- one canonical ScriptIDE plan index,
- one canonical reference-game location policy,
- explicit separation between framework plans vs product-consumer guidance.

