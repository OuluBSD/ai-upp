# ScriptIDE Card Game Reference

This directory contains reference `.gamestate` projects for the `ScriptIDE` card-game host.

Current references:
- [`Hearts`](./Hearts): full multiplayer sample with rules, AI, and animations
- [`Solitaire`](./Solitaire): browser-hosted Klondike board scaffold used to validate the transpiler + `.form` path without Hearts-specific logic

`Hearts` remains the richest example. It shows:
- a real `.gamestate` entrypoint
- a real `.form` table layout
- Python game logic running inside ByteVM
- table labels, buttons, card sprites, hidden hands, trick animations, pass animations, and round/game summaries

Use Hearts as the starting point for other card games.

## File Layout

A typical game directory looks like this:

```text
MyGame/
  game.gamestate
  table.form
  main.py
  mygame/
    logic.py
    ai.py
```

Minimal roles:
- `game.gamestate`: declares the entry script, entry function, and layout
- `table.form`: defines named zones and controls for the table
- `main.py`: owns UI synchronization and callback entrypoints
- `logic.py`: owns the rules and mutable game state

Example `.gamestate`:

```json
{
  "entry_script": "main.py",
  "entry_function": "start",
  "layout": "table.form",
  "metadata": {
    "players": 4,
    "browser_modules": [
      {
        "name": "mygame.logic",
        "kind": "js",
        "path": "browser/mygame.logic.js"
      }
    ]
  }
}
```

## Runtime Model

`ScriptIDE` loads the `.gamestate`, opens `table.form`, and executes the Python entry function.

Your Python code talks to the UI through the `cardgame_view` binding.
For compatibility, the runtime still also exposes `hearts_view` as an alias.

Core calls:
- `cardgame_view.set_card(card_id, asset_path, x, y, rotation_deg=0)`
- `cardgame_view.move_card(card_id, zone_id, offset, animated)`
- `cardgame_view.remove_sprite(card_id)`
- `cardgame_view.clear_sprites()`
- `cardgame_view.begin_sprite_frame()`
- `cardgame_view.get_zone_rect(zone_id)`
- `cardgame_view.set_label(zone_id, text)`
- `cardgame_view.set_button(zone_id, text, enabled)`
- `cardgame_view.set_highlight(zone_id, enabled)`
- `cardgame_view.set_status(text)`
- `cardgame_view.log(text)`
- `cardgame_view.set_timeout(delay_ms, callback_name)`
- `cardgame_view.set_expected_sprite_count(zone_id, count)`

## Browser Module Convention

For browser-hosted projects, `.gamestate` metadata may declare `browser_modules`.
Current supported module kind:

- `js`: fetch a project-owned JavaScript file and expose `__scriptwebhost_module__`
- `py`: transpile a project-owned Python file and export the listed names

Example browser module file:

```javascript
const __scriptwebhost_module__ = {
  answer() { return 42; }
};
```

The transpiled Python entry can then import it by name, for example:

```python
import mygame.logic
```

Example Python browser module metadata:

```json
{
  "name": "mygame_bridge",
  "kind": "py",
  "path": "browser/mygame_bridge.py",
  "exports": ["make_label", "next_state"]
}
```

## Required Python Entry Points

Current host conventions expect these functions in `main.py`:
- `start()`
- `refresh_ui()`
- `on_click(card_id)`
- `on_button(button_id)`

Optional interaction callbacks:
- `on_drag(card_id, zone_id)`

Optional timed callbacks can be scheduled by name with `set_timeout()`, for example:
- `ai_step()`
- `next_round()`
- `finish_trick_collect()`

## How To Structure A New Card Game

Recommended split:

1. Keep rules in `logic.py`.
   - deck
   - hands
   - turn state
   - trick / pile / stack / board state
   - scoring
   - win / round transitions

2. Keep rendering and user interaction in `main.py`.
   - map logic state to form zones
   - create / move / remove sprites
   - update labels and buttons
   - schedule animations and delayed state transitions

3. Keep `table.form` generic.
   - hand zones
   - trick / center area
   - labels
   - buttons
   - status areas

Do not put game rules in the C++ host unless the feature is clearly reusable across multiple games.

## `.form` Conventions

`table.form` is a real `Form` file. The runtime reads actual control geometry from the generated form tree.

Important item properties:
- `Variable`: stable id used from Python
- `Type`: usually `Label` or `Button`
- `UserClass`: semantic role such as `HAND`, `TRICK`, `LABEL`, `BUTTON`, `CONTAINER`
- `Anchor`: positioning rule like `BOTTOM_CENTER`, `CENTER_LEFT`, `TOP_RIGHT`, etc.
- `Parent`: optional parent control id

Typical named zones:
- `hand_self`
- `hand_left`
- `hand_top`
- `hand_right`
- `trick_bottom`
- `trick_left`
- `trick_top`
- `trick_right`
- `status_line`

For new games, prefer stable semantic ids over hard-coding positions in Python.

## Sprite Rules

