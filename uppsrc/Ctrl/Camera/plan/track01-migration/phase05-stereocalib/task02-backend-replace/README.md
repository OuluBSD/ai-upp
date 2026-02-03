# Task 02 - Replace StereoCalibrationTool Video Backend

Status: pending

## Goal
Replace the buggy video input backend in StereoCalibrationTool with Draw/Video (and Draw/Camera where applicable), while preserving all features.

## Scope
- Replace UsbStereoSource capture backend (currently V4l2Capture) with Draw/Video backend.
- Keep HmdStereoSource and VideoStereoSource behavior intact.
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
