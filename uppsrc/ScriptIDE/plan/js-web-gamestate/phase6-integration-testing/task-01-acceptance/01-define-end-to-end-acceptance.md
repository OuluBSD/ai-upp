# Task: Define End-to-End Acceptance for JS Web GameState

## Goal

Define the acceptance gates for the JavaScript ByteVM frontend, the separate Skylark-based browser host, the HTML `.form` renderer, and the Klondike reference project.

## Acceptance Stack

1. Language frontend
   - JavaScript modules load through the new ByteVM frontend
   - callable exports can be invoked by the host
2. `.gamestate` host
   - ScriptIDE can open and run a browser-capable `.gamestate`
   - ScriptIDE launches a separate localhost-only Skylark-based host program
3. Browser runtime
   - form shell loads
   - named zones appear with correct geometry
   - sprite and label updates propagate live
4. Klondike gameplay
   - new game deals correctly
   - stock/waste interactions work
   - legal tableau/foundation moves work
   - win detection works

## Required Test Types

- ByteVM frontend unit tests
- `.gamestate` schema compatibility tests
- browser session protocol tests
- separate-process launch and shutdown tests
- desktop-to-browser form parity checks
- reference-project smoke tests for Klondike

## Manual Demo Criteria

- From ScriptIDE, user opens `uppsrc/ScriptIDE/reference/Solitaire/game.gamestate`
- ScriptIDE starts the separate Skylark-based web host
- Default browser opens the local session URL
- The Klondike board is visible using HTML elements, not canvas
- User can complete at least one valid move

## Regression Gates

- Existing Python `.gamestate` projects still run
- `reference/Hearts` still works in desktop host mode
- ScriptCLI headless execution is unaffected unless browser host is explicitly requested

## Risks

- Browser acceptance can be flaky unless ports, caching, and session reset are deterministic
- If desktop and browser form hosts diverge, tests must compare normalized geometry rather than raw pixels

## Acceptance Criteria

- [ ] Test matrix documented
- [ ] Manual demo path documented
- [ ] Regression expectations documented for Hearts and Python `.gamestate`
