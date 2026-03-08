AGENTS

Scope
- Applies to `upptst/WmrTest`.

Purpose
- WmrTest is the integration test app for SoftHMD camera + tracking + fusion.
- Planning artifacts for tracking work live under `upptst/WmrTest/plan/`.

Workflow
- Before changes, read `upptst/WmrTest/plan/cookie.txt` and relevant plan/task files.
- Update plan tasks as work progresses and record completions in `plan/cookie.txt`.
- Keep UI/debug toggles in WmrTest minimal and explicit; prefer menu-driven toggles.
- Interface naming convention: abstract interfaces use concrete names without a `Base` suffix; implementations are prefixed (e.g., `SoftHmdVisualTracker`).
- GeomEvent should be extended for VR/HMD/controller events; watch out for trivial/union constraints if required.
- Build and test before considering a task done:
  - Build: `python3 script/build.py -mc 1 -j12 WmrTest`
  - Run cmdline tests added under `WmrTest` (e.g., `--test-dump` or `--test-track`) and fix any failures.
  - StereoCalibrationTool cmdline capture checks:
    - `bin/StereoCalibrationTool --test-hmd --hmd-timeout-ms=8000`
    - `bin/StereoCalibrationTool --test-usb --usb-device=/dev/videoX --usb-timeout-ms=8000`
