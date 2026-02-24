# SDL Integration and Robust PolyXML (2010-11)
**Date Span:** 2010-11-01 to 2010-11-30

November 2010 was a month of significant external integration and core architectural hardening. The most prominent addition was the arrival of the **SDL** integration package. Led by the introduction of `SDLCtrl` and `SDLSurface`, this enabled U++ applications to host high-performance multimedia and game content powered by the Simple DirectMedia Layer library. Supporting both windowed and experimental fullscreen modes, this integration bridged the gap between U++'s standard UI toolkit and the specialized world of low-level graphics acceleration, immediately supported by a suite of demos.

The `PolyXML` polymorphic serialization system achieved a new level of robustness. Development focused on "unknown class" handling—the ability for the system to gracefully skip or preserve data when streaming in classes that are not registered or known to the current binary. This is a critical requirement for maintaining document compatibility across different versions of an application or between modular plugins. The `ClassFactory` and `REGISTERCLASS()` infrastructure were also refined to better handle cross-package dependencies, particularly regarding `Image` resources.

Infrastructure and diagnostic capabilities received vital upgrades. The `VppLog` system gained the `LOG_TIMESTAMP` option, providing developers with much-needed temporal context when debugging complex multi-threaded interactions. `XmlRpc` was hardened for better HTTP compliance and gained a specialized server tracing API, while `HttpClient` also received a dedicated trace mode. The PostgreSQL driver was bolstered with a `KeepAlive` option, ensuring stable long-term connections in unstable network environments.

User interface ergonomics continued to reach professional standards. The `PlotterCtrl` in the `Geom` package was extended with fast mouse panning and zooming, aligning its behavior with commercial GIS software standards. `FileSel` was upgraded to support Terminal Services client shares on Windows, and `RichEdit` gained `SingleLine` and `Filter` capabilities, making it more suitable for specialized text entry tasks. TheIDE's build system was further polished with fixes for Makefile exports and initial compatibility adjustments for the emerging `Clang` compiler toolchain.

## References
- [1] 1db52a97f — SDL: SDLCtrl initial release (koldo, 2010-11-17)
- [2] 98d6f8a0b — SDL: SDLSurface introduced; .usc support added (koldo, 2010-11-20)
- [3] 5f79c2eba — PolyXML: Correct handling of unknown classes during streaming (micio, 2010-11-21)
- [4] 6d40225ee — Core: VppLog: Added LOG_TIMESTAMP option (cxl, 2010-11-12)
- [5] 5109c728c — PostgreSQL: Added KeepAlive option (cxl, 2010-11-11)
- [6] 715ff8b0c — XmlRpc: Added SetXmlRpcServerTrace (cxl, 2010-11-09)
- [7] acc88a410 — Web: HttpClient trace mode introduced (cxl, 2010-11-26)
- [8] 8abfd3eaf — Geom/Ctrl: PlotterCtrl standardizes GIS-style panning/zooming (rylek, 2010-11-15)
- [9] 5cb014b02 — CtrlLib: FileSel support for Terminal Services shares (cxl, 2010-11-16)
- [10] 44ffc21d2 — RichEdit: Added SingleLine and Filter support (cxl, 2010-11-14)
- [11] 23117fa56 — theide: Fixed Makefile export library omission bug (dolik, 2010-11-01)
- [12] a15de079b — Core: Clang toolchain compatibility fixes (cxl, 2010-11-06)
- [13] 2fc7b2931 — plugin/gif: Added LCT support and 64-bit fixes (cxl, 2010-11-07)
- [14] adf871365 — bazaar: Tree container made Moveable and Iterable (kohait, 2010-11-04)
- [15] b17c321d2 — bazaar: Urr MinGW linkage fix for ws2_32 (kohait, 2010-11-04)
- [16] b1e798150 — Core: XmlIO gains Node() method (cxl, 2010-11-21)
- [17] c562a3300 — Core: GetIniKey prioritized search in executable directory (cxl, 2010-11-24)
- [18] 0320f478f — RichEdit: Optimized speller dictionary search (cxl, 2010-11-24)
- [19] 9e3b1834e — Draw: Added GetPaintRect to DrawDrawing (rylek, 2010-11-23)
- [20] 755f3c957 — GridCtrl: Fixed state of accept/cancel buttons (unodgs, 2010-11-10)
