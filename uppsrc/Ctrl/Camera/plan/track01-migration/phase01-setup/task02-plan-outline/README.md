# Task 02 - Plan Outline

Status: completed

## Goal
Define the migration plan for Ctrl/Camera package and downstream integrations.

## Deliverables
- Task list with dependencies and acceptance checks.
- Identify files in WmrTest and WebcamRecorder to move/adapt.
- Clarify which parts of WebcamCV become optional effects adapters.
- Identify new Ctrl/Camera package skeleton (files, API surface, backends).

## Source File Inventory (Initial)
### WmrTest
- `upptst/WmrTest/WmrTest.cpp` (camera preview UI, overlays, tracking views, background thread)
- `uppsrc/SoftHMD/Camera.{h,cpp}` (USB camera capture pipeline)
- `uppsrc/SoftHMD/TrackingImpl.{h,cpp}` (SoftHMD visual tracker + fusion adapter)
- `uppsrc/SoftHMD/Tracking.{h,cpp}` (StereoTracker + overlays/pointcloud)
- `uppsrc/SoftHMD/System.{h,cpp}` (HMD pose/controller updates used by WmrTest)

### WebcamRecorder
- `reference/WebcamRecorder/WebcamRecorder.cpp` (V4L2 capture, device/format UI, preview)
- `reference/WebcamRecorder/WebcamRecorder.upp`
- `plugin/libv4l2/*` (V4L2 capture helper; used via `V4l2Capture` and `V4l2DeviceParameters`)
- `plugin/jpg/*` (MJPEG decode path via `JPGRaster`)

### WebcamCV
- `reference/WebcamCV/WebcamCV.h` (effects classes + UI host)
- `reference/WebcamCV/WebcamCV.cpp` (demo wiring, frame loop)
- `reference/WebcamCV/YAPE*.cpp` (feature demo)
- `uppsrc/ComputerVision/*` stays in place; Ctrl/Camera will add adapters only.

### api/Media (candidate for Draw/Video)
- `uppsrc/api/Media/Media.h` and core types (`Types.*`, `MediaStream.*`, `MediaAtomBase.*`)
- Capture backends: `Capture_V4L2.*`, `DeviceManager_V4L2.*`, `Capture_DShow.*`, `DeviceManager_Win32.*`, `Capture_OpenCV.*`
- Audio/Video helpers: `Audio.*`, `Video.*`, `FileIn.*`

## Proposed Package Skeletons
### Draw/Video (shared headless base)
- `uppsrc/Draw/Video/Video.upp`
- `uppsrc/Draw/Video/Video.h` (public API)
- `uppsrc/Draw/Video/VideoTypes.{h,cpp}` (frame/format types)
- `uppsrc/Draw/Video/VideoBackend.{h,cpp}` (capture backend interface)
- `uppsrc/Draw/Video/VideoEffects.{h,cpp}` (optional processing hooks)

### Draw/Camera (headless/core)
- `uppsrc/Draw/Camera/Camera.upp`
- `uppsrc/Draw/Camera/Camera.h` (public API)
- `uppsrc/Draw/Camera/CameraBackend.{h,cpp}` (adapter to Draw/Video backend interface)
- `uppsrc/Draw/Camera/BackendSoftHMD.{h,cpp}` (SoftHMD input adapter)
- `uppsrc/Draw/Camera/BackendV4L2.{h,cpp}` (V4L2 input adapter)
- `uppsrc/Draw/Camera/Effects.{h,cpp}` (optional processing hooks; no CV code moved)

### Ctrl/Camera (UI)
- `uppsrc/Ctrl/Camera/Camera.upp`
- `uppsrc/Ctrl/Camera/Camera.h` (public API)
- `uppsrc/Ctrl/Camera/CameraView.{h,cpp}` (generic preview + overlay rendering)
- Depends on `Draw/Camera`

## Task Dependencies
- Phase01 Task03 defines Draw/Video base and api/Media migration.
- Phase02 (WmrTest extraction) depends on Draw/Video + Draw/Camera + Ctrl/Camera skeletons.
- Phase03 (WebcamRecorder) depends on V4L2 backend in Draw/Video/Draw/Camera and Ctrl/Camera UI.
- Phase04 (WebcamCV) depends on Draw/Video effects surface.
- Phase05 (StereoCalibrationTool) is blocked pending explicit approval.

## Notes
- Keep Draw/Video minimal and headless; focus on high-performance capture/processing interfaces.
- Draw/Camera is headless and should adapt Draw/Video interfaces for camera-specific needs.
- Ctrl/Camera is UI only and should not include backend capture logic.
- WmrTest should keep tracking-specific UI and menu wiring local; only camera/overlay UI moves.
- WebcamRecorder keeps encode/output logic; only capture + preview moves to Ctrl/Camera.
- StereoCalibrationTool integration is blocked and requires explicit yes/no approval.

## Progress
- Plan executed; tasks tracked in phase02+.
