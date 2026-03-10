# Task: Migration and Compatibility Risks

## Goal
Identify and document the architectural mismatches and compatibility risks between the KDE/Qt assumptions of the original Hearts game and the U++/ScriptIDE/ByteVM target environment.

## Background / Rationale
The original KDE Hearts game relies heavily on Qt's event loop, C++ client-server networking, and specific Qt drawing APIs. Our target is a single-process, Python-driven game executing within a custom U++ `Ctrl` inside ScriptIDE. Anticipating the friction points early prevents architectural dead-ends.

## Scope
- Identifying what should be preserved exactly (e.g., scoring rules, AI heuristics) vs what must be adapted (e.g., event handling, rendering).
- Documenting the mismatch between the KDE client-server model and the target single-player local Python execution model.
- Evaluating the performance of drawing many card sprites via U++ `Draw` from Python bindings.

## Non-goals
- Solving these risks with code in this task.

## Dependencies
- `02-kde-hearts-source-audit.md`

## Concrete Investigation Steps
1. Compare the KDE `QCanvas` or custom Qt drawing usage in `clients/human/` against the capabilities of U++ `Ctrl::Paint`.
2. Analyze the network message loop in `server/gamemanager.cpp` and determine how to flatten it into synchronous function calls or asynchronous U++ `PostCallback` events.
3. Review the ByteVM's execution speed to assess if AI algorithms need optimization.

## Affected Subsystems
- Plugin System / VM Bridge
- Hearts Python Implementation

## Implementation Direction
Produce a risk assessment matrix. For each risk, propose a mitigation strategy (e.g., "Risk: Client-server networking is too complex. Mitigation: Flatten the architecture into a single local game loop where 'computer players' are just Python functions called in sequence.").

## Risks
- Underestimating the difficulty of converting async network logic to local event-driven UI logic.

## Acceptance Criteria
- [ ] Documented mismatches between KDE/Qt and U++/ScriptIDE.
- [ ] List of features to preserve vs. adapt.
- [ ] Risk mitigation matrix defined.
