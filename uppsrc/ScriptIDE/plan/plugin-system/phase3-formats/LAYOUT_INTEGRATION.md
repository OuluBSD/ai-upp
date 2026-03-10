# `.form` Layout Runtime Integration

## Loading Workflow
1. **Parser**: `LayoutLoader` class parses the JSON `.form` file using `uppsrc/Core`.
2. **Coordinate Resolution**:
   - Anchors are applied relative to the parent `Ctrl` (the game view tab).
   - Relative offsets are converted to absolute pixel coordinates.
3. **Instantiation**:
   - `Zone` objects are created as internal data structures within the `GameView` control.
   - Initial `Sprite` objects are loaded as `Image` assets.
4. **Linking**:
   - Named zones and sprites are stored in an `ArrayMap<String, ...>` for quick lookup.

## Python Integration
The `GameView` provides a set of ByteVM bindings to manipulate the loaded layout:

| Python Function | Description |
| :--- | :--- |
| `get_zone(id)` | Returns a handle to a zone. |
| `move_card(card_id, target_zone_id)` | Animates a card moving to a new zone. |
| `set_text(element_id, text)` | Updates a label (e.g., player name or score). |
| `clear_zone(id)` | Removes all items from a zone. |

## Event Dispatch
Interaction events (e.g., clicking a card) are captured by the `GameView` C++ code and dispatched to the Python VM as callback function calls (e.g., `on_card_clicked(card_id)`).

## Synchronization
The `GameView` maintains the "Visual Truth". The Python script maintains the "Game Logic Truth". Periodic sync or event-driven updates ensure they stay aligned.
