# Task 02 - Adapt WmrTest to CameraCtrl

Status: pending

## Goal
Refactor WmrTest to use the new CameraDraw + CameraCtrl packages.

## Scope
- Replace internal camera UI classes in WmrTest with CameraCtrl package types.
- Use CameraDraw for capture backend and data flow.
- Keep WmrTest-specific menus and tracking-only views as local code.
- Ensure background thread feeds CameraDraw/CameraCtrl with frames/overlays/stats.

## Acceptance
- WmrTest compiles and behavior matches current app.
- No regression in capture stats, overlay rendering, or tracking views.
