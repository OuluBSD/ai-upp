# Task: Plan Klondike `.gamestate` Reference Project

## Goal

Define the reference project to be created at `uppsrc/ScriptIDE/reference/Solitaire`, using JavaScript and the browser-capable `.gamestate` stack to implement Klondike.

## Background / Rationale

The repository currently has `reference/Hearts` as the only concrete `.gamestate` reference. Klondike should become the first reference that proves:
- JavaScript entry modules
- browser-hosted execution through the separate Skylark-based web host
- `.form` rendered as HTML elements
- single-player card-game interaction

## Scope

- Project file layout
- Game-state module split
- `.gamestate` metadata additions for JavaScript/browser hosting
- `.form` zone requirements for Klondike

## Proposed Project Layout

```text
uppsrc/ScriptIDE/reference/Solitaire/
  game.gamestate
  table.form
  main.js
  solitaire/
    logic.js
    layout.js
    rules.js
    assets/
```

## Required Klondike Zones

- `stock_pile`
- `waste_pile`
- `foundation_1`
- `foundation_2`
- `foundation_3`
- `foundation_4`
- `tableau_1`
- `tableau_2`
- `tableau_3`
- `tableau_4`
- `tableau_5`
- `tableau_6`
- `tableau_7`
- `status_line`
- `new_game_button`
- `undo_button`

## Interaction Model

Phase 1 should avoid drag-and-drop as a hard dependency.

Recommended input contract:
- click source card or pile to select
- click destination pile to move
- button for stock draw / restart / undo
- optional double-click shortcut for auto-foundation later

## `.gamestate` Direction

The schema should be extended to support at least:
- `entry_language: "javascript"`
- `entry_script: "main.js"`
- `host: "browser"`
- `layout: "table.form"`

Existing Python projects must remain valid without these new fields.

## Risks

- Klondike quickly exposes missing host features such as pile fan-out, face-down cards, selection, move validation, and undo.
- If drag-and-drop is required too early, the browser host will block the reference project.

## Acceptance Criteria

- [ ] Planned `reference/Solitaire` file layout documented
- [ ] Planned `.gamestate` schema extensions documented
- [ ] Required `.form` zones documented
- [ ] Minimal playable interaction model documented
