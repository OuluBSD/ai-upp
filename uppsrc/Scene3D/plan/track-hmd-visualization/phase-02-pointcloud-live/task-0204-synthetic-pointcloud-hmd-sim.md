Task 0204 - Synthetic pointcloud + dummy HMD sim

Goal
- Create a synthetic test harness to validate localization logic with ideal data.

Checklist
- [x] Generate a reference pointcloud with random points + two controller cylinders.
- [x] Implement a dummy HMD stereo camera that renders synthetic stereo frames from the reference cloud using calibration.
- [x] Feed synthetic frames into the localization stub to recover HMD pose.
- [x] Feed cylinder detections into controller pose stub and associate with IDs.
- [x] Add a debug view showing recovered pose vs ground truth.

Notes
- Keep all of this in test/sim code paths; no hardware required.
