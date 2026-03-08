AGENTS

Scope
- Applies to `uppsrc/StereoCalibrationTool`.

Purpose
- Stereo camera calibration tool supporting HMD stereo cameras, side-by-side webcams, and prerecorded stereo videos.

Notes
- Keep calibration math and data formats in Geometry; the tool should be a UI/driver for capture and export.
- When created, ensure `AGENTS.md` is the first file listed in the package `.upp` manifest.
- Cmdline capture tests:
  - `--test-hmd --hmd-timeout-ms=<ms>` captures a single HMD frame and exits.
  - `--test-usb --usb-device=/dev/videoX --usb-timeout-ms=<ms>` captures a single USB frame and exits.
