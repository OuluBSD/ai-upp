# Small Value Optimization and the HTTP Server (2012-05)
**Date Span:** 2012-05-01 to 2012-05-31

May 2012 was a month of massive core library expansion and the formalization of U++ as a high-performance web server platform. The pioneering work in **SVO_VALUE** (Small Value Optimization) reached its culmination, providing a hyper-efficient data handling core that became the foundation for a suite of new serialization and networking features. The most significant architectural addition was the introduction of the **HttpResponse** class and the **HttpServer** reference example. By integrating these into the canonical `Core` library, U++ completed its web stack, enabling developers to build not just web clients but fully-featured, standalone web services with native SSL and cookie support. This leap in connectivity was further hardened with the addition of **XmlizeByJsonize**, providing a cross-format serialization bridge, and improved SCGI request handling within the `HttpHeader` parser.

The framework reached a significant compiler milestone by officially **dropping support for MSC 7.1** (Visual C++ 2003). This allowed the development team to clean up numerous legacy workarounds and adopt more modern C++ idioms throughout the codebase. The `Core` library also gained several vital diagnostic and utility features: `LOGHEX` and `DUMPHEX` for streamlined binary debugging, `AsString(T *ptr)` for pointer logging, and `TcpSocket::GlobalTimeout` for better control over long-lived network connections. Multi-threading robustness was also improved, with `CONSOLE_APP_MAIN` and `GUI_APP_MAIN` now automatically wrapping application logic in `try/catch(Exc)` blocks to ensure graceful termination on unhandled U++ exceptions.

Graphical and professional components continued their march toward maturity. The **Rainbow** project saw intensive activity, including the introduction of the `_DBG_Ungrab` debug mode for X11 to resolve mouse-grabbing issues during debugging sessions. The `Draw` package's primary raster types—including `Font`, `Image`, and `Painting`—were updated to natively support **Xmlize** and **Jsonize**, enabling the easy persistence of complex graphical states. In the Bazaar, the **FSMon** (File System Monitor) package was introduced, providing a cross-platform way to respond to real-time file system changes, while **Controls4U** achieved compatibility with the OpenCV computer vision library.

TheIDE and professional tooling were refined for high-efficiency workflows. A new "Insert as C string" feature was added to the editor, and the `SrcUpdater` system reached production stability. Documentation reached a new peak of accessibility with the launch of formal specifications for the **.upp package format**, providing long-term transparency for the framework's build system. The month closed with the promotion of the **Scatter** package to the canonical release suite and critical fixes for `NoLayoutZoom` behavior, ensuring that U++ applications maintained perfect visual proportions across a wide range of display resolutions and DPI settings.

## References
- [1] 856f93f30 — Core: HttpResponse class introduced (cxl, 2012-05-21)
- [2] 3156ff7d8 — reference: HttpServer example launched (cxl, 2012-05-21)
- [3] a745abee0 — Core: XmlizeByJsonize cross-format bridge (cxl, 2012-05-20)
- [4] 0154a17ce — Core: MSC 7.1 support officially dropped; AsString(T*) added (cxl, 2012-05-12)
- [5] 9c66fcab3 — Draw: Font, Image, and Painting gain Xmlize/Jsonize support (cxl, 2012-05-14)
- [6] f3876f57f — Core: Main entry macros now include try/catch(Exc) wrappers (cxl, 2012-05-14)
- [7] ab0115995 — Bazaar: FSMon (File System Monitor) introduced (micio, 2012-05-12)
- [8] 7645c1b5d — CtrlCore: _DBG_Ungrab for X11 debugging added (cxl, 2012-05-17)
- [9] 8850c3e02 — Core: LOGHEX and DUMPHEX binary logging utilities (cxl, 2012-05-28)
- [10] 7873c4ded — Core: TcpSocket::GlobalTimeout introduced (cxl, 2012-05-15)
- [11] 0c46b0086 — Core: HttpHeader adds support for SCGI requests (cxl, 2012-05-20)
- [12] e148bae3d — theide: Formal documentation for .upp package format (dolik, 2012-05-29)
- [13] 0714217ba — uppsrc: Scatter package promoted to canonical release (cxl, 2012-05-28)
- [14] 305cb2d93 — Controls4U: OpenCV compatibility added (koldo, 2012-05-16)
- [15] 6ecaedceb — ide: Insert clipboard as C string / Convert to C string (cxl, 2012-05-22)
- [16] 832424d8f — Core: Improved HTTP Cookie support and precedence (cxl, 2012-05-22)
- [17] 084f27465 — CtrlCore: NoLayoutZoom fixed for project templates (cxl, 2012-05-28)
- [18] f2c79982e — theide: Gdb_MI2 adds Ctrl+Q quickwatch for cursor variable (micio, 2012-05-18)
- [19] b415beb83 — Bazaar: PolyXML separates ClassFactory for modularity (micio, 2012-05-26)
- [20] 023030747 — Sql: SqlBinary achieves PGSQL9 compatibility (cxl, 2012-05-20)
- [21] 39131edb0 — Core: SVO_VALUE Xmlize moved into ValueArray/Map methods (cxl, 2012-05-16)
