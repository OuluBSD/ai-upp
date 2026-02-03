# Task 02 - Plan Outline

Status: pending

## Goal
Define the migration plan for CameraCtrl package and downstream integrations.

## Deliverables
- Task list with dependencies and acceptance checks.
- Identify files in WmrTest and WebcamRecorder to move/adapt.
- Clarify which parts of WebcamCV become optional effects adapters.
- Identify new CameraCtrl package skeleton (files, API surface, backends).

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
- `uppsrc/ComputerVision/*` stays in place; CameraCtrl will add adapters only.

## Proposed Package Skeletons
### CameraDraw (headless/core)
- `uppsrc/CameraDraw/CameraDraw.upp`
- `uppsrc/CameraDraw/CameraDraw.h` (public API)
- `uppsrc/CameraDraw/CameraBackend.{h,cpp}` (abstract backend interface + common types)
- `uppsrc/CameraDraw/BackendSoftHMD.{h,cpp}` (SoftHMD input adapter)
- `uppsrc/CameraDraw/BackendV4L2.{h,cpp}` (V4L2 input adapter)
- `uppsrc/CameraDraw/Effects.{h,cpp}` (optional processing hooks; no CV code moved)

### CameraCtrl (UI)
- `uppsrc/CameraCtrl/CameraCtrl.upp`
- `uppsrc/CameraCtrl/CameraCtrl.h` (public API)
- `uppsrc/CameraCtrl/CameraView.{h,cpp}` (generic preview + overlay rendering)
- Depends on `CameraDraw`

## Task Dependencies
- Phase02 (WmrTest extraction) depends on CameraDraw + CameraCtrl skeletons.
- Phase03 (WebcamRecorder) depends on V4L2 backend in CameraDraw and CameraCtrl UI.
- Phase04 (WebcamCV) depends on CameraDraw effects surface.
- Phase05 (StereoCalibrationTool) is blocked pending explicit approval.

## Notes
- Keep CameraDraw minimal at first (core capture + frame/overlay pipelines).
- CameraCtrl is UI only and should not include backend capture logic.
- WmrTest should keep tracking-specific UI and menu wiring local; only camera/overlay UI moves.
- WebcamRecorder keeps encode/output logic; only capture + preview moves to CameraCtrl.
- StereoCalibrationTool integration is blocked and requires explicit yes/no approval.
