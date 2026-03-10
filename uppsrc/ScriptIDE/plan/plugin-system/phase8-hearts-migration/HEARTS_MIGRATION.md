# Hearts Python Migration Plan

## Module Structure
The game will be organized into a `hearts` package:
- `logic.py`: Core rules (trick resolution, scoring, passing).
- `player.py`: Player base class and human/AI implementations.
- `ai.py`: Ported KDE AI heuristics.
- `bridge.py`: Glue code connecting logic to ScriptIDE C++ bindings.
- `main.py`: Entry point called by `.gamestate`.

## Logic Mapping
| KDE C++ Concept | Python implementation |
| :--- | :--- |
| `GameManager` | `hearts.logic.MatchManager` |
| `computerplayer3_2.cpp` | `hearts.ai.SmartAI` |
| `points_for(Card)` | `hearts.logic.calculate_points(cards)` |
| `Shooting the moon` | `MatchManager.resolve_round_scores()` |

## UI-to-Logic Interaction
Python logic will reference UI elements by the IDs defined in `.xlay`:
```python
import hearts_view # C++ bound module

def on_card_played(card_id):
    zone = hearts_view.get_zone("trick_area")
    hearts_view.move_card(card_id, "trick_area")
```

## AI Migration Strategy
1. **Analyze**: Transcribe heuristics from `clients/computer/computerplayerbase.cpp`.
2. **Implement**: Create a stateless `choose_card(hand, trick, round_state)` function in Python.
3. **Verify**: Use unit tests to ensure the AI chooses valid and "smart" cards compared to the original.

## Turn Resolution
The game loop will be event-driven to avoid blocking the IDE thread:
1. `wait_for_human_input()` (yields control).
2. Human clicks card -> C++ calls Python `on_card_clicked`.
3. Python validates, updates state, and triggers AI turns.
4. AI turns are processed with small delays (simulated) to show animations.
