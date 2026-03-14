# JavaScript Web GameState Track

## Goal

Add a JavaScript frontend path to `ByteVM`, host `.gamestate` projects in a browser through a separate Skylark-based local web program coordinated by ScriptIDE, and prove the stack with a Klondike reference project under `uppsrc/ScriptIDE/reference/Solitaire`.

## Investigated Constraints

- `ByteVM` currently executes Python only. There is no JavaScript parser, compiler, or module loader in `uppsrc/ByteVM`.
- `.gamestate` execution already exists through `CardGamePlugin`, but it is currently Python-oriented.
- `.form` is now a real U++ Form XML file, not the older custom JSON idea.
- The existing reference project is `uppsrc/ScriptIDE/reference/Hearts`.
- `uppsrc/ScriptIDE/reference/Solitaire` does not exist yet and must be created as part of this track.
- The web-server path should follow `reference/Skylark*` examples and `uppsrc/Skylark` techniques.
- The web server should be a separate program, not embedded directly into ScriptIDE.

## Phase Order

1. Discovery and feasibility audit
2. ByteVM JavaScript frontend design
3. Separate Skylark-based web host design
4. `.form` to HTML/DOM renderer design
5. Klondike `.gamestate` reference project plan
6. End-to-end acceptance plan

## Non-goals For This Track

- Replacing the Python frontend in `ByteVM`
- Adding a canvas-based renderer
- Designing a general internet-facing deployment system
