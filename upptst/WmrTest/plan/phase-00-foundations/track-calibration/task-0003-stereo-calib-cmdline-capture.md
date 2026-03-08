Task 0003 - StereoCalibrationTool cmdline capture tests
Status: done

Goals
- Add CLI switches to capture a single frame from HMD and USB stereo sources.
- Ensure the GUI still starts so the same preview/capture flow is used.
- Auto-exit with success on a non-black frame; exit with failure on timeout.

Acceptance
- `StereoCalibrationTool --test-hmd --hmd-timeout-ms=8000` captures a non-black frame when HMD is available.
- `StereoCalibrationTool --test-usb --usb-device=/dev/videoX --usb-timeout-ms=8000` captures a non-black frame from a webcam.
- On failure/timeout, the app exits and releases the device.