The host is frame-based now.

In every `refresh_ui()`:
1. call `cardgame_view.begin_sprite_frame()`
2. re-emit every sprite that should exist this frame
3. remove only special transient sprites explicitly if needed
4. call `set_expected_sprite_count(...)` for important zones

This avoids stale-card rendering bugs.

Keep these assertions in your game code. They are not optional debugging noise. They catch real desynchronization between logic state and rendered state.

## Single-Player Games

Single-player card games fit this host well.

Examples:
- Solitaire variants
- FreeCell
- Spider
- Poker trainer
- Klondike-like puzzle games

Recommended pattern:
- player interaction through `on_click()` / `on_button()`
- add `on_drag()` when card movement should follow the pointer before drop
- no networking
- no remote authority
- logic state fully local
- use `set_timeout()` for animations, hint delays, auto-complete, or deal sequences

For a solitaire-like game:
- represent tableau/foundation/waste/stock as separate logic piles
- define one form zone per pile
- render each pile as a stack of sprites
- prefer `on_drag(card_id, zone_id)` for move/drop interaction

## Local Multiplayer / Hotseat

Hotseat games are just single-process games with multiple logical players.

Recommended pattern:
- one shared `GameState`
- explicit `turn`
- `refresh_ui()` highlights the active player
- hide private information for non-active players if needed

For asymmetric visibility:
- render player 0 as face cards
- render other hands as backs
- optionally add a “pass device” / “next player” button between turns

## Online / Networked Multiplayer

The current Hearts example is local-only. For multiplayer, treat the game logic as authoritative and the UI as a projection.

Recommended architecture:

1. Separate pure game state from transport.
   - `logic.py` should validate moves and apply state transitions
   - networking code should only deliver commands and state snapshots

2. Choose an authority model.
   - host-authoritative: one peer owns the state and broadcasts updates
   - server-authoritative: external server owns the state
   - lockstep: every peer applies the same deterministic commands

3. Keep UI updates local.
   - incoming network events update local state
   - then call `refresh_ui()`

4. Do not let remote messages mutate rendering directly.
   - they should mutate game state first

5. Add explicit local-player identity.
   - map remote player ids to seat ids
   - only allow local input when it is the local player’s turn

Practical starting point:
- add a transport module next to `logic.py`
- expose `submit_local_move(...)`
- expose `apply_remote_event(...)`
- keep all visual logic in `main.py`

## Animation Pattern

The current Hearts example uses a reliable pattern:

1. logic enters a pending state
   - for example `trick_pending`

2. UI starts an animation phase
   - for example moving trick cards to the winner

3. a timeout fires
   - `finish_trick_collect()`

4. logic finalizes the transition
   - `resolve_trick()`

Use the same pattern for:
- dealing
- passing
- collecting tricks
- capturing stacks
- flipping cards
- end-of-round pause

Avoid long blocking loops. Use `set_timeout()` and small state-machine steps instead.

## Assertions And Debugging

Keep logic-side and render-side assertions in your game.

Recommended checks:
- no duplicate cards
- no `None` cards
- expected total card count
- expected rendered count in visible zones
- no card rendered in both hand and trick

Useful CLI flags when debugging in `ScriptIDE`:
- `--autostart`
- `--autoplay`
- `--maximize`
- `--stdout-log`
- `--dump-scene`
- `--dump-console`
- `--dump-python-stack`
- `--exit-on-assert`
- `--timeout-ms=...`
- timed actions like `--click-first-hand-cards-at=...` and `--press-button-at=...`

Example:

```bash
bin/ScriptIDE --maximize --autostart --stdout-log --dump-scene --exit-on-assert \
  uppsrc/ScriptIDE/reference/Hearts/game.gamestate
```

## Asset Guidance

For standard playing cards, prefer shared assets under:

```text
share/imgs/cards/default/
```

The Hearts reference currently uses:
- face cards from `share/imgs/cards/default/`
- hidden-hand backs like `back9.png`

If your game uses a standard deck, reuse the shared deck path instead of copying per-game assets.

## Suggested Implementation Order For A New Game

1. Create `game.gamestate`
2. Create a minimal `table.form`
3. Implement `logic.py` with deal / turn / win rules
4. Implement `start()` and `refresh_ui()`
5. Render one visible hand and one center area
6. Add click handling
7. Add AI or hotseat turn progression
8. Add animations
9. Add assertions
10. Add summary / game-over UI

## What To Copy From Hearts

Good things to reuse:
- form zone naming pattern
- `refresh_ui()` structure
- delayed animation phases
- render assertions
- hand sorting and overlap rendering pattern
- score / status label updates

Things to replace:
- Hearts-specific rules
- pass-direction logic
- trick-point logic
- hearts-broken rules
- suit-specific AI

## Current Limitation

The Python binding module is now named `cardgame_view`.
The runtime still registers `hearts_view` as a compatibility alias for older scripts.

Do not treat that name as meaning “only Hearts is supported”. It is just the current binding name.
