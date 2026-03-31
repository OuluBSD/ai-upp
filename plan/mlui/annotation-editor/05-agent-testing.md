# AnnotationEditor — Agent-Based Usability Testing

## Goal

After MLUI Focus pages are wired (see 02-focus-pages.md), connect agents via MCP
to test the AnnotationEditor from two perspectives:

1. **Application developer perspective**
   Can I script annotation of a batch of images without touching the mouse?
   Can I define a .mlui script and apply it to 10 images via MCP calls?

2. **End user perspective**
   Does the tool guide me through annotating a new dataset?
   Are the affordances discoverable via focus page descriptions?

## MCP server startup

```bash
bin/AnnotationEditor --mlui-server__ 127.0.0.1:8083
```

`--mlui-server__` is the flag consumed by `StartMluiRuntime()`.
`RegisterMluiRuntimeStarter()` can auto-register this flag in `GUI_APP_MAIN`.

## Expected agent command flow

```
mlui.focus.list
→ ["annotation_canvas", "object_list", "dataset_browser", "image_navigation", "mlui_script"]

mlui.focus.get annotation_canvas
→ {tool: "select", selected_id: -1, image_path: "", zoom: 1.0, actions: [...]}

mlui.focus.action dataset_browser.open_dataset {"index": 0}
mlui.focus.action image_navigation.open_image {"index": 0}
mlui.focus.get annotation_canvas
→ {image_path: "/data/...", annotation_count: 0, ...}

mlui.focus.action mlui_script.list_slots {}
→ [{slot_id: "header_bar", hint: "...", bbox_hint: {...}}, ...]

mlui.focus.action annotation_canvas.apply_script_slot {"slot_id": "header_bar", "bbox": {"x":0,"y":0,"w":1280,"h":60}}
mlui.focus.action annotation_canvas.apply_script_slot {"slot_id": "sidebar", "bbox": {"x":0,"y":60,"w":200,"h":660}}

mlui.focus.get object_list
→ {objects: [{id:1, slot_id:"header_bar", bbox:...}, {id:2, slot_id:"sidebar", bbox:...}]}

mlui.focus.action image_navigation.next_image {}
```

## Test scenarios

### Scenario 1: Developer batch annotation
- Open dataset with 10 images
- Load a .mlui script
- For each image: apply all slots with bbox from script hints scaled to image size
- Verify: all objects created, slot_ids set, bboxes reasonable

### Scenario 2: End user guided flow
- Agent calls `mlui.focus.list` — discovers available pages
- Agent reads `annotation_canvas` description — understands tool affordances
- Agent attempts to select a tool, annotate one object, undo it
- Verifies feedback at each step (selected_id changes, annotation_count changes)

### Scenario 3: Permission gating
- Agent tries `delete_selected` with nothing selected
- Expects: action disabled with reason "No object selected"
- Agent selects object, retries — succeeds

### Scenario 4: Script round-trip
- Agent creates a .mlui script via MluiScriptEditor (or by writing JSON directly)
- Agent applies it to first image → verifies stubs created
- Agent calls `copy_hints_from_image` → verifies bbox_hints written to script

## What to measure

- Actions attempted vs actions succeeded
- Number of MCP round-trips per annotation task
- Any action that produces unexpected state (regression indicator)
- Focus page descriptions rated for clarity by agent (ask agent to rate)
