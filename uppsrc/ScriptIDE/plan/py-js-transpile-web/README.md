# Python to JavaScript Web GameState Track

## Goal

Keep Python as the authored language for `.gamestate` projects and add a Python-to-JavaScript transpilation pipeline for the supported subset needed by browser-hosted execution.

## Why This Supersedes The Earlier JS Frontend Direction

- `ByteVM` already executes Python and the repo already contains Python game projects.
- A native JavaScript frontend in `ByteVM` would split the VM architecture and increase language/runtime scope dramatically.
- A transpiler keeps authored game code aligned with the current Python/Hearts work while still enabling browser execution.

## Main Outcome

- `.gamestate` remains Python-first
- browser host consumes transpiled JavaScript artifacts
- `ScriptWebHost` serves browser execution/runtime data
- the first target reference remains Klondike under `reference/Solitaire`

## Phase Order

1. Discovery and supported-subset audit
2. Transpiler core
3. Browser runtime contract
4. `.form` DOM runtime integration
5. Klondike Python-authored reference project
6. End-to-end testing
