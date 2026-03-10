# `.form` JSON Schema Design

## Format Overview
The `.form` format describes a 2D scene for card games. It is used by both the visual editor and the runtime renderer.

## Schema Hierarchy

### Root Object
- `name`: String (Title of the layout)
- `background_color`: Map (r, g, b)
- `zones`: List of Zone Objects
- `metadata`: Map (Custom attributes)

### Zone Object
Zones are spatial containers for cards or other entities.
- `id`: String (Unique name for Python referencing)
- `rect`: Map (x, y, w, h)
- `anchor`: String (e.g., `TOP_LEFT`, `CENTER`, `BOTTOM_RIGHT`)
- `type`: String (`HAND`, `TRICK`, `DECK`, `GENERIC`)
- `z_index`: Integer
- `items`: List of Sprite/Asset Objects (Initial state)

### Sprite/Asset Object
- `id`: String
- `image`: String (Path to asset)
- `rect`: Map (Local x, y, w, h)
- `rotation`: Double
- `visible`: Boolean

## Example File (`table.form`)
```yaml
name: "Standard Hearts Table"
background_color: {r: 40, g: 160, b: 40}
zones:
  - id: "south_hand"
    type: "HAND"
    rect: {x: 50, y: 400, w: 500, h: 100}
    anchor: "BOTTOM_CENTER"
  - id: "trick_area"
    type: "TRICK"
    rect: {x: 200, y: 150, w: 200, h: 200}
    anchor: "CENTER"
  - id: "north_name"
    type: "GENERIC"
    rect: {x: 250, y: 20, w: 100, h: 30}
    anchor: "TOP_CENTER"
```

## Referencing in Python
Python scripts can query zones by ID:
`hand = view.get_zone("south_hand")`
