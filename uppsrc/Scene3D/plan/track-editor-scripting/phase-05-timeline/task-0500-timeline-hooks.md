# Task 0500 - Timeline Hooks

## Goal
Provide timeline hooks similar to ActionScript MovieClip.

## Scope
- `onFrame` per object.
- `gotoAndPlay`, `gotoAndStop` for scene and object timelines.
- Frame labels (optional).

## Success Criteria
- Scripts can control playback and receive frame callbacks.

## Status
- Done

## Notes
- Added `gotoAndPlay`/`gotoAndStop` for stage + display object proxies.
- Added `onFrame` binding to `enterFrame` dispatcher.
