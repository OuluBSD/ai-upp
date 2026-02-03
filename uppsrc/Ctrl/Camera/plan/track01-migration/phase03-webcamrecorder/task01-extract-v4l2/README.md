# Task 01 - Extract V4L2 Backend into Draw/Video

Status: done

## Goal
Move V4L2 capture logic (and any reusable webcam capture utilities) into Draw/Video.

## Scope
- Implement Draw/Video V4L2 backend and device enumeration.
- Provide a generic frame delivery interface.
- Keep device selection/configuration hooks.

## Acceptance
- WebcamRecorder builds using Draw/Video V4L2 backend.
- Draw/Video device enumeration provides formats/resolutions.
