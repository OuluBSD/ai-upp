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

Next tasks:
1. Refine opponent hand presentation from the old human client.
   - Add better spacing/stacking rules per seat.
   - Add current-player highlighting around the active seat.
2. Port explicit passing interaction.
   - Add a visible `Pass cards` action similar to the old `QPushButton`.
   - Add `Clear selection`.
   - Disable pass until exactly 3 cards are selected.
3. Port stronger play-state feedback.
   - Highlight current player seat.
   - Show current leading suit and trick winner.
   - Distinguish invalid clicks in the table UI, not only the log pane.
4. Port score history from `pointsbox.cpp`.
   - Add a round-by-round scoreboard pane or modal.
   - Show per-round points and cumulative totals.
5. Port table animation intent from `HumanInterface::moveTable()`.
   - After trick resolution, animate cards toward the winner before clearing.
   - Keep the center table readable for a short pause.
6. Harden gameplay flow.
   - Add `New game` / `Restart round`.
   - Add end-of-round and end-of-game overlays.
   - Add smoke coverage for open, pass, resize, autoplay, and close.

Implementation rule:
- Prefer lifting layout and behavior directly from the 1.98 client, then adapt to current Python game logic and U++ drawing APIs.
- Keep the host-side C++ generic and move Hearts-specific rules into `reference/Hearts/main.py` unless the functionality is clearly reusable for other card games.
