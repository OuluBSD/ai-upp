# KDE Hearts Source Audit Results

## Gameplay Features
- **Players**: 4 (typically 1 human vs 3 AI).
- **Deck**: Standard 52 cards.
- **Round Flow**:
  1. Dealing: 13 cards per player.
  2. Passing: 3 cards passed according to rotation (Right, Left, Front, None).
  3. Opening: Player with 2 of Clubs leads.
  4. Tricks: Standard trick-taking rules. Must follow suit.
  5. Hearts Breaking: Hearts cannot be led until played on a previous trick or player has only hearts.
  6. Game Over: When a player reaches 100 points (KDE default).

## Scoring Rules
- **Each Heart**: 1 point.
- **Queen of Spades**: 13 points.
- **Shooting the Moon**: If one player takes all 26 points, they get 0 and everyone else gets 26.

## AI / Player Setup
- `computerplayer3_2.cpp` and `computerplayer4.cpp` implement logic for choosing which cards to pass and which to play.
- AI considers point cards, voiding suits, and defensive play against "Shooting the Moon".

## Asset Inventory
- **Card Images**: 72x96 PNGs.
- **Naming**: `[S|H|D|C][Rank].png`.
- **Card Back**: Present in `clients/human/pics/`.
- **Table**: "Table Green" (40, 160, 40) background.

## UI / State Model
- Centered table with 4 card slots for the active trick.
- Labels for player names and scores.
- Interactive hand at the bottom for human input.
- "Pass" button visible only during the passing phase.

## Gap Analysis & Reinterpretation
- **Networking**: KDE uses a client-server model. ScriptIDE will use a local ByteVM loop.
- **Drawing**: KDE uses Qt canvas. ScriptIDE will use U++ `Draw` and `.form` definitions.
- **Logic**: C++ logic will be rewritten in Python for ByteVM.

## Licensing
- Original code is GPL. Assets should be attributed to the KDE project.
