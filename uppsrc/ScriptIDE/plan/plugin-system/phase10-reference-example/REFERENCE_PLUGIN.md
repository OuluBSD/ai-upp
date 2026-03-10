# Reference Card-Game Plugin & Hearts Project

## Plugin Implementation: `CardGamePlugin`
- **Class**: `CardGamePlugin : public IPlugin`
- **Responsibilities**:
  - Registers `.gamestate` and `.xlay` file extensions.
  - Provides `CardGameDocumentHost` (Runtime View).
  - Provides `CardGameLayoutEditor` (FormEditor adaptation).
  - Implements `IPythonBindingProvider` for the `hearts_view` module.
  - Implements `ICustomExecuteProvider` for launching `.gamestate`.

## Custom View: `CardGameView`
- **Derived from**: `Ctrl`, `IDocumentHost`.
- **Functionality**: 
  - Loads `.xlay` YAML via `LayoutLoader`.
  - Maintains a list of active card sprites and zones.
  - Handles `Paint` by drawing the background color and all active sprites.
  - Captures mouse clicks and forwards them to Python.

## Hearts Example Project Structure
`uppsrc/ScriptIDE/reference/Hearts/`
- `game.gamestate`: Main entry.
- `table.xlay`: Visual layout.
- `main.py`: Entry script.
- `hearts/`: Python package (logic, ai, bridge).
- `assets/`: 52 card PNGs + card back.

## Minimal API Surface (C++ to Python)
```python
import hearts_view
hearts_view.clear_trick()
hearts_view.set_slot(player_index, card_id) # Animates card to player's slot in trick area
hearts_view.update_score(player_index, score)
hearts_view.show_message("Player 1 leads with 2 of Clubs")
```

## Demonstration Goals
1. Open `table.xlay` -> Show visual editor.
2. Open `game.gamestate` -> Show static game view.
3. Press F5 on `game.gamestate` -> Game starts, AI plays cards, animations trigger.
