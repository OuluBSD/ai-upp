# Manual Stereo Calibration Tool - DONE

## Phase 1: High-Perf Data Block Buffer
- Implement `LinkedList<RawDataBlock, true>` in `SoftHMD/Camera` - DONE
- Use `RWMutex` for granular locking between `AppendRaw` and `ProcessRawFrames` - DONE
- Fix AB-BA deadlock between `mutex` and `raw_mutex` - DONE
- Implement frame serial numbering for independent consumer tracking - DONE
- Add `PeakFrames` for non-destructive queue access - DONE

## Phase 2: Manual match point selection
- Implement `PreviewCtrl` with split-screen side-by-side view - DONE
- Add `VIS_FRAME_BRIGHT` filtering to eliminate flickering in live view - DONE
- Implement non-destructive `PeakFrame` polling for reliable bright frame capture - DONE
- Handle mouse clicks to select match point pairs (left then right) - DONE
- Draw green circles and index labels for selected match pairs - DONE
- Track match coordinates in `matches_list` - DONE

## Phase 3: Diagnostics and Project Persistence
- Add `--test-live` and `--verbose` arguments for systematic testing - DONE
- Implement verbose logging for synchronization and capture processes - DONE
- Fix frame backlog issue by consuming only latest bright frame in ReadFrame - DONE
- Add `last_serial` tracking to prevent redundant UI updates - DONE
- Implement Project Directory support (CLI and startup selection) - DONE
- Auto-save/load `project.json` (matches), `report.txt`, and PNG frames - DONE
- Support separate `dist_l` and `dist_r` (mm) per match pair - DONE
- Add "Edit" menu for removing snapshots and matches - DONE

## Phase 4: Calibration Solver
- Integrate `plugin/Eigen` for non-linear optimization - DONE
- Implement Bundle Adjustment solver minimizing reprojection and metric range errors - DONE
- Support fisheye distortion model (4th order polynomial) - DONE
- Auto-save solved parameters to `calibration.stcal` in project directory - DONE
- Implement automatic calibration loading in SoftHMD based on USB VID/PID (HP VR1000) - DONE

## Phase 5: Verification and Review (Next)
- [TODO] Implement Rectification Preview (show horizontal epipolar lines over stereo images).
- [TODO] Visual "Undistort" toggle in PreviewCtrl to check polynomial fit quality.
- [TODO] Residual heatmap or error overlay per match point.
- [TODO] Export to "production" path (`share/calibration/hp_vr1000/calibration.stcal`) with confirmation.