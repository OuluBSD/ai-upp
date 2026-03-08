# SCGI Servers and the Cypher Modularization (2010-10)
**Date Span:** 2010-10-01 to 2010-10-31

October 2010 was a month of significant advancement for U++ in the realms of web connectivity, modular security, and core architectural refinement. The most impactful technological addition was the introduction of the **ScgiServer** within the `Web` bundle. Simple Common Gateway Interface (SCGI) provided a high-performance alternative to traditional CGI, and the U++ implementation, featuring a virtual-method based event model and native multi-threading, enabled seamless integration with modern web servers like Nginx. This effort was immediately supported by the `ScgiHello` reference example and comprehensive sample configurations for both Windows and *nix systems.

The Bazaar saw a major leap in security modularity with the introduction of the **Cypher** package. Designed as a flexible, object-oriented foundation for encryption, `Cypher` unified the handling of Initialization Vectors (IVs/nonces) and provided a consistent stream-based interface for various cryptographic algorithms. This immediately transformed the `Protect` package, which was refactored to use the new `Cypher` core for its web authentication schema, now featuring a fully functional Client/Server architecture for software protection.

Core library ergonomics and performance were also a primary focus. The `Tuple` container reached a new level of utility by becoming natively `Moveable` (provided its elements are moveable), and `String` gained a powerful `Replace` method. Networking was further hardened with the addition of unicode escape handling (`%uxxxx`) in `UrlDecode`, ensuring robust processing of internationalized web payloads. Serialization also matured, with `ValueArray` and `ValueMap` gaining native `Xmlize` support, allowing for more intuitive storage of complex nested data structures.

User interface components continued their steady climb in sophistication. `Scatter` received a massive interactivity boost with mouse-wheel zoom support and dynamic mouse-cursor feedback during scrolling. `GridCtrl` was refined with property-grid improvements, a new `ClearChanged()` method, and better handling of summary horizontal scrolling. TheIDE's text editor gained a highly requested feature: `Shift+MouseWheel` for horizontal scrolling, making the navigation of wide code files significantly smoother. The build system was also kept current, adding support for Ubuntu 10.10 and improving the `AutoSetup` logic for TDM-MINGW and Visual Studio 2010.

## References
- [1] 6fc2830ee — Web: ScgiServer introduced (cxl, 2010-10-03)
- [2] ebfeebaf3 — reference: ScgiHello example (cxl, 2010-10-03)
- [3] 3b7dc2597 — Bazaar: Cypher modular encryption package introduced (micio, 2010-10-02)
- [4] 1dfd3af82 — Bazaar: Protect refactored to use new Cypher core (micio, 2010-10-03)
- [5] 54df285bf — Core: String::Replace introduced (cxl, 2010-10-10)
- [6] 2d86e9af3 — Core: Tuples made natively Moveable (cxl, 2010-10-19)
- [7] c2f885422 — Web: Unicode escape handling added to UrlDecode (rylek, 2010-10-22)
- [8] f73a7aecf — Core: Xmlize support for ValueArray and ValueMap (cxl, 2010-10-27)
- [9] 1ddc569c7 — CtrlLib: LineEdit Shift+MouseWheel for horizontal scroll (cxl, 2010-10-11)
- [10] 79a5ee8f2 — Scatter: Added dynamic mouse behavior and zoom (koldo, 2010-10-22)
- [11] a7c763095 — Bazaar: Protect Client/Server web authentication (micio, 2010-10-12)
- [12] af3d8a782 — reference: Nginx sample configuration for SCGI (jeremy_c, 2010-10-04)
- [13] 1a543404d — Geom/Ctrl: PlotterCtrl mouse wheel zoom (rylek, 2010-10-04)
- [14] 37acc2944 — Sqlite3: FTS (Full Text Search) support enabled (cxl, 2010-10-13)
- [15] 0df8e08e5 — Sqlite3: GetTransactionLevel added (cxl, 2010-10-13)
- [16] 38314e3e9 — CtrlLib: RichTextView added WhenMouseMove callback (cxl, 2010-10-14)
- [17] 6c9b8ccc9 — CtrlLib: Picture::Get introduced (cxl, 2010-10-12)
- [18] a1bb3fb2a — lpbuild: Ubuntu 10.10 packaging support (dolik, 2010-10-08)
- [19] c6255d919 — Core: GetHash for Index and Map (cxl, 2010-10-27)
- [20] 0b2d4289d — ide: Fixed autosetup for VS2010 and TDM-MINGW (ndrew2k, 2010-10-09)
