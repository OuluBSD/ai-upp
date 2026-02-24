# Headless Draw and the NewDraw Merge (2009-07)
**Date Span:** 2009-07-01 to 2009-07-31

July 2009 was a month of massive consolidation for the U++ graphical subsystem. The intensive development that had been brewing in the "newdraw" nest finally reached its primary milestone: the "NewDraw merge." This fundamental refactor unified the core `Draw` hierarchy, making the entire drawing system more platform-independent and modular. The most striking result of this effort was the realization of a truly "headless" `Draw` implementation. By reaching this milestone, U++ applications gained the ability to perform complex graphical operations—including text rendering with accurate font metrics—in environments without a GUI, such as server-side console applications or command-line utilities. To demonstrate this, the `ConsoleDraw` reference example was introduced, showcasing high-quality rasterization performed entirely in a non-GUI context.

A critical component of this graphical revolution was the complete overhaul of font metrics. New implementations for X11, Win32, and general POSIX platforms were developed to ensure consistent text layout across all supported operating systems. The `Draw` package itself was rigorously documented during this period, covering everything from core `Image` and `ImageBuffer` classes to specialized drawers like `DataDrawer` and `DrawingDraw`. Visual integrity was further bolstered by fixes for rotated text, underlined fonts, and palette generator overflows.

The `Painter` package matured alongside the `Draw` refactor, gaining implementation for `DrawArc`, `DrawPolyPolyPolygon`, and `DrawPolyPolyline`. It also received support for `PEN_*` styles and standardized line-width handling. Parallel to the core push, the Bazaar flourished with the addition of an alternative MultiThreading package and significant updates to `ScatterControl`. The `SysInfo` package expanded its reach with new desktop recording capabilities, while the `Tcc` (Tiny C Compiler) integration underwent a package renaming and refinement phase.

Core library enhancements included support for the Blackfin CPU architecture and a behavior change for `Vector::Clear`, which now aggressively deallocates all memory—a vital optimization for long-running processes. The database layer continued its steady refinement with patches for PostgreSQL and the addition of BLOB support for MySQL. Deployment policies were also updated, with TheIDE's installation path for icons moving to more standard Linux desktop locations, signaling the framework's increasing alignment with modern desktop environments.

## References
- [1] 342011f75 — NewDraw 'merge' (cxl, 2009-07-06)
- [2] 6df29eb3b — CtrlCore, Draw: New headless draw (cxl, 2009-07-06)
- [3] 68a73dff4 — Draw: headless draw final milestone reached (cxl, 2009-07-19)
- [4] d723d337b — reference: ConsoleDraw headless Draw example (cxl, 2009-07-19)
- [5] ff431295a — Developing X11 independent font metrics (cxl, 2009-07-02)
- [6] ba4d690c8 — Developing Win32 font metrics (cxl, 2009-07-03)
- [7] ca4e37c8c — Developing posix font metrics (cxl, 2009-07-03)
- [8] bacf2dc0e — Draw: fixed underline issue, Font documented (cxl, 2009-07-08)
- [9] 8021c04db — Draw: Fixed problem with rotated texts (cxl, 2009-07-09)
- [10] b6119f310 — Alternative MultiThreading package added to Bazaar (mrjt, 2009-07-10)
- [11] 8ea1022dd — SysInfo: Added desktop recording (koldo, 2009-07-14)
- [12] 53af28a20 — Added implementation for Painter::DrawArc (rylek, 2009-07-30)
- [13] 1264da8e6 — Added DrawPolyPolyPolygonOp and DrawPolyPolylineOp (rylek, 2009-07-28)
- [14] 8c0aac824 — Core: Vector::Clear deallocates all memory (cxl, 2009-07-31)
- [15] 29701e5eb — Core: Blackfin CPU support (cxl, 2009-07-26)
- [16] 30ba42b00 — PostgreSQL patches (cxl, 2009-07-29)
- [17] 29649cf06 — BLOB in MySQL support (cxl, 2009-07-26)
- [18] e48f20a68 — GetPaintRect support in SystemDraw (rylek, 2009-07-17)
- [19] f0f0e4fb2 — Painter: support for PEN_*, fixed line width (cxl, 2009-07-31)
- [20] 0a1120e19 — TopWindow::NoCloseBox (cxl, 2009-07-03)
