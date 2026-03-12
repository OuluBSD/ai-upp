# Phase 12 — Hearts GUI Integration Plan

## Goal

Wire a live ScriptIDE widget (`HeartsView`) to the Hearts Python game, replacing the
headless stubs in `CardGamePlugin::SyncBindings()` with real UI calls.  The result is a
playable Hearts window that opens inside ScriptIDE when a `.gamestate` file is executed.

---

## Current State

| Layer | Status |
|---|---|
| Game logic (`hearts/logic.py`) | Complete, tested |
| AI (`hearts/ai.py`) | Complete, sorts correctly (closures fixed) |
| `main.py` bridge | Complete (`start`, `refresh_ui`, `on_click`, `autoplay_loop`) |
| Headless autoplay | **Working** — `bin/ScriptCLI run game.gamestate --autoplay` |
| Card assets | 52 cards + back PNG in `reference/Hearts/assets/` |
| Zone layout | `table.form` JSON defines all zones |
| `hearts_view` module | Headless stubs only — **to replace** |

---

## Architecture

```
ScriptIDE
└── HeartsView  (new Ctrl, owns card sprites + zone rects)
      │
      │  registered as dockable pane or standalone window
      │  opened by CardGamePlugin::Execute()
      │
      ▼
CardGamePlugin::SyncBindings(vm, heartsView*)
      │  replaces stub lambdas with real calls into HeartsView
      ▼
PyVM  ←──  hearts_view.set_card / move_card / get_zone_rect / clear_sprites / log
      │
      ▼
main.py  (start → refresh_ui → process_ai_turns → on_click)
```

---

## Implementation Phases

### Phase A — HeartsView Widget (ScriptIDE)

**File**: `uppsrc/ScriptIDE/HeartsView.h` / `HeartsView.cpp`

`HeartsView` is a `Ctrl`-derived widget that:

1. **Zone layout**: Parses `table.form` JSON on construction.
   Computes absolute pixel rects for each named zone given the widget size.
   Re-computes on `Layout()` override (anchors: CENTER, BOTTOM_CENTER, etc.).

2. **Card sprite map**: `VectorMap<String, CardSprite>` keyed by `card_id`.
   `CardSprite` holds: `Image img`, `Rect rect`, `bool selected`.

3. **API surface** (called by `CardGamePlugin` bindings):
   ```cpp
   // Place or update a card sprite at absolute pixel position
   void SetCard(const String& card_id, const String& asset_path, int x, int y);

   // Move card sprite to the center of a named zone
   void MoveCard(const String& card_id, const String& zone_id, int offset, bool animated);

   // Return the absolute pixel rect of a named zone as a dict-like struct
   struct ZoneRect { int x, y, w, h; };
   ZoneRect GetZoneRect(const String& zone_id);

   // Clear all card sprites
   void ClearSprites();

   // Append a line to the status/log area
   void Log(const String& msg);

   // Register click callback (called by CardGamePlugin after binding)
   Function<void(const String& card_id)> WhenCardClick;
   ```

