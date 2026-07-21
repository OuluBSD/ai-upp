# ShaderTemplateMatcher Agent Notes

This is a headless contract and regression executable for the generic shader
template matcher in `uppsrc/VisualStateModel`. It must not contain recognition
logic duplicated from that package. Keep provider-specific assets outside the
repository and use `bin/build.exe` for builds.

The deterministic fixture can be generated without provider assets:

    bin\ShaderTemplateMatcher.exe --fixture tmp/m0295_recorded_manifest.json tmp/m0295_recorded_crop_map.vsm

The generated manifest/crop-map pair is suitable for the OCR-free recorded
pipeline regression only; it is not a production calibration.
