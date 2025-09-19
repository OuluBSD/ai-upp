CURRENT TASK

Objective
- Bring GuboCore (3D GUI) to practical parity with CtrlCore’s developer-facing API while preserving its manager-driven rendering/event model.

Progress
- Phase 1 implemented: SurfaceFrame capture plumbing now delegates to Gu::SurfaceManager; Surface container-level frame capture getters/setters are manager-backed. Next up: Gubo 3D parity items in Phase 2–4.

Plan (phased)
- Phase 1 — Capture/Mouse plumbing
  - Implement frame-level capture helpers by delegating to managers:
    - SurfaceFrame::{Get/Set}Captured, {Get/Set}WithMouse -> Gu::SurfaceManager
    - GuboFrame equivalents -> Gu::GuboManager
  - Implement container-level capture accessors:
    - Surface::{Get/Set}FrameCaptured/WithMouse (manager-backed)
    - Gubo::{Get/Set}FrameCaptured/WithMouse (already wired; verify)

- Phase 2 — Missing core methods (mirroring CtrlCore)
  - Gubo::GetWorkArea, ReleaseGuboCapture, GetCaptureGubo
  - Surface::GetWorkArea, ReleaseSurfaceCapture, GetCaptureSurface
  - Use manager’s capture root and platform work-area from active scope.

- Phase 3 — Event loop and focus hooks
  - TopGubo::Run/OpenMain/FocusEvent/UpdateFromTransform3D
  - TopSurface::Run/OpenMain/FocusEvent/UpdateFromTransform2D
  - Strategy: call into the active manager (`Gu::GuboManager` / `Gu::SurfaceManager`) and reuse existing HAL/Eon main loop integration where available (e.g., `Surface::EventLoop()`/`MainLoop()`), keeping the public API shape similar to `TopWindow::Run`.

- Phase 4 — 3D interaction parity
  - Port 2D behaviors from `GI2D.cpp` to `GI3D.cpp` where stubs exist:
    - MouseEventInFrameContent / MouseWheelInFrameContent / MouseEventInFrameCaptured
    - Ensure deep mouse move/leave consistency and Z-order walking matches 2D.

- Phase 5 — Paint pipeline verification
  - Audit `GeomInteraction3D::Redraw` to ensure `CtrlDrawBegin/End` begin/end symmetry and clipping on frame/content match 2D.
  - Ensure `PaintPreFrame`/`PaintPostFrame` frame clipping mirrors `Surface` behavior.

- Phase 6 — Utility completion
  - ProgDraw3::GetFrameSize and `operator Image()` (if required by callers). For now, keep as command recording; implement image extraction only if needed by tests or previews.
  - Add minor diagnostics (DumpDrawCommands) to aid bringing up complex UIs.

Parity Checklist (CtrlCore → GuboCore)
- Focus/capture API: SetCapture/ReleaseCapture, IsCaptured, GetCaptured (per-manager)
- Frames: FrameLayout/Paint/AddSize, OverPaint, mouse enter/leave, capture inside frame
- Event dispatch: mouse move/wheel/button, deep dispatch, key/hotkey paths
- Layout: DeepLayout/DeepFrameLayout, SizePos helpers
- Redraw: pending flags, effect vs. content redraw, debug draw toggles

Validation
- Minimal demo: create a `TopGubo`, add a child `Gubo` with simple `Paint(Draw3&)` and mouse handlers; exercise capture/with-mouse transitions via `Dispatch(CtrlEvent)`.
- Verify manager (GuboLib) reports capture and with-mouse consistently while moving the pointer across nested children and frames.

Repository Hygiene
- Update `GuboCore.upp` to list `AGENTS.md` first and `CURRENT_TASK.md` second (done).
- Follow Header Include Policy in all new/modified `.cpp` files; defer cleanup of existing violations to a separate pass.

Notes
- Keep API names and behavior aligned with CtrlCore for developer familiarity. Where 3D requires different semantics (e.g., depth-aware hit-testing), document the differences inline in the implementation.