4. **Painting** (`Paint` override):
   - Fill background green (#28A028 per `table.form`).
   - Draw all `CardSprite` images at their rects.
   - Draw selected cards 20px higher.
   - Draw zone outlines in debug mode (optional).

5. **Mouse handling** (`LeftDown` override):
   - Hit-test click against card sprites (back-to-front z-order).
   - Fire `WhenCardClick(card_id)`.

6. **Image loading**: Use `StreamRaster::LoadFileAny(asset_path)`.
   Cache loaded images to avoid re-reading disk on `refresh_ui()`.

---

### Phase B — CardGamePlugin GUI Wiring

**File**: `uppsrc/ScriptCommon/CardGamePlugin.cpp`

`CardGamePlugin` gains a `HeartsView*` pointer set by the IDE before `Execute()`.

```cpp
void CardGamePlugin::SetView(HeartsView* v) { view = v; }
```

`SyncBindings(vm)` gains an overload / conditional that wires real lambdas when `view != nullptr`:

```cpp
void CardGamePlugin::SyncBindings(PyVM& vm)
{
    if(view) {
        PY_MODULE(hearts_view, vm)
        PY_MODULE_FUNC(log,           [this](args){ view->Log(args[0]); }, nullptr)
        PY_MODULE_FUNC(clear_sprites, [this](args){ view->ClearSprites(); }, nullptr)
        PY_MODULE_FUNC(set_card,      [this](args){ view->SetCard(...); }, nullptr)
        PY_MODULE_FUNC(move_card,     [this](args){ view->MoveCard(...); }, nullptr)
        PY_MODULE_FUNC(get_zone_rect, [this](args){ return ZoneRectToDict(view->GetZoneRect(...)); }, nullptr)
    } else {
        // headless stubs (existing code)
    }
}
```

**Constraint**: `ScriptCommon` must not depend on `CtrlCore`.
`HeartsView` lives in `ScriptIDE`, so `CardGamePlugin` cannot `#include` it directly.
Solution: introduce a thin `IHeartsView` interface in `ScriptCommon/PluginInterfaces.h`:

```cpp
struct IHeartsView {
    virtual void SetCard(const String& id, const String& path, int x, int y) = 0;
    virtual void MoveCard(const String& id, const String& zone, int offset, bool anim) = 0;
    virtual Value GetZoneRect(const String& zone) = 0;   // returns Value dict
    virtual void ClearSprites() = 0;
    virtual void Log(const String& msg) = 0;
    virtual ~IHeartsView() {}
};
```

`HeartsView` implements `IHeartsView`. `CardGamePlugin` holds `IHeartsView*`.

---

### Phase C — ScriptIDE Launcher Integration

**File**: `uppsrc/ScriptIDE/PythonIDE.cpp`

When the user opens / double-clicks a `.gamestate` file:

1. Create `HeartsView` instance (or reuse existing docked pane).
2. Call `plugin->SetView(heartsView)`.
3. Show the view (dock it, or open as floating window).
4. Call `plugin->Execute(path)` — this calls `start()` in Python.
5. Wire `heartsView->WhenCardClick` to call `on_click(card_id)` in the VM.

**Execution model**: For the human-playable mode, `start()` returns after initial
`refresh_ui()` (no blocking loop). Subsequent UI events call back into Python via
`WhenCardClick`. For `--autoplay`, `start()` runs `autoplay_loop()` synchronously —
the GUI will need a thread or a timer-driven step loop (defer to Phase D).

---

### Phase D — Async / Animated Playback (stretch goal)

- Run `autoplay_loop()` on a background thread with GIL.
- Each `refresh_ui()` call posts a `Refresh()` to the GUI thread.
- Add a 200 ms timer between tricks for visual pacing.
- Animate `move_card()` with linear interpolation over N frames.

---

## File Checklist

| File | Action |
|---|---|
| `uppsrc/ScriptIDE/HeartsView.h` | Create |
| `uppsrc/ScriptIDE/HeartsView.cpp` | Create |
| `uppsrc/ScriptIDE/ScriptIDE.upp` | Add HeartsView.cpp |
| `uppsrc/ScriptCommon/PluginInterfaces.h` | Add `IHeartsView` interface |
| `uppsrc/ScriptCommon/CardGamePlugin.h` | Add `SetView(IHeartsView*)`, `IHeartsView* view` |
| `uppsrc/ScriptCommon/CardGamePlugin.cpp` | Replace stubs with real calls |
| `uppsrc/ScriptIDE/PythonIDE.cpp` | Launch HeartsView on `.gamestate` execute |
| `uppsrc/ScriptIDE/ScriptIDE.upp` | (already lists HeartsView if added above) |

---

## Zone Anchor Computation

`table.form` uses anchor strings to position zones relative to the widget:

| Anchor | Meaning |
|---|---|
| `CENTER` | Centered on widget |
| `BOTTOM_CENTER` | Horizontally centered, pinned to bottom |
| `CENTER_LEFT` | Vertically centered, pinned to left |
| `CENTER_RIGHT` | Vertically centered, pinned to right |
| `TOP_CENTER` | Horizontally centered, pinned to top |

`CONTAINER` zones with `anchor=CENTER` contain children with rects relative to the
container's computed top-left.

---

## Acceptance Criteria

- [ ] `bin/ScriptIDE` opens a Hearts table window when executing `game.gamestate`
- [ ] All 13 cards of Player 0 are rendered in the hand zone
- [ ] Clicking a card in the passing phase selects it (pops up 20 px)
- [ ] After 3 cards selected, pass executes and trick play begins
- [ ] AI players play their cards (visible in trick zones)
- [ ] Score display updates at round end
- [ ] Autoplay mode (via `--autoplay` CLI flag or menu) runs full game visually

---

## Start Point: Phase A (HeartsView widget)

Begin with `HeartsView.h` / `HeartsView.cpp` as a standalone `Ctrl` that:
- Accepts hard-coded zone rects initially (from `table.form` defaults)
- Renders PNG card images
- Fires `WhenCardClick`

Integrate with `CardGamePlugin` / ScriptIDE launcher in Phase B/C.
