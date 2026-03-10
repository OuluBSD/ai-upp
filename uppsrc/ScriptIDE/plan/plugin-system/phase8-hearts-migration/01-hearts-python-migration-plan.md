# Task: Hearts Python Migration Plan

## Goal
Design the architecture and package structure for the Python implementation of Hearts, mapping KDE Hearts logic into ByteVM-compatible scripts.

## Background / Rationale
The reference project requires a playable Hearts game driven by Python scripts executing within ScriptIDE's `.gamestate` environment. This logic must recreate the KDE Hearts rules, turn flow, and AI behavior, but interface with the new ScriptIDE UI bindings instead of Qt.

## Scope
- Defining the Python package structure for the game.
- Mapping KDE gameplay behavior (passing, trick resolution, scoring) into Python modules.
- Re-implementing or migrating the AI strategy from `clients/computer/`.
- Defining the UI-to-logic bridge (event handling, layout binding).

## Non-goals
- Writing the final Python code in this planning phase.

## Dependencies
- `02-kde-hearts-source-audit.md`
- `01-python-vm-binding-bridge.md`

## Concrete Investigation Steps
1. Review the C++ logic in `server/gamemanager.cpp` and `clients/computer/computerplayer3_2.cpp` from the KDE tarball.
2. Outline a Python class structure representing the Game, Round, Player, Deck, and Trick.
3. Define how the Python script will locate UI elements defined in the `.form` (e.g., `view.get_zone("player_1_hand")`).

## Affected Subsystems
- Reference Example Project (Python scripts)

## Implementation Direction
Create a document outlining the Python module hierarchy (e.g., `hearts/logic.py`, `hearts/ai.py`, `hearts/ui.py`) and a flowchart showing the turn resolution loop bridging Python and the plugin view.

## Risks
- Subtle bugs in AI translation could result in unplayable or trivially easy computer opponents.
- Synchronous Python logic might block the ScriptIDE main thread if not designed iteratively or with coroutine/callback patterns.

## Acceptance Criteria
- [ ] Documented Python package structure.
- [ ] Mapped strategy for game loop, scoring, and AI.
- [ ] Defined UI-to-logic interaction model.
