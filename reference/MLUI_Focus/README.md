# MLUI_Focus

Reference package with short implementation skeletons for all focus-model concepts.

## Included Concepts (13)

1. `FocusPage`
2. `FocusRoute`
3. `FocusSiteMap`
4. `FocusLink`
5. `FocusQuery`
6. `FocusComponent`
7. `FocusForm`
8. `FocusActionContract`
9. `FocusState`
10. `FocusHistory`
11. `FocusDiff`
12. `FocusPermissions`
13. `FocusDevtools`

## What Is Implemented

- Registry functions for each concept (register/get/push/set style).
- Small data structs with minimal helper methods.
- Reference-based registration calls (APIs accept `const Focus*Def&` links).
- Macro helpers for runtime page instrumentation:
  - `MLUI_USE_VAR`
  - `MLUI_USE_CTRL`
  - `MLUI_USE_STATE`
  - `MLUI_USE_ACTION`
- `INITBLOCK` setup for all concepts in one domain scenario.
- Demo app (`MLUIFocusReferenceApp`) that updates:
  - state
  - history
  - diff
  - permissions
  - devtools log
- `Access(Visitor&)` export via `EmitAllFocusArtifacts(...)` for MLUI automation path.

## Domain Scenario

The GUI is a small issue tracker style editor:
- board on left
- issue editor on right

Focus concepts are attached to this domain so the example resembles real app code, not just concept text.
