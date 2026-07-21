# VideoLiveRecognitionLoopConsole Agent Notes

This package is the headless console entrypoint for the shader-evidence
descriptor path. The implementation is shared with
`reference/VideoLiveRecognitionLoop/main.cpp`; do not copy recognition or
decision logic into this package.

Build with `bin\build.exe`, never `script/build.py`. Keep diagnostics on
stdout/stderr and preserve the `--shader-evidence-frame-config` contract.
