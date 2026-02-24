# System Hotkeys and Computational Geometry
**Date Span:** 2009-09-01 to 2009-09-30

### Global Interactivity
Introduced `Ctrl::RegisterSystemHotKey` for cross-platform global keyboard shortcut handling on both Win32 and X11. This enabled applications to respond to shortcuts even when not in focus, showcased via a new `Hotkey` reference example.

### Algorithmic Foundations
Added `ConvexHullOrder`, a 2D convex hull calculation algorithm, strengthening the framework's computational geometry tools. Core performance was improved with optimizations in `Vector::SetCountR` and expanded 8-bit charset support.

### Leptonica and Image Analysis
Launched the `Leptonica` library wrapper in the Bazaar, introducing advanced page layout analysis, text baseline finding, and marker detection. `plugin/tiff` was patched for 2bpp images, and `AttrText` gained standardized icon support via `SetImage`.

### Database and UI Polish
PostgreSQL support matured with `EXCEPT` operator support and critical charset fixes. `MSSQL` gained `IdentityInsert` for migrations. UI refinements included fixed separators in `DropList` and `ArrayCtrl`, and a resolution for the F5-run crash in TheIDE.
