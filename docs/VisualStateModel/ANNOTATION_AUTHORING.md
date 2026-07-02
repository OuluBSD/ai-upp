# VisualStateModel — Annotation Overlay Authoring Tools

## Overview

`FrameCanvas` supports direct annotation authoring on top of the frame
overlay. Users can create, select, and move annotation rectangles by dragging
with the mouse. Changes are saved through `VsmAnnotationLayer` and session
storage.

---

## Operations

### Create Annotation (drag-to-create)

Click and drag on an empty area of the canvas to define a new annotation
rectangle. On mouse-up, a new `VsmRegionAnnotation` is added with:

- `id`: `"ann-<tick>"` (auto-generated)
- `name`: `"New"` (editable in the Annotation Editor panel)
- `x`, `y`, `w`, `h`: derived from the drag rectangle

A minimum size of 8×8 pixels is enforced. The new annotation is automatically
selected. `WhenAnnotationCreated` fires → `OnAnnotationChanged()` → saves to
`annotation_path_`.

### Select Annotation (click)

Click inside an existing annotation rectangle to select it. The border turns
bright green and corner handles appear. `WhenRegionSelected(ann_id)` fires →
`MainWindow` updates `RegionPropsPanel` and logs the selection.

If click hits a changed-region box but no annotation, the region is selected
instead. Clicking empty space deselects all.

### Move Annotation (drag on selected)

Drag an annotation rectangle to reposition it. The live position is updated
on each `MouseMove`. On `LeftUp`, `WhenAnnotationMoved` fires →
`OnAnnotationChanged()` → saves to disk.

### Resize Annotation

Not yet implemented. Corner handles are drawn for visual feedback only.
Resize via the Annotation Editor panel (edit x/y/w/h fields directly).
Resize handles are planned as a follow-up task.

---

## Persistence

Authoring changes are saved immediately on `OnAnnotationChanged()`:

```cpp
void MainWindow::OnAnnotationChanged()
{
    auto errs = annotation_layer_.Validate();
    // log warnings...
    if(!annotation_path_.IsEmpty())
        annotation_layer_.Save(annotation_path_);
}
```

The annotation file is written to:
```
<session_root>/annotations/annotations.json
```

### Load on Session Open

When a session is opened via `File → Open Session…`:
- `annotations/annotations.json` is loaded if present.
- The annotation layer is set on both `AnnotationEditorPanel` and `FrameCanvas`.
- If no annotation file exists, the path is recorded for the first save.

---

## FrameCanvas API

```cpp
// Provide a non-const layer for authoring
frame_canvas_.SetAnnotationLayer(&annotation_layer_);

// Wire up authoring events
frame_canvas_.WhenAnnotationCreated = [=] { OnAnnotationChanged(); };
frame_canvas_.WhenAnnotationMoved   = [=] { OnAnnotationChanged(); };
```

---

## Limitations

- Resize via drag handles is not yet implemented.
- No undo/redo.
- `id` is generated from `GetTickCount()` — not guaranteed unique across
  restarts if the same session is edited quickly in multiple runs.
- Canvas coordinates are 1:1 pixel (no zoom/pan); large sessions may need
  scrolling (not yet implemented).
