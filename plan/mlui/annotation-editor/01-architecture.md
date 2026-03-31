# AnnotationEditor — MLUI Architecture

## Repos and packages involved

```
ConvNetCpp/src/AnnotationEditor/   — the GUI app
ai-upp/uppsrc/Core/MluiScript/     — .mlui data structs (no GUI)
ai-upp/uppsrc/Ctrl/Mlui/           — GUI dialogs for .mlui editing
ai-upp/uppsrc/CtrlLib/Mlui.h/.cpp  — MluiFocusPage, RegisterMluiFocusPage, etc.
```

`CtrlLib` already includes `Mlui.h` — no extra .upp dependency for focus pages.
`AnnotationEditor.upp` uses `Ctrl/Mlui` for the script editor dialog.

## Main classes

| Class | File | Role |
|---|---|---|
| `AnnotateView` | main.cpp | Canvas + annotation state |
| `AnnotationEditorWindow` | main.cpp | DockWindow, menus, docked panels |
| `MluiScript` | Core/MluiScript/MluiScript.h | .mlui file data |
| `MluiScriptSlot` | Core/MluiScript/MluiScript.h | one named region template |
| `MluiScriptLink` | Core/MluiScript/MluiScript.h | image↔.mlui association |
| `SetGeometryDialog` | Ctrl/Mlui/MluiCtrls.h | bbox/polygon entry without drawing |
| `MluiScriptEditor` | Ctrl/Mlui/MluiCtrls.h | .mlui file editor dialog |

## Slot-to-annotation convention

Slot assignment stored on `AnnotationObject` via existing metadata arrays:
- `metadata_keys[i] == "mlui_slot_id"` → `metadata_values[i]` = slot_id
- `metadata_keys[i] == "mlui_script"` → `metadata_values[i]` = script path

Helper functions: `MluiSlotIdKey()`, `MluiScriptKey()` in MluiScript.h.

## Dataset↔script association

- `Dataset::mlui_script_path` — default script for all images in a dataset
- `ImageEntry::mlui_scripts` — per-image list of `MluiScriptLink` (path + confidence)
- Multiple .mlui files per image are intentional: large images can contain multiple UI layouts

## bbox_hint coordinate system

Normalized 0.0–1.0 (0 = left/top, 1 = right/bottom of image).
`Rectf(0,0,0,0)` = unset.

`MluiScriptReferenceImage` records the image used when hints were derived,
so hints can be re-normalized if the reference changes.

## MLUI Focus pages (planned — not yet wired)

See 02-focus-pages.md.
