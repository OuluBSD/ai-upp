# GTK Maturity and the REST of Core (2013-01)
**Date Span:** 2013-01-01 to 2013-01-31

January 2013 was a month of intensive consolidation for the U++ GTK backend and a significant expansion of the core networking and serialization suites. The **Rainbow GTK backend** reached a new level of platform parity, receiving implementation for critical desktop features: native **TrayIcon** support, global **RegisterSystemHotKey** handling, and high-performance **SetSurface** rendering. The backend was also hardened with fixes for clipboard operations, delayed window activation, and support for the `_NET_FRAME_EXTENTS` protocol, ensuring that U++ applications could correctly calculate their frame geometry in modern Linux desktop environments.

The core library's connectivity and data handling reached several professional milestones. **HttpRequest** was upgraded to support the "REST of methods"—including HEAD, DELETE, OPTIONS, and TRACE—effectively completing the framework's capability for building and consuming modern RESTful APIs. **Jsonize** was also refined for better web interoperability, now outputting `int64` values as raw numbers up to +/- 2^53 to maintain precision when communicating with JavaScript-based environments. Core numerical precision was also boosted, with **FormatDouble** now providing up to 16 digits of accuracy.

User interface ergonomics and system integration saw steady refinement. The **FileSel** component was enhanced to allow **multiple directory selection** in `SelectDir` mode, providing more flexibility for bulk data organization tasks. **RealizeDirectory** was fixed to correctly handle network paths, and **Vector::Insert** received a strategic memory optimization by reducing its allocation reserve to 50%, balancing performance with footprint. The framework's internal multi-threading (MT) was also hardened with specialized fixes for X11 and GTK concurrency.

Specialized application layers continued their maturation. The **Skylark** web framework received fixes for session clearing and identity management, while the **Functions4U** and **SysInfo** community packages were updated with improved time formatting and new translations. Graphical polish continued with fixes for JPEG hotspot rendering and standardized underline behavior in the GTK backend. The month closed with the addition of the **Euskara (Basque)** translation for the system information suite, further expanding the framework's global footprint.

## References
- [1] 366359733 — CtrlLib: GTK TrayIcon implementation completed (cxl, 2013-01-17)
- [2] d676e2d8e — CtrlCore: GTK/X11 RegisterSystemHotKey support added (cxl, 2013-01-19)
- [3] 0c5d0f87e — CtrlCore: GTK SetSurface and multi-monitor WorkArea fixes (cxl, 2013-01-20)
- [4] fe1527c05 — Core: HttpRequest adds support for the REST of methods (HEAD, DELETE, etc.) (cxl, 2013-01-25)
- [5] a1a9653d8 — Core: Jsonize handles int64 as numbers up to +/- 2^53 (cxl, 2013-01-24)
- [6] 4cd6491bc — Core: FormatDouble precision increased to 16 digits (cxl, 2013-01-01)
- [7] fb80a72de — CtrlLib: FileSel SelectDir allows multiple directory selection (cxl, 2013-01-19)
- [8] 8a044fe44 — Core: Vector::Insert allocation reserve reduced to 50% (cxl, 2013-01-27)
- [9] be4b5aa15 — Core: RealizeDirectory fixed for network names (cxl, 2013-01-01)
- [10] 51795d463 — CtrlCore: GTK _NET_FRAME_EXTENTS support (cxl, 2013-01-13)
- [11] 2594709c1 — CtrlCore: GTK adds support for file pasting and dropping (cxl, 2013-01-02)
- [12] 66eb049be — CtrlCore: GTK IsPainting implemented (cxl, 2013-01-03)
- [13] 13dbe1dd9 — CtrlCore: X11 and GTK multi-threading (MT) fixes (cxl, 2013-01-26)
- [14] 43191e369 — Core: TcpSocket fixed for BSD compatibility (cxl, 2013-01-26)
- [15] 7df43aa5c — Skylark: Fixed ClearSession identity issues (cxl, 2013-01-13)
- [16] 7266b9547 — plugin/jpg: Hotspot initialization fix (unodgs, 2013-01-15)
- [17] 8079a3b6d — CtrlCore: GTK fixed underline rendering (cxl, 2013-01-28)
- [18] 5d8a0f77e — CtrlCore: GTK clipboard and HttpQuery fixes (cxl, 2013-01-01)
- [19] 3f4e1e764 — CtrlCore: GTK fixed delayed activation issues (cxl, 2013-01-12)
- [20] 91e89dfa0 — SysInfo: Basque (Euskara) translation added (koldo, 2013-01-19)
