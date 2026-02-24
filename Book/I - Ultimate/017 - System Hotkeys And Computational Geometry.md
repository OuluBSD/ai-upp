# System Hotkeys and Computational Geometry (2009-09)
**Date Span:** 2009-09-01 to 2009-09-30

September 2009 saw U++ expanding its reach into global system interactivity and advanced computational algorithms. A major feature addition was the introduction of `Ctrl::RegisterSystemHotKey`, providing a cross-platform API for capturing global keyboard shortcuts. Implemented for both Win32 and X11 (with specific workarounds for MSC 7.1 compiler bugs), this capability allowed U++ applications to respond to user input even when not in focus—a vital feature for utility and productivity software. This was immediately complemented by a new `Hotkey` reference example demonstrating the registration and handling of these system-wide events.

The framework's algorithmic foundation was bolstered by the addition of `ConvexHullOrder`, a new computational geometry algorithm for calculating 2D convex hulls. This addition signaled a move toward providing more specialized mathematical tools directly within the core library. Core performance and flexibility were also addressed with optimizations in `Vector::SetCountR` and the re-introduction of expanded 8-bit encoding support (including CP1161 and ARMSCII-8), catering to a broader international developer base.

The Bazaar saw the ambitious start of `Leptonica` library support. This effort aimed to wrap the powerful Leptonica image processing library, beginning with page layout analysis, text baseline finding, and rectangular marker detection. These features, complete with dedicated demos, provided U++ developers with high-level tools for document analysis and OCR-related tasks. Parallel to this, the `plugin/tiff` package received a patch for 2bpp images, and `AttrText` was enhanced with `SetImage`, allowing icons to be placed directly to the left of text in a standardized way.

The database layer continued its rapid maturation. PostgreSQL support was refined with critical charset fixes and the implementation of the `EXCEPT` (SQL minus) operator in `SqlExp`. `MSSQL` gained the `IdentityInsert` feature, allowing developers to bypass standard identity column restrictions during complex data migrations. UI refinements remained steady, with patches for separators in `DropList` and `ArrayCtrl`, and fixes for X11 `SetSurface` following the previous month's extension. TheIDE also received targeted fixes, including a crash resolution for the F5 (Run) command and improvements to the `IconDes` editor.

## References
- [1] ec3fe5e28 — CtrlCore: Ctrl::[Un]RegisterSystemHotKey (cxl, 2009-09-24)
- [2] 824a7c790 — CtrlCore: X11 version of RegisterSystemHotKey (cxl, 2009-09-24)
- [3] 09b739649 — reference: Hotkey registration demonstration (cxl, 2009-09-24)
- [4] 4ed79e9d0 — Added 2D convex hull calculation algorithm (rylek, 2009-09-17)
- [5] 422577d73 — Start of Leptonica library support in Bazaar (micio, 2009-09-06)
- [6] 14ae89ab3 — Bazaar: Leptonica page layout analysis with demo (micio, 2009-09-06)
- [7] b2e9a9ae1 — Bazaar: Leptonica text baseline finding (micio, 2009-09-07)
- [8] 2cb544acc — Draw: AttrText now has 'SetImage' support (cxl, 2009-09-22)
- [9] 620a85376 — SqlExp: PGSQL except/minus support (cxl, 2009-09-04)
- [10] 2e0e1145b — MSSQL: IdentityInsert bypass (cxl, 2009-09-29)
- [11] c301565f6 — X11 SetSurface fixed (cxl, 2009-09-02)
- [12] d1e4bc82a — Core: Fixed inefficiency in Vector::SetCountR (cxl, 2009-09-07)
- [13] 01fd10092 — Core: Added multiple 8-bit encodings (cxl, 2009-09-17)
- [14] 86003943f — CtrlLib: Fixed separator in DropList (cxl, 2009-09-04)
- [15] 44efd2a55 — CtrlLib: Fixed separator in ArrayCtrl (cxl, 2009-09-07)
- [16] a431f7e91 — plugin/tiff patch for 2bpp images (cxl, 2009-09-06)
- [17] c948749ad — PostgreSQL: fixed charset issues (cxl, 2009-09-04)
- [18] 5aa32116d — New calculator object: CalcSequenceNode (rylek, 2009-09-23)
- [19] eedba84ed — Fixed crash when pressing F5 in IDE (cxl, 2009-09-07)
- [20] 791bcd409 — DropList::Format respects DisplayAll modifier (rylek, 2009-09-01)
