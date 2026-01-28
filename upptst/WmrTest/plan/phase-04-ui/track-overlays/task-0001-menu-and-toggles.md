Task 0001 - Menu and overlay toggles

Checklist
- Add MenuBar with App/View/Help.
- View toggles: show descriptors, show matches, show stereo split, show stats overlay.
- Define overlay rendering layers and priority.

Deliverables
- UI toggle spec and rendering plan.

UI toggle spec (WmrTest)
- App menu: start/stop capture, load/save calibration, exit.
- View menu:
  - Toggle descriptors (points only).
  - Toggle descriptor IDs.
  - Toggle match lines (stereo + temporal).
  - Toggle stats overlay (fps, keypoints, matches).
  - Toggle split view (bright/dark).
- Help menu: about + controls.

Overlay layers (order)
1) Raw image (bright/dark).
2) Descriptors (points).
3) Match lines (stereo/temporal).
4) Stats/labels (text).
