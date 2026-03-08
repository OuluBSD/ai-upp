# Task 02 - Migrate api/Media Capture Backends

Status: completed

## Goal
Move relevant capture + device backend code from `uppsrc/api/Media` into Draw/Video / Draw/Camera, keeping `MediaAtomBase` in api/Media.

## Scope
- Evaluate `Capture_*` and `DeviceManager_*` files for stub/obsolete code.
- Move only usable backends; keep stubs in api/Media until replaced.
- Update `api/Media/Media.h` to include new locations.
- Use `git mv` for moved files.

## Acceptance
- Draw/Camera hosts usable capture/device backends.
- api/Media retains only `MediaAtomBase` + compatibility headers.

## Notes / Findings
- `Capture_V4L2.*`, `Capture_DShow.*`, `DeviceManager_Win32.*` are stubs or TODO-heavy.
- `DeviceManager_V4L2.*` and `Capture_OpenCV.*` depend on legacy api/Media types.

## Outcome
- Removed stub/legacy capture and device manager files from api/Media (to be reimplemented in api/Camera).
