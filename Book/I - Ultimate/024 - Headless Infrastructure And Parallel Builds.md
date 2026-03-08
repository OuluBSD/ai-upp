# Headless Infrastructure and Parallel Builds (2010-04)
**Date Span:** 2010-04-01 to 2010-04-30

April 2010 was a definitive month for U++'s build infrastructure and its journey toward enterprise-grade modularity. The `lpbuild` script, the framework's primary Linux distribution tool, underwent a major transformation. By removing its hard dependency on TheIDE and introducing parallel build support, the development team significantly reduced compilation times and paved the way for automated package generation in environments without a graphical head. This effort culminated in the splitting of the Linux distribution into three distinct packages: `upp` (the core libraries), `theide`, and `theide-nogtk`, providing users with specialized binaries tailored to their specific needs.

The framework's multi-threading capabilities were put to the test with critical MT fixes in the `XmlRpc` package. These refinements ensured that the remote procedure call engine remained stable under concurrent load, supported by the new `XmlRpcPerform` function for group-based execution. Parallel to this, the database layer achieved better "error persistence": `SqlSession` was refactored to record the first error encountered in a sequence rather than the last, a change that significantly improved the debugging process for complex transaction chains. `SqlArray` also became more flexible with the introduction of the `WhenFilter` gate, allowing developers to filter records as they are being fetched from the database.

User interface ergonomics continued to advance with a focus on professional data presentation. `ArrayCtrl` was refined with a new `WhenScroll` event and critical fixes for handling selections in "VirtualCount" mode. The `EditField` became more robust with improved clipboard handling in ReadOnly mode and the introduction of the `never_hide` flag, ensuring that label icons remain visible even in cramped layouts. Visual aesthetics were polished with an improved `Etched(Image)` implementation, providing better-looking toolbars for applications using classic U++ themes.

At the core level, the framework addressed long-standing integration hurdles. `MD5_CTX` was renamed to `UPP_MD5_CTX` to resolve symbol clashes with OpenSSL, a move that simplified the inclusion of U++ headers alongside established security libraries. The `ArrayIndex` container was expanded with `Detach` and `PopDetach` methods, providing finer control over object lifecycles. The community's global footprint reached a new peak, with the official website receiving a flurry of translations in Catalan, Spanish, and French, alongside the launch of dedicated bug report and patch submission pages to streamline user contributions.

## References
- [1] c5b621b5f — lpbuild: removed dependency on theide, parallel build support (dolik, 2010-04-02)
- [2] b4cddfa9b — lpbuild: packages split to upp, theide, and theide-nogtk (dolik, 2010-04-29)
- [3] 2b505d382 — SqlSession: First SQL error is recorded instead of last (cxl, 2010-04-14)
- [4] a9d7aed5b — SqlArray: WhenFilter Gate for fetched records (cxl, 2010-04-07)
- [5] 8048ccf96 — XmlRpc: Critical MT fixes (cxl, 2010-04-09)
- [6] 68fb60465 — XmlRpc: Added XmlRpcPerform for group execution (cxl, 2010-04-09)
- [7] 3fe9f09e6 — Core: MD5_CTX renamed to UPP_MD5_CTX (rylek, 2010-04-07)
- [8] e0b33067a — ArrayIndex: Detach and PopDetach added (cxl, 2010-04-26)
- [9] 599259776 — CtrlLib: ArrayCtrl WhenScroll introduced (cxl, 2010-04-16)
- [10] 3e2bd542b — CtrlLib: 'never_hide' flag for label icons (rylek, 2010-04-27)
- [11] 6ebc494a5 — Draw: Improved Etched(Image) for classic toolbars (cxl, 2010-04-02)
- [12] 8f2f5f97e — Docking: Fixed key event blocking from main window (mrjt, 2010-04-08)
- [13] e695c8dc2 — Web: ISAPI extension POST data handling fix (rylek, 2010-04-25)
- [14] 8193b6d30 — CtrlLib: EditField ReadOnly Ctrl+C fix (cxl, 2010-04-18)
- [15] 7ccf33dc1 — Sql: fixed logical OR operator for SqlBool (cxl, 2010-04-26)
- [16] 723e360a1 — uppweb: bug reports and patch submission page (dolik, 2010-04-24)
- [17] 3b496e5d5 — uppweb: Added Catalan and Spanish translations (koldo, 2010-04-24)
- [18] 42701584a — uppweb: index page translated to French (koldo, 2010-04-25)
- [19] d1ce30b4e — lpbuild: gcc4.2 forbidden in dependencies (dolik, 2010-04-07)
- [20] a45624362 — XmlRpc: DeXml support for string values (cxl, 2010-04-06)
