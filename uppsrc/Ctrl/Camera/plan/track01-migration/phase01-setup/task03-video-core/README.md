# Task 03 - Draw/Video Core Package

Status: active

## Goal
Define and extract shared headless video/camera interfaces into `uppsrc/Draw/Video`, and migrate relevant code from `uppsrc/api/Media`.

## Scope
- Introduce `uppsrc/Draw/Video` as the shared base for Draw/Camera and ComputerVision.
- Move or adapt reusable capture/device/core types from `uppsrc/api/Media/*`.
- Keep UI out of Draw/Video.

## Proposed Mapping (Draft)
### Draw/Video core
- `Types.{h,cpp}` -> `VideoTypes.{h,cpp}` (formats, planes, timestamps)
- `MediaStream.{h,cpp}` -> `VideoStream.{h,cpp}` (if used outside api/Media)
- `MediaAtomBase.{h,cpp}` -> keep in api/Media or move to api/Video if still Eon-specific

### Capture backends
- `Capture_V4L2.*` -> `Draw/Video/V4L2Capture.*`
- `DeviceManager_V4L2.*` -> `Draw/Video/V4L2DeviceManager.*`
- `Capture_DShow.*` + `DShow_inc.h` -> `Draw/Video/DShowCapture.*`
- `DeviceManager_Win32.*` -> `Draw/Video/WinDeviceManager.*`
- `Capture_OpenCV.*` -> `Draw/Video/OpenCVCapture.*`

### Helpers
- `Video.*` -> `Draw/Video/VideoUtil.*` (format conversions, timers)
- `Audio.*` -> consider separate `Draw/Audio` (or keep in api/Media)
- `FileIn.*` -> keep in api/Media unless generalized

## Acceptance
- Draw/Camera and ComputerVision can share base interfaces without circular deps.
- api/Media content is migrated or proxied, with clear ownership.

## Notes
- Focus on performance, modularity, and debuggability.
- Keep naming explicit and verbose to avoid backend confusion.
