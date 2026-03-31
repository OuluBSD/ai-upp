# MluiScript — Design Notes

## What a .mlui file is

A JSON template declaring the named regions (slots) that should exist in an
annotated image of a particular UI layout archetype.

One .mlui file = one layout type.  An image may link to multiple .mlui files
(large screenshots containing multiple UI panels each matching a different script).

## File format (.mlui = JSON)

```json
{
  "format": "AnnotationEditorScript",
  "version": 1,
  "name": "Dashboard Layout",
  "description": "Standard dashboard screenshot with header, sidebar, and content",
  "author": "...",
  "reference_image": {
    "file_path": "/data/screenshots/dashboard_001.png",
    "width": 1280,
    "height": 720
  },
  "slots": [
    {
      "slot_id": "header_bar",
      "label": "Header Bar",
      "category": "ui_element",
      "hint": "Top navigation bar spanning full width",
      "bbox_hint": {"left": 0.0, "top": 0.0, "right": 1.0, "bottom": 0.083},
      "required": true,
      "allow_multiple": false,
      "metadata_keys": [],
      "metadata_values": []
    },
    {
      "slot_id": "sidebar",
      "label": "Left Sidebar",
      "category": "ui_element",
      "hint": "Vertical navigation sidebar",
      "bbox_hint": {"left": 0.0, "top": 0.083, "right": 0.156, "bottom": 1.0},
      "required": false,
      "allow_multiple": false
    }
  ]
}
```

## bbox_hint coordinates

- Normalized: 0.0 = left/top edge, 1.0 = right/bottom edge of image
- `{0,0,0,0}` = unset (hint not derived yet)
- `reference_image` records the image dimensions used when hints were written
- Agents use absolute pixel coords in action calls; hints are only suggestions

## How slot_id links annotations to slots

`AnnotationObject` carries the link in its existing metadata arrays:
- key `"mlui_slot_id"` → value = slot_id string
- key `"mlui_script"` → value = path to the .mlui file

## Multiple .mlui per image

`ImageEntry::mlui_scripts` is a `Vector<MluiScriptLink>`.
Each `MluiScriptLink` has a `script_path` and a `confidence` (1.0 = manual, <1.0 = model guess).

Use cases:
- Large screenshot containing two separate UI layouts
- Overlapping dialogs matching different scripts
- Model proposes candidate scripts; human confirms

## Workflow for building a script

1. Annotate first image manually (draw bboxes, assign categories)
2. In MluiScriptEditor: add slots, set slot_ids on each object via metadata
3. Click "Copy hints from image" — derives normalized bbox_hints from current bboxes
4. Save → .mlui file

OR via MCP agent:
1. Agent calls `list_slots` — gets slot definitions
2. Agent calls `apply_script_slot(slot_id, bbox)` for each slot in current image
3. Hints update automatically after annotations are accepted

## Workflow for applying a script to new images

1. AnnotationEditor opens image, sees linked .mlui files (from dataset default or per-image)
2. Object list shows unfilled slots as ghosted entries (future UI enhancement)
3. "Apply Script" creates stubs with bbox scaled from hints × image size
4. Human reviews, adjusts geometry
5. Or: model-generated suggestions imported as AnnotationObject.suggestions → human accepts

## Future: model integration

See 04-pipeline.md for the full annotation→training→recognition loop.
Relevant additions to MluiScript struct (not yet implemented):
- `Vector<double> reference_embedding` — CLIP embedding of reference image for coarse matching
- `MluiMatch` struct — model output: which scripts match an image and where
