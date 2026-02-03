# Task 02 - Adapt WebcamRecorder to Ctrl/Camera (Legacy Preserved)

Status: done

## Goal
Refactor reference/WebcamRecorder to use Draw/Video + Ctrl/Camera for capture and preview, while preserving legacy direct V4L2 under flagLEGACY.

## Scope
- Add Draw/Video backend option and CLI for automated tests.
- Use Ctrl/Camera for preview UI and overlay stats.
- Keep direct V4L2 path behind flagLEGACY for comparisons.

## Acceptance
- WebcamRecorder compiles and captures at least as fast as before.
- Preview path uses Ctrl/Camera UI components.
