# VisualStateModel — Annotation Validator

## Overview

`VsmAnnotationLayer` provides validation methods to check annotation correctness.
Two types of validation are supported:

1. **Hierarchy Validation** (`Validate()`): Checks for missing parents, empty IDs, 
   zero-sized rectangles, and cycles in the parent-child hierarchy.
2. **Bounds Validation** (`ValidateBounds()`): Checks that all annotation rectangles 
   fit within the session's frame dimensions.

This is useful for ensuring annotations are well-formed before use in authoring tools
or for detecting stale/invalid annotations in stored `.json` files.

---

## API

### Hierarchy Validation

```cpp
Vector<VsmAnnotationLayer::ValidationError> layer.Validate() const;
```

Checks the annotation hierarchy for structural issues:
- Empty annotation IDs
- Missing parent annotations (dangling references)
- Zero or negative rectangle dimensions
- Cycles in the parent-child chain (annotation chain that loops back to itself)

### Bounds Validation

```cpp
Vector<VsmAnnotationLayer::ValidationError> layer.ValidateBounds(int frame_w, int frame_h) const;
```

Checks that each annotation rectangle is fully contained within the session's frame:
- Top-left corner `(x, y)` must be >= `(0, 0)`
- Bottom-right corner `(x + w, y + h)` must be <= `(frame_w, frame_h)`

### Result Type

Both methods return `Vector<ValidationError>`. Each error has:

```cpp
struct ValidationError {
    VsmAnnotationId annotation_id;  // ID of the offending annotation
    String          message;        // Human-readable error message
};
```

---

## Validation Rules

### Hierarchy (`Validate()`)

An annotation layer is considered valid if:

- Every annotation has a non-empty `id`.
- Every `parent_id` (if non-empty) refers to an existing annotation in the layer.
- Every annotation has `w > 0` and `h > 0`.
- No annotation's parent chain forms a cycle (no annotation is its own ancestor).

### Bounds (`ValidateBounds()`)

An annotation layer is considered valid if:

- For each annotation with rect `(x, y, w, h)`:
  - `x >= 0` and `y >= 0`
  - `x + w <= frame_w` and `y + h <= frame_h`

---

## Result Interpretation

### No Issues Found

```cpp
auto errs = layer.Validate();
if(errs.IsEmpty()) {
    Cout() << "Hierarchy is valid\n";
}

auto bounds_errs = layer.ValidateBounds(640, 480);
if(bounds_errs.IsEmpty()) {
    Cout() << "All annotations fit within frame\n";
}
```

### Handling Issues

```cpp
auto errs = layer.Validate();
for(const auto& err : errs) {
    Cout() << "Annotation " << err.annotation_id << ": " << err.message << "\n";
}
```

### Combined Validation

```cpp
Vector<VsmAnnotationLayer::ValidationError> all_errs;
all_errs.AppendAll(layer.Validate());
all_errs.AppendAll(layer.ValidateBounds(frame_width, frame_height));

if(all_errs.IsEmpty()) {
    Cout() << "Annotation layer is fully valid\n";
} else {
    Cout() << "Found " << all_errs.GetCount() << " issues\n";
}
```

---

## Non-Goals

- Does **not** validate annotation metadata (names, anchor points, hotspots).
- Does **not** validate linked region IDs or fingerprints (those are optional
  authoring hints).
- Does **not** validate binding types or references.
- Does **not** auto-repair invalid annotations — validation is read-only.

---

## Reference Tool

`reference/VisualStateAnnotationValidate/` demonstrates both validation methods:

1. Creates 2 synthetic annotation layers in-code (no file I/O).
2. Creates a valid layer with proper hierarchy and all rectangles in bounds.
3. Creates a broken layer with a cycle and at least one out-of-bounds rectangle.
4. Runs both `Validate()` and `ValidateBounds()` on each layer.
5. Prints validation results.
6. Asserts the valid layer reports zero issues and the broken layer reports
   both hierarchy and bounds violations.

```sh
bin/build.exe -m 7 -j12 VisualStateAnnotationValidate
bin\VisualStateAnnotationValidate.exe
```

Expected output:

```
=== VisualStateModel Annotation Validator Demo ===

Frame dimensions: 640x480

Creating valid annotation layer...
  Created with 2 annotations (root + child)
  Hierarchy validation: 0 issues
  Bounds validation: 0 issues
  OK: Valid layer passed both checks

Creating broken annotation layer (cycle + out-of-bounds rect)...
  Created with 3 annotations (A -> C -> B -> A cycle + C out of bounds)
  Hierarchy validation: 1 issues
    [ann-a] Hierarchy cycle detected
  Bounds validation: 1 issues
    [ann-c] Annotation rect [700,500,800,600] outside bounds [0,0,640,480]
  OK: Broken layer detected both cycle and bounds issues

=== All validation checks passed ===
```

---

## Design Notes

- Validation is **read-only** — no annotations are modified.
- `Validate()` checks structural properties (hierarchy, IDs, sizes).
- `ValidateBounds()` checks geometric properties (frame fit).
- Both methods return the same `ValidationError` type for consistency.
- Methods can be called independently or in sequence for comprehensive checking.
- Cycle detection uses depth-first traversal from each node; max depth is
  `annotations.GetCount() + 1` to detect cycles quickly.
