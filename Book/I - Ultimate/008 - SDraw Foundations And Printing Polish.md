# SDraw Foundations and Printing Polish (2008-12)
**Date Span:** 2008-12-01 to 2008-12-31

December 2008 was characterized by foundational work on `SDraw` (Software Draw) and a rigorous push to stabilize the printing pipeline. `SDraw` emerged as a core experimental area, integrating with the Anti-Grain Geometry (AGG) library and introducing Xft/FreeType character rendering to provide high-quality software-based rasterization. This period also saw the introduction of `TopWindow::FrameLess` for Win32 and assorted visual refinements in `TrayIcon`.

The printing system received critical attention, resolving color inaccuracies and artifacts when rendering images on specific hardware. Native mode metrics (`GetPagePixels`, `GetPixelsPerInch`) were corrected, and `DrawImage` received several stability fixes to prevent crashes on X11. TheIDE continued to evolve with a complete abbreviations overhaul, command-line export capabilities, and improved SVN integration, including new toolbar iconography and conflict-handling logic in `usvn`.

Core utilities were refined with a non-recursive implementation of `RealizeDirectory` and new time conversion helpers. On the database front, OLEDB and MSSQL support matured with better BLOB handling via `ISequentialStream` and optimized string fetching. Visual integrity was bolstered by the addition of the `ASSERT_INDEX` macro for `ArrayCtrl`, ensuring column consistency across the UI framework.

## References
- [1] 1c89d5e63 — TopWindow::FrameLess, so far for Win32 only (cxl, 2008-12-01)
- [2] b40888093 — void Print(Report& r, PrinterJob& pd, bool center) variant (cxl, 2008-12-01)
- [3] cf03eb890 — Fixed color and artifacts problem when printing Images (cxl, 2008-12-03)
- [4] 5506ef5e9 — New printing fixes now fixed not to crash X11 (cxl, 2008-12-03)
- [5] fa308cd10 — Draw metrics return correct values in native mode (cxl, 2008-12-04)
- [6] d37df33d6 — Modified export, export from commandline (cxl, 2008-12-05)
- [7] f06809288 — theide - Complete abbreviation (cxl, 2008-12-06)
- [8] 0bc12c910 — Fixed Ruler bugs; added ASSERT_INDEX macro (rylek, 2008-12-08)
- [9] 14a4dafc8 — Updated LIBTIFF to current stable version 3.8.2 (rylek, 2008-12-08)
- [10] cae1f62d5 — Fixed fetching BLOBS to use ISequentialStream (cxl, 2008-12-09)
- [11] 540c6a824 — MST's multimonitor hack in GetWorkArea (rylek, 2008-12-09)
- [12] 7e470b554 — Removed recursion from RealizeDirectory (cxl, 2008-12-15)
- [13] fc506396c — svn icon in toolbar (cxl, 2008-12-15)
- [14] 5f6327085 — working on SDraw (cxl, 2008-12-15)
- [15] f2f767905 — ArrayCtrl with FIXED HeaderCtrl (cxl, 2008-12-15)
- [16] 198cc512c — Abbreviations improvement (fi fj fk predefined) (cxl, 2008-12-17)
- [17] d56225490 — Time convert, TreeCtrl HighlightCtrl (cxl, 2008-12-18)
- [18] d7e3d83b4 — Playing with AGG / SDraw (cxl, 2008-12-21)
- [19] a074dd729 — TrayIcon::IsVisible (cxl, 2008-12-26)
- [20] 5b719a018 — SDraw Xft/FreeType character rendering (cxl, 2008-12-26)
