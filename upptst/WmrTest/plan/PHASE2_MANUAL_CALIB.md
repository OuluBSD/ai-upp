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

## Phase 3: Diagnostics and State
- Add `--test-live` and `--verbose` arguments for systematic testing - DONE
- Implement verbose logging for synchronization and capture processes - DONE
- Fix frame backlog issue by consuming only latest bright frame in ReadFrame - DONE
- Add `last_serial` tracking to prevent redundant UI updates - DONE
- [TODO] Calibration Solve logic (Phase 2 extension)
- [TODO] .stcal persistence for point pairs (Phase 2 extension)
