# Hearts GUI Revival

Reference archive: `tmp/hearts-1.98.tar.bz2`

Primary reference files:
- `hearts-1.98/clients/human/humaninterface.cpp`
- `hearts-1.98/clients/human/humaninterface.h`
- `hearts-1.98/clients/human/tabledisplayer.cpp`
- `hearts-1.98/clients/human/hand.cpp`
- `hearts-1.98/clients/human/pointsbox.cpp`

Current migration status:
- Done: embedded `.gamestate` game host in `ScriptIDE`
- Done: X11-capable `ScriptIDE` configs (`X11`, `X11 USEMALLOC`)
- Done: resize-triggered `refresh_ui()` in the game host
- Done: first HUD pass copied from the old client model:
  - seat labels (`self`, `left`, `top`, `right`)
  - bottom status line
  - names + total scores visible during play
- Done: first hidden-opponent-hand pass:
  - left, top, and right players now render card backs
  - seat labels include remaining hand counts

Architectural correction:
- The current Hearts host is still using a custom JSON `.form` parser in
  `CardGameDocumentHost::SetLayout()`.
- That is not the same renderer path as `bazaar/FormExample` or the `Form` package.
- `CardGameLayoutEditor` currently embeds `FormEdit` but saves/loads the same custom
  JSON scene, so editor and runtime are already drifting from real `Form`.
- This should now be treated as transitional code, not the desired endpoint.

Next tasks:
1. Fix the `.form` architecture.
   - Migrate Hearts `table.form` to the real `Form` runtime path.
   - Replace the manual JSON parser in `CardGameDocumentHost`.
   - Keep card-game-specific rendering as an overlay layer.
2. Align editor and runtime.
   - Make `.form` editing and `.gamestate` rendering use the same underlying format.
   - Extend `FormEditor` only in ways the runtime can understand.
3. Port score history from `pointsbox.cpp`.
   - Add a round-by-round scoreboard pane or modal.
   - Show per-round points and cumulative totals.
4. Port table animation intent from `HumanInterface::moveTable()`.
   - After trick resolution, animate cards toward the winner before clearing.
   - Keep the center table readable for a short pause.
5. Harden gameplay flow.
   - Add `New game` / `Restart round`.
   - Add end-of-round and end-of-game overlays.
   - Add smoke coverage for open, pass, resize, autoplay, and close.

Implementation rule:
- Prefer lifting layout and behavior directly from the 1.98 client, then adapt to current Python game logic and U++ drawing APIs.
- Keep the host-side C++ generic and move Hearts-specific rules into `reference/Hearts/main.py` unless the functionality is clearly reusable for other card games.
- Do not deepen the custom JSON `.form` fork; move toward the real `Form` runtime instead.
