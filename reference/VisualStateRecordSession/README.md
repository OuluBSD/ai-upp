# VisualStateRecordSession

Console reference package demonstrating `VsmCaptureSink`.

Builds a small synthetic source session, opens it as a `VsmFrameSource` via
`VsmSessionStoreSource`, records it into a new session directory with
`VsmCaptureSink`, then verifies the recorded session:

- opens and replays through `VsmSessionStoreSource` again,
- runs the `VsmObservationPipeline` against it (OCR rule check).

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateRecordSession
bin/VisualStateRecordSession.exe
```

See `docs/VisualStateModel/CAPTURE_SINK.md` for the capture sink design notes.
