# `.form` Layout Runtime Integration

## Status Correction
This document described the JSON-based layout runtime that was later implemented in
`CardGameDocumentHost::SetLayout()`. That path is now considered the wrong target.

Why:
- `bazaar/FormExample` shows the intended runtime path:
  - `FormWindow form;`
  - `form.Load(file);`
  - `form.Layout("Default");`
- the current Hearts `.form` path does not use that renderer at all
- `CardGameLayoutEditor` uses `FormEdit` as an editing surface, but serializes a custom
  JSON scene instead of real `Form` XML

Result:
- editor and runtime are on different formats
- `.form` no longer means "U++ Form file" in the card-game path
- layout bugs are harder to diagnose because the real runtime is bypassed

## Correct Loading Workflow
1. **Parser**: use the `Form` package runtime (`Form::Load`, `Form::Layout`) for `.form`.
2. **Instantiation**:
   - create real U++ controls from the `.form` content
   - host the generated form inside the `.gamestate` document host
3. **Overlay / Extensions**:
   - card-game-specific sprites, hit testing, and animation live above or beside the
     form runtime, not instead of it
4. **Linking**:
   - Python references named controls / containers / overlay zones through a stable bridge

## Python Integration
The game view should provide bindings that target the live `Form` runtime plus any
explicit overlay layer:

| Python Function | Description |
| :--- | :--- |
| `get_zone(id)` | Returns a rect / control handle for a named form element or overlay zone. |
| `move_card(card_id, target_zone_id)` | Animates a card moving to a target form zone / overlay zone. |
| `set_text(element_id, text)` | Updates a label or text-bearing control. |
| `clear_zone(id)` | Clears the overlay content associated with a zone. |

## Event Dispatch
Interaction events should be captured from the hosted form controls and from the
overlay layer, then dispatched to the Python VM as callback function calls.

## Synchronization
The hosted `Form` runtime plus overlay layer maintain the visual truth. The Python
script maintains the game-logic truth. Event-driven updates should keep them aligned.

## Deprecated Approach
The following approach is now deprecated:
- parse `.form` as ad hoc JSON
- store runtime objects as custom `Zone` structs only
- maintain a second layout engine unrelated to `Form`
