# Task 02 - Adapt WmrTest to Ctrl/Camera

Status: completed

## Goal
Refactor WmrTest to use the new Draw/Camera + Ctrl/Camera packages.

## Scope
- Replace internal camera UI classes in WmrTest with Ctrl/Camera package types.
- Use Draw/Camera for capture backend and data flow.
- Keep WmrTest-specific menus and tracking-only views as local code.
- Ensure background thread feeds Draw/Camera/Ctrl/Camera with frames/overlays/stats.

## Acceptance
- WmrTest compiles and behavior matches current app.
- No regression in capture stats, overlay rendering, or tracking views.

## Progress
- Switched WmrTest test harnesses (dump/track) to Draw/Camera StereoSource.
- WmrTest UI and capture path now use Ctrl/Camera + Draw/Camera.
