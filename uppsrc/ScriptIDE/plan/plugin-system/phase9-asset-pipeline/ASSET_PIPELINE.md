# Asset Import & Conversion Pipeline

## Source Material
- **Location**: `./tmp/hearts-1.98.tar.bz2` -> `clients/human/pics/`
- **Quantity**: 52 rank/suit images + card back + Joker (if present).
- **Format**: PNG, 72x96 pixels.

## Target Structure
Assets will be placed in the reference project directory:
`uppsrc/ScriptIDE/reference/Hearts/assets/`

### Renaming Mapping
To improve readability in Python/C++, assets will be renamed:
- `C2.png` -> `clubs_2.png`
- `Dj.png` -> `diamonds_jack.png`
- `Hq.png` -> `hearts_queen.png`
- `Sk.png` -> `spades_king.png`
- `back.png` -> `card_back.png`

## Extraction Script (Pseudo-code)
```bash
# Unpack
tar -xjf ./tmp/hearts-1.98.tar.bz2 -C ./tmp/hearts_assets
# Copy and rename logic
for suit in C D H S; do
  for rank in 2 3 4 5 6 7 8 9 10 a j q k; do
    cp "pics/${suit}${rank}.png" "Hearts/assets/${suit_full}_${rank_full}.png"
  done
done
```

## Integration with `.form`
The `.form` loader will use relative paths:
`image: "assets/hearts_queen.png"`

## Licensing & Attribution
A `CREDITS.md` file will be added to the assets folder:
"Card assets derived from KDE Hearts (2004) by Luis Pedro Coelho. Distributed under GPL."

## Verification
- [ ] All 52 cards present in target folder.
- [ ] Card back image present.
- [ ] Images open correctly in standard viewers.
