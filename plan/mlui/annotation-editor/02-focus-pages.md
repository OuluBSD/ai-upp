# AnnotationEditor — MLUI Focus Pages

## Status: not yet implemented

Focus pages expose the AnnotationEditor's state and actions to MCP agents.
`AnnotationEditorWindow` needs an `Access(Visitor& v)` override and an `INITBLOCK`.

## Planned pages

### `annotation_canvas`
Primary working surface.

State to expose:
- `selected_id` — currently selected annotation id (-1 = none)
- `current_tool` — "select" | "bbox" | "polygon" | "brush" | "eraser" | "keypoint" | "review"
- `zoom` — current zoom level
- `image_path` — currently open image file path
- `image_index` — index within dataset
- `annotation_count`, `suggestion_count`

Actions:
- `select_tool(tool: string)`
- `select_object(id: int)`
- `delete_selected()`
- `undo()`, `redo()`
- `center_view()`
- `apply_script_slot(slot_id: string, bbox: {x,y,w,h})` — create/update slot object with given pixel bbox

### `object_list`
Object tree panel.

State: `cursor_id`, `object_count`, `suggestion_count`, `keypoint_count`

Actions:
- `select_row(id: int)`
- `delete_object(id: int)`
- `set_geometry(id: int, bbox: {x,y,w,h})` — change geometry without drawing
- `add_object(category: string, bbox: {x,y,w,h})` — create new object

### `dataset_browser`
Dataset and image navigation.

State: `dataset_name`, `dataset_path`, `image_count`, `current_index`, `image_path`

Actions:
- `open_dataset(index: int)`
- `open_image(index: int)`

### `image_navigation`
Prev/next within a dataset.

State: `can_prev`, `can_next`, `current_index`, `total_images`

Actions:
- `prev_image()`, `next_image()`
- `jump_to_best()` — smart-next by priority score

### `mlui_script`
Script template management.

State: `script_path`, `slot_count`, `unfilled_slots` (list of slot_ids not yet in current image)

Actions:
- `apply_script()` — create stubs for all unfilled slots
- `apply_slot(slot_id, bbox)` — create/update one specific slot
- `list_slots()` — return slot definitions with hints
- `copy_hints_from_image()` — update script hints from current annotations

## Implementation outline

```cpp
// In AnnotationEditorWindow:

INITBLOCK {
    MLUI::RegisterFocusPage("annotation_canvas", "Annotation Canvas",
        "Main image editing surface with tools and selected object state")
        .Workflow(1, "Open image, select tool, annotate objects")
        .Action("select_tool",   "Select active tool", "Switch annotation mode")
        .Action("select_object", "Select object",      "Highlight object by id")
        .Action("delete_selected","Delete selected",   "Remove current selection")
        .Action("undo",          "Undo",               "Undo last action")
        .Action("redo",          "Redo",               "Redo last undone action")
        .Action("apply_script_slot", "Apply script slot", "Create/move slot object");

    MLUI::RegisterFocusPage("object_list", "Object List", ...)
        ...;
    MLUI::RegisterFocusPage("image_navigation", "Image Navigation", ...)
        ...;
    MLUI::RegisterFocusPage("mlui_script", "MLUI Script", ...)
        ...;
}

virtual bool Access(Visitor& v) override {
    bool base = DockWindow::Access(v);
    if(auto* av = dynamic_cast<AutomationVisitor*>(&v)) {
        auto& canvas = MLUI::GetFocusPage("annotation_canvas");
        canvas.ClearRuntime();
        MLUI_USE_STATE(canvas, "selected_id",   annotate_view.GetSelectedId(),  "...");
        MLUI_USE_STATE(canvas, "current_tool",  annotate_view.GetToolName(),    "...");
        MLUI_USE_STATE(canvas, "zoom",          annotate_view.GetZoom(),        "...");
        MLUI_USE_STATE(canvas, "image_path",    annotate_view.GetImagePath(),   "...");
        MLUI_USE_ACTION(canvas, "select_tool",  true, "Switch annotation mode");
        ...
        return true;
    }
    return base;
}
```

Action handlers wired in constructor:
```cpp
MLUI::GetFocusPage("annotation_canvas")
    .ActionHandler("select_tool", [this](const ValueMap& args) -> Value {
        String tool = args["tool"];
        annotate_view.SetToolByName(tool);
        return ValueMap();
    })
    .ActionHandler("select_object", [this](const ValueMap& args) -> Value {
        annotate_view.SelectById((int)args["id"]);
        return ValueMap();
    })
    ...;
```

## Getters needed on AnnotateView

These public methods need to be added (currently state is private):
- `int  GetSelectedId() const`
- `String GetToolName() const`  — maps ToolType enum → string
- `double GetZoom() const`
- `String GetImagePath() const`
- `int  GetImageIndex() const`
- `void SelectById(int id)` — programmatic selection
- `void SetToolByName(const String& name)`
