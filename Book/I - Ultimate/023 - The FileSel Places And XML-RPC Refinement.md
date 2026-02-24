# The FileSel Places and XML-RPC Refinement (2010-03)
**Date Span:** 2010-03-01 to 2010-03-31

March 2010 was characterized by significant user-facing refinements in core UI components and a deep hardening of the framework's serialization and networking layers. The most visible change was the complete overhaul of `FileSel`. The standard file selection dialog received a new "places" bar on the left, providing quick access to common folders like Home, Downloads, and Media. This was supported by new core functions for reliably locating system folders across Win32 and X11, ensuring that U++ applications felt more natively integrated with the user's desktop environment.

The `XmlRpc` package, introduced just a few months prior, underwent a rapid maturation phase. Key additions included a "short-circuit" mode for local client-server communication, a new `Ret` method for cleaner result handling, and improved error reporting and logging flags. Parallel to this, the `PolyXML` system was enhanced with support for class grouping and internationalized descriptions, further cementing its role as the framework's primary polymorphic serialization engine. Core XML handling was also refined, with `XmlNode` gaining improved support for multiline strings and `XmlParser` receiving new `Peek*` methods for non-destructive tag inspection.

The framework's commitment to modern standards and professional workflows was evident in several key areas. `CtrlCore` achieved a major milestone with native Unicode support in `CommandLine()` and `Environment()` on Windows, enabling dynamic charset switching at runtime. Multi-threading was further stabilized with a critical fix for `PanicMessageBox` in X11 and the adoption of thread-safe `_r` variants for system time functions in POSIX. The PDB debugger became more productive with the ability to rename files directly from the UI and improved handling of self-inheriting classes in Assist++.

The Bazaar and community projects continued to flourish. The `Scatter` package received a massive interactivity boost with mouse-based zoom and scroll, while `AESStream` saw documentation and API improvements. A new `RepGen` package for report generation began taking shape, and `Functions4U` reached a milestone with its documentation being fully converted to the T++ format. The framework's global footprint also expanded with new Spanish (SP-SP) and Italian (IT-IT) translations, and the official website was refreshed with a new RSS feed and improved download pages. Finally, the introduction of `IDEXIT` provided a cleaner, more standardized way for applications to handle window closing events, giving it precedence over standard `IDYES` signals.

## References
- [1] afdd82972 — CtrlLib: FileSel now has a left bar with common folders (cxl, 2010-03-08)
- [2] ec3ec7ae3 — CtrlCore: Unicode support in CommandLine() and Environment() (rylek, 2010-03-10)
- [3] fa3441cfd — XmlRpc: Shortcircuit mode; Draw: FindIml deadlock fix (cxl, 2010-03-09)
- [4] 3f7da15be — XmlRpc: Client now has Ret method (cxl, 2010-03-09)
- [5] 7c7f23cba — PolyXML: Added support for simple class grouping (micio, 2010-03-16)
- [6] 52e5e0447 — Scatter: Mouse zoom and scroll; Internationalization (koldo, 2010-03-03)
- [7] ba4ff7c5b — CtrlCore: IDEXIT introduced with precedence over IDYES (cxl, 2010-03-23)
- [8] 9e896d777 — XmlNode: Improved support for multiline strings (cxl, 2010-03-21)
- [9] 9aff9a5b3 — Core: XmlParser Peek* methods (cxl, 2010-03-15)
- [10] bad76742d — Core: CParser provides column position information (cxl, 2010-03-10)
- [11] 42b3008be — CtrlLib: RichEdit supports dropping Image files (cxl, 2010-03-26)
- [12] 54a63c6e8 — CtrlLib: TabCtrl Slave-based slave management (cxl, 2010-03-03)
- [13] 3d79b68a9 — RichText: PaintInfo - shrink_oversized_objects (cxl, 2010-03-04)
- [14] 449208fcc — Core: FindFile::IsExecutable (cxl, 2010-03-09)
- [15] c11ff4bfe — Core/CtrlLib: Fixing GetDownloadsFolder() (cxl, 2010-03-08)
- [16] 5ee19714e — lpbuild: Clean-up and conversion to T++ docs (dolik, 2010-03-30)
- [17] 3d7873dc2 — uppweb: Added RSS feed (dolik, 2010-03-27)
- [18] f2de015fb — Sql: Installable SqlSession errorhandler routing (cxl, 2010-03-17)
- [19] 1dc7f3b22 — plugin/dbf: WriteRow() for sequential output (rylek, 2010-03-30)
- [20] 4fdc65100 — plugin/jpg: Fixed to work in Win64 (cxl, 2010-03-06)
