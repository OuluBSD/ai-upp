# Task 02 - Replace StereoCalibrationTool Video Backend

Status: completed

## Goal
Replace the buggy video input backend in StereoCalibrationTool with Draw/Video (and Draw/Camera where applicable), while preserving all features.

## Scope
- Move UsbStereoSource to Draw/Camera and back it with Draw/Video.
- Move HmdStereoSource to SoftHMD and register it for Draw/Camera.
- Keep VideoStereoSource behavior intact.
- Preserve all UI features, capture list, and headless test flags.

## Acceptance
- Live preview and capture work without crashes on /dev/video0.
- Formats/resolutions/ fps selection still available and accurate.
- Cmdline tests continue to work:
  - `--test-hmd --hmd-timeout-ms=<ms>`
  - `--test-usb --usb-device=/dev/videoX --usb-timeout-ms=<ms>`

## Notes
- Maintain feature parity and avoid regressions in calibration flow.
- Ensure any moved helpers remain in Geometry/ComputerVision as required.

## Progress
- Added Draw/Camera stereo source registry + helper split function.
- Added UsbStereoSource backed by Draw/Video.
- Added SoftHMD HmdStereoSource registration.
- StereoCalibrationTool now consumes Draw/Camera stereo sources.
- Moved PreviewCtrl to Ctrl/Camera and added StereoPreviewCtrl with context menu toggles.
- Camera tab uses StereoPreviewCtrl and can auto-load stcal for USB devices.
- Live preview prefers bright frames for HMD sources to avoid bright/dark flicker.
