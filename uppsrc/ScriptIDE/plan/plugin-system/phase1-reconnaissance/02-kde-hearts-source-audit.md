# Task: KDE Hearts Source Audit

## Goal
Inspect and summarize the KDE Hearts source code, features, and assets from `./tmp/hearts-1.98.tar.bz2` to prepare for its Python reimplementation.

## Background / Rationale
The target is a Python-based Hearts game running within ScriptIDE, inspired by the 2004 KDE Hearts game. A thorough understanding of its rules, logic, AI, and graphical assets is required to ensure faithful parity where practical.

## Scope
- Extracting gameplay rules, menu options, and scoring systems.
- Analyzing the AI behavior and human client interaction.
- Cataloging the assets (`clients/human/pics/`).
- Identifying KDE/Qt dependencies that need to be replaced with U++ / Python equivalents.

## Non-goals
- Porting the C++ code directly.
- Writing Python game logic.

## Dependencies
- Extracted contents of `./tmp/hearts-1.98.tar.bz2`.

## Concrete Investigation Steps
1. Unpack the tarball.
2. Read `server/gamemanager.cpp` to understand scoring, passing, and turn logic.
3. Review `clients/computer/` for AI behavior.
4. Review `clients/human/` for UI, drawing assumptions, and save/load logic.
5. Catalog the card images and table assets.
6. Create a feature inventory and gap analysis against ScriptIDE/ByteVM capabilities.

## Affected Subsystems
- Hearts Game Design (Python)
- Asset Pipeline

## Implementation Direction
Produce a comprehensive document detailing the exact rules (e.g., shooting the moon, point values), AI algorithms, UI states, and a list of required assets.

## Risks
- KDE-specific APIs may obscure the core game logic, requiring careful reading.
- Some features may be too complex for a v1 Python implementation.

## Acceptance Criteria
- [ ] Documented gameplay features, options, scoring, round flow, and passing rules.
- [ ] Documented AI setup and behavior.
- [ ] Catalog of required assets (cards, icons).
- [ ] List of KDE/Qt specifics that must be reinterpreted.
- [ ] Gap analysis against ScriptIDE.
