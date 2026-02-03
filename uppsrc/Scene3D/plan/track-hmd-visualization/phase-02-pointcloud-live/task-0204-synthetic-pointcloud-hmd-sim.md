Task 0204 - Synthetic pointcloud + dummy HMD sim

Goal
- Create a synthetic test harness to validate localization logic with ideal data.

Checklist
- [ ] Generate a reference pointcloud with random points + two controller cylinders.
- [ ] Implement a dummy HMD stereo camera that renders synthetic stereo frames from the reference cloud using calibration.
- [ ] Feed synthetic frames into the localization stub to recover HMD pose.
- [ ] Feed cylinder detections into controller pose stub and associate with IDs.
- [ ] Add a debug view showing recovered pose vs ground truth.

Notes
- Keep all of this in test/sim code paths; no hardware required.
