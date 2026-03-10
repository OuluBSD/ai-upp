# FormEditor Adaptation Strategy for `.xlay`

## Overview
We will adapt U++'s native `FormEditor` to serve as a WYSIWYG designer for card game tables.

## Reusable Subsystems
- **Property Grid**: Can be used to edit `id`, `rect`, `anchor`, and custom metadata.
- **Manipulation Logic**: Selection, dragging, resizing, and snapping.
- **Undo/Redo**: Core stack can be reused.

## Required Forks/Changes
Standard `FormEditor` is tied to `lay` files and `Ctrl` classes. For `.xlay`, we need:
1. **Custom Serializer**: Replace the `lay` file generator with a YAML serializer/deserializer matching the `.xlay` schema.
2. **Game-Specific Toolbox**: Instead of `Button` or `EditField`, provide `Zone`, `CardPlaceholder`, and `Sprite`.
3. **Z-Order Management**: Visual layering controls (Bring to Front, Send to Back).
4. **Anchor Visualization**: Show coordinate origins (e.g., `BOTTOM_CENTER`).

## Implementation Model: `GameLayoutEditor`
We will create a new class `GameLayoutEditor` that inherits from `FormEditor` or copies its core logic to avoid breaking standard U++ dialog editing.

### Toolbox Mapping
| Toolbox Item | Underlying U++ Class | `.xlay` Type |
| :--- | :--- | :--- |
| Hand Zone | `StaticRect` (styled) | `HAND` |
| Trick Area | `StaticRect` (styled) | `TRICK` |
| Asset/Sprite | `ImageCtrl` | `SPRITE` |

## Preview vs. Runtime
- **Editor**: Uses bounding boxes, handles, and labels for IDs.
- **Runtime**: Renders clean assets without editor gizmos.
- The `GameLayoutEditor` will produce the same YAML that the `LayoutLoader` consumes.
