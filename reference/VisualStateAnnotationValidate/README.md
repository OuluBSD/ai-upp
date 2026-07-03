# VisualStateAnnotationValidate

Console reference package demonstrating `VsmAnnotationLayer` validation.

Creates 2 synthetic annotation layers in-code — one valid (proper hierarchy, 
all rectangles within frame bounds) and one broken (with a hierarchy cycle 
and at least one rectangle out of bounds). Runs both the existing hierarchy 
validation (`Validate()`) and the new bounds check (`ValidateBounds()`) 
against each layer, prints results, and asserts that the valid layer reports 
zero issues while the broken layer reports both hierarchy and bounds violations.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateAnnotationValidate
bin\VisualStateAnnotationValidate.exe
```

See `docs/VisualStateModel/ANNOTATION_VALIDATOR.md` for design notes and API reference.
