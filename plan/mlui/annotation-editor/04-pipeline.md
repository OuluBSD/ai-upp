# AnnotationEditor → Training → Realtime Recognition Pipeline

## Overview

The AnnotationEditor is the hub: it produces training data and consumes model outputs.
The .mlui file is the schema connecting annotation, training, and runtime.

```
┌─────────────────────────────────────────┐
│  AnnotationEditor                       │
│  - manual annotation with slot_ids      │
│  - MluiScript defines slot vocabulary   │
│  - exports per-slot training datasets   │
└──────────────────┬──────────────────────┘
                   │ export (COCO / custom)
                   ▼
┌─────────────────────────────────────────┐
│  Training pipeline                      │
│  - per-slot object detector             │
│    (YOLO, DETR, or lightweight CNN)     │
│  - one model per .mlui file, or         │
│    one model for all slots (multi-class)│
└──────────────────┬──────────────────────┘
                   │ .pt / .onnx
                   ▼
┌─────────────────────────────────────────┐
│  Realtime recognizer                    │
│  Two-stage:                             │
│                                         │
│  Stage 1: Coarse .mlui match            │
│  - Embed image (CLIP or MobileNet)      │
│  - Compare vs reference_embedding       │
│    stored per .mlui file                │
│  - Returns: list of candidate scripts   │
│    with confidence scores               │
│                                         │
│  Stage 2: Fine localization             │
│  - For each candidate script:           │
│    run the per-script slot detector     │
│  - Returns: MluiMatch                   │
│    {script, slot_id, bbox, confidence}  │
└──────────────────┬──────────────────────┘
                   │ MluiMatch JSON
                   ▼
┌─────────────────────────────────────────┐
│  AnnotationEditor (import)              │
│  - MluiMatch → AnnotationObject.        │
│    suggestions with slot_id in metadata │
│  - Human reviews → accepts/rejects      │
│  - Accepted → training data for loop    │
└─────────────────────────────────────────┘
```

## MluiMatch struct (not yet implemented)

```cpp
struct MluiMatchInstance {
    String slot_id;
    Rectf  bbox;        // absolute pixels
    double confidence;
};

struct MluiMatchEntry {
    String script_path;
    double script_confidence;  // how well this script matches the image
    Vector<MluiMatchInstance> instances;
};

struct MluiMatch {
    String image_path;
    Vector<MluiMatchEntry> matches;
    void Jsonize(JsonIO& jio);
};
```

File extension: `.mluimatch` (JSON).
Import path: `File → Import → MLUI Match Results...`
Creates `AnnotationObject` entries in `ImageEntry::suggestions` with confidence < 1.

## Export format for training

Each slot maps to a category in training data.
Export options:
- COCO format with category = slot_id (already have COCO export)
- YOLO format (flat label files per image)
- Per-script export: only export images that have at least one slot annotated
  from a given script (to get a clean training set per script)

Training entry point: not in AnnotationEditor — external script / ConvNetCLI.
AnnotationEditor only handles export and suggestion import.

## Open questions

1. **One model per script vs shared multi-class model**
   - Per-script: smaller, more accurate, but N models to maintain
   - Multi-class: one model, slot_id as class label, more training data needed
   - Recommendation: start with multi-class, one model per dataset

2. **Reference embedding storage**
   - Store in .mlui file as `reference_embedding: [float, ...]`
   - Computed once from reference_image when script is finalized
   - Which embedding model? CLIP ViT-B/32 is practical (runs on CPU, ~150ms)

3. **Coarse matching threshold**
   - Cosine similarity > 0.7 = candidate (rough estimate)
   - Multiple scripts can match one image (intended)
   - User can override/lock script assignments per image

4. **Training loop trigger**
   - Manual: user exports and runs training externally
   - Future: "Train" button in AnnotationEditor triggers ConvNetCLI job
   - Even further future: background continuous learning

5. **ConvNetCLI role**
   - Headless runner for training and inference
   - Can be called with `--infer-mlui dataset_path script.mlui → matches.mluimatch`
   - Import result back into AnnotationEditor via File → Import
