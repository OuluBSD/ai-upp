# Phase 5: Rectification and Undistortion Preview

This phase focuses on visually verifying the solved calibration parameters before they are deployed to the tracking system.

## Task 1: Rectification Visualization
- Add a toggle `Show Epipolar Lines` to the UI.
- When enabled, `PreviewCtrl` should draw thin, horizontal red lines across the split-screen view.
- Purpose: Verify that matching points lie on the same horizontal scanline after (theoretical) rectification.
- Status: done (2026-01-29). Added Review toggle and epipolar line rendering in preview.

## Task 2: Real-time Undistortion Preview
- Add a toggle `Undistort View` to the UI.
- Use the solved `angle_to_pixel` polynomial to perform software-based undistortion.
- Since real-time undistortion can be heavy, implement it using a cached `Image` in `CapturedFrame` or a simplified re-sampling in `PreviewCtrl::Paint`.
- Purpose: Visually confirm that straight lines in the physical world (e.g., edges of a monitor or calibration board) appear straight in the preview.
- Status: done (2026-01-29). Cached undistort images per capture and per-live frame, using LensPoly mapping.

## Task 3: Residual Heatmap
- Draw error vectors (lines from measured pixel to reprojected pixel) for each match point.
- Color-code markers based on error magnitude (Green < 1px, Yellow < 3px, Red > 3px).
- Display the RMS (Root Mean Square) error in the status bar or report tab.
- Status: done (2026-01-29). Residual vectors and color coding drawn in preview; RMS displayed in status bar and overlay.

## Task 4: Production Export
- Add a button `Deploy Calibration`.
- This should copy `calibration.stcal` to `share/calibration/hp_vr1000/calibration.stcal` after a confirmation prompt.
- Ensure the directory is created if it doesn't exist.
- Status: done (2026-01-29). Deploy button writes to share calibration path with confirmation.

## Files to Modify:
- `uppsrc/StereoCalibrationTool/StereoCalibrationTool.h`: Add UI state and toggle declarations.
- `uppsrc/StereoCalibrationTool/StereoCalibrationTool.cpp`: Implement rendering logic and undistortion math.
- `uppsrc/Geometry/Camera.h/cpp`: Ensure `LensPoly` can be used efficiently for on-the-fly undistortion.
