# Painter 2.0 and the ODBC Frontier (2009-02)
**Date Span:** 2009-02-01 to 2009-02-28

February 2009 saw U++ doubling down on its new graphical infrastructure while simultaneously expanding its database reach. The "Painter" package, introduced just a month prior, underwent a massive transition to version 2.0. This was not a mere refinement but a fundamental optimization and feature expansion. The new engine introduced high-performance Bezier curve approximators, a finished "stroker" for path outlines, and a robust "dasher" for complex line patterns. Clipping support and "NOAA" (Non-Overlapping Anti-Aliasing) were added, culminating in the month's breakthrough: subpixel rendering. This allowed for LCD-optimized text and graphics that felt significantly sharper on contemporary monitors.

As Painter 2.0 landed in `uppsrc`, its predecessor was archived, and a flurry of `PainterExamples` followed, showcasing SVG Arc support, various font technologies, and cache-glyph performance tests. The month even saw experiments with LCD-optimized rendering flags and specialized rounding fixes to ensure perfect pixel alignment across platforms.

Parallel to the graphical push, the database layer crossed a new frontier with the development of the ODBC connector. By the end of the month, the first working version of the ODBC driver was committed, providing a bridge to a vast array of legacy and enterprise data sources on both Windows and Linux. Existing connectors like SQLite3 and Oci8 (Oracle) received targeted fixes, particularly for Win64 compatibility, signaling the framework's readiness for the 64-bit transition.

The UI framework continued to mature with developer-centric ergonomics. `ArrayCtrl` gained the ability to restore the cursor position after a header-click sort—a small but vital quality-of-life improvement. It also received new integrity asserts to catch data-binding errors early. `Splitter` gained transparency support, and the GTK Chameleon backend was improved to handle "night themes" more gracefully. On the tooling side, TheIDE enabled user-definable keys for the `IconDes` editor and improved support for Unicode files with Byte Order Marks (BOM).

## References
- [1] b017fc959 — Painter 2.0 FINISHED! (cxl, 2009-02-12)
- [2] cf0caff14 — Painter 2.0 moved to uppsrc (cxl, 2009-02-12)
- [3] 486c0e793 — ODBC connector (1st working version) (cxl, 2009-02-27)
- [4] 761b710a3 — Painter 2.0: implemented bezier curve approximators (cxl, 2009-02-02)
- [5] f9554adb2 — Stroker finished! (cxl, 2009-02-06)
- [6] 44f94aee9 — Painter 2.0 dasher (cxl, 2009-02-07)
- [7] 64994ce5a — Painter 2.0 clipping (cxl, 2009-02-12)
- [8] 7db487113 — Subpixel rendering support in Painter (cxl, 2009-02-21)
- [9] 33078261f — Subpixel rendering finished (cxl, 2009-02-22)
- [10] 33c2c2f2b — ArrayCtrl: restore cursor after header sort (cxl, 2009-02-03)
- [11] bb9a2158c — Transparent Splitter; integrity ASSERT in ArrayCtrl (rylek, 2009-02-02)
- [12] ea8d677f9 — GTK Chameleon improvements for night themes (cxl, 2009-02-15)
- [13] 8307f28d6 — Win64 fixes (cxl, 2009-02-17)
- [14] 30b677638 — SvgArc support (cxl, 2009-02-14)
- [15] 5d7b666c4 — BOM unicode files support (cxl, 2009-02-13)
- [16] 8445ecc79 — TheIDE: IconDes keys now definable (cxl, 2009-02-13)
- [17] a2f1e8609 — ODBC linux fix (cxl, 2009-02-27)
- [18] e16d4ea2d — Fixed rounding problem in subpixel rendering (cxl, 2009-02-23)
- [19] f5653dff7 — Developing rasterizer (cxl, 2009-02-01)
- [20] 61083d8dd — Painter: Optimized curve approximations (cxl, 2009-02-23)
