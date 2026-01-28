Task 0002 - Stereo calibration app (generic)

Checklist
- Support live stereo webcams with side-by-side frames.
- Support HMD stereo camera input.
- Support prerecorded stereo videos/files.
- Define stub interfaces for non-live sources.

Deliverables
- App spec (UI + input sources) and data flow.

App spec (UI)
- Tabs: Sources | Capture | Solve | Review | Export.
- Sources: select input type (HMD, USB stereo, file), resolution, fps, side-by-side split settings.
- Capture: live view + frame quality indicator + sample count; store accepted frames.
- Solve: run calibration, show residuals and coverage.
- Review: preview undistort + rectification, overlay reprojection errors.
- Export: save JSON/.cfg to `share/calibration/<device-id>/stereo.json`.

Input sources (stubs)
- HMD camera: via SoftHMD/USB capture (frame + timestamp).
- USB stereo: single stream split into L/R by width/2 (offset configurable).
- Video file: simple frame reader stub; assume side-by-side format.

Data flow
1) Source -> Frame splitter -> L/R images.
2) Detect checkerboard/aruco -> collect samples.
3) Solve intrinsics/extrinsics -> compute baseline, FOV, distortion.
4) Produce runtime calibration struct + export file.
