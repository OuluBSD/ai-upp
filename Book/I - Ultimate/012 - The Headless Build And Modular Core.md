# The Headless Build and Modular Core (2009-04)
**Date Span:** 2009-04-01 to 2009-04-30

April 2009 was a pivotal month for U++'s internal modularity and deployment versatility. The most significant architectural change was the formal separation of TheIDE's build logic from its graphical user interface. By isolating `ide/core` from the GUI components, the framework paved the way for a truly "headless" version of the U++ make utility, `umk`. This allowed for complex builds to be integrated into server-side automation and continuous integration pipelines without requiring an X11 display or Win32 windowing environment.

The database layer continued its expansion with the introduction of the dedicated `MSSQL` package, built upon the previous month's ODBC work. This included comprehensive `SqlBinary` support, although initial implementation notes acknowledged ongoing compatibility tuning for PostgreSQL. The maturation of the database suite was accompanied by a major documentation effort, particularly for `Path.h` and `LocalProcess.h`, ensuring that core system utilities were well-understood by the growing developer base.

Graphical and UI components saw several high-impact mergers and refinements. The `MultiList` component was merged into the core `ColumnList`, and `TabBar` received advanced stacking and sorting capabilities. The `Docking` framework was further stabilized with fixes for tab sizing and improved support for `PopUpDockWindow`. System introspection capabilities were also bolstered, with the `SysInfo` package receiving major updates for Linux, including window rectangle retrieval, mouse positioning, and screen capture saving.

TheIDE itself became significantly more "context-aware." The `Alt+J` shortcut was enhanced to jump directly to the Layout Designer when invoked on layout class templates, and the PDB debugger was upgraded to show live variable values via tooltips and improved string display. Behind the scenes, the framework's portability was extended to OpenBSD and NetBSD, while the community infrastructure expanded with the launch of Debian 64-bit nightly builds.

## References
- [1] 24537231d — Separated non-gui ide/core for headless umk (cxl, 2009-04-04)
- [2] bd4ac2445 — TheIDE: build process now separated from GUI (cxl, 2009-04-05)
- [3] 2f309d397 — MSSQL package (ODBC based) (cxl, 2009-04-20)
- [4] c4a2d7f9b — SqlBinary implementation (cxl, 2009-04-20)
- [5] a45a3835d — SysInfo: added Linux window/mouse handling (koldo, 2009-04-03)
- [6] cc271a3ba — mrjt's MultiList now merged into ColumnList (cxl, 2009-04-28)
- [7] a6b3970b8 — TabBar stacking/sorting; Docking refinements (mrjt, 2009-04-27)
- [8] aefe13696 — Alt+J jumps to layout designer for layout templates (cxl, 2009-04-17)
- [9] c9e56076d — PDB debugger shows variable tooltips (cxl, 2009-04-17)
- [10] 107b0a495 — Improved OpenBSD and NetBSD compatibility (cxl, 2009-04-24)
- [11] 7442ff952 — Added Debian 64-bit nightly builds (micio, 2009-04-26)
- [12] 320ff58f1 — JPGRaster::GetExifThumbnail (cxl, 2009-04-07)
- [13] 2fb88fb63 — Draw: Rescale progress Gate; Painter: ImagePainter (cxl, 2009-04-08)
- [14] 75c083094 — FileSel NativeIcon issues fixed in POSIX (cxl, 2009-04-09)
- [15] 925006ab7 — Improved string display in PDB debugger (cxl, 2009-04-15)
- [16] 4839ea02e — TheIDE: Sort packages by name (cxl, 2009-04-17)
- [17] 7fba1de66 — LocalProcess.h documented (cxl, 2009-04-19)
- [18] cd99a5247 — Optimization: const-ref for Pointf/Sizef/Rectf params (rylek, 2009-04-19)
- [19] 2bb046475 — Conflicting Painter routines moved to Core (cxl, 2009-04-21)
- [20] 3c976c3d4 — Drawing refactored (cxl, 2009-04-26)
