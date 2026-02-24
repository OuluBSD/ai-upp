# Single Instances and Secure Networking (2011-02)
**Date Span:** 2011-02-01 to 2011-02-28

February 2011 was a month of significant advancement for U++ application lifecycle management and the robustness of its networking layer. The most impactful technological addition was the introduction of the **Uniq** package in the Bazaar. Designed to facilitate "single instance" applications, `Uniq` provided a cross-platform mechanism (utilizing overlapped I/O pipes on Windows and SudoLib-backed sockets on Linux) to ensure that only one instance of an application could run at a time, while also providing a standardized way to pass command-line arguments from secondary attempts to the main instance. This period also saw the `Protect` package reach a new level of maturity, with its encryption macros becoming fully multi-thread safe and gaining locale-aware server-side logic.

The networking stack underwent a major hardening phase. The `Socket` class was refactored to support per-instance error handling, replacing the legacy global error string with a more thread-safe and precise reporting mechanism. This was particularly vital for the newly introduced **HttpsClient**, which received extended error handling specifically for SSL-related failures and improved end-of-file detection. Parallel to this, the `PostgreSQL` driver was updated to utilize the core `Web` package, streamlining its internal connectivity logic.

User interface ergonomics and visual fidelity remained a core priority. The **Animate** system was further refined, resulting in much smoother transitions for `PopUpTable` and `ColorPopup` controls. `RichEdit` was enhanced to store single-object selections on the system clipboard in QTF and RTF formats, alongside their native object formats, significantly improving interoperability with other word processors. X11 support was also bolstered with the addition of the "urgency hint" (`_NET_WM_STATE_DEMANDS_ATTENTION`), ensuring that U++ applications could correctly signal for user attention in modern Linux window managers.

The Bazaar and community projects saw a period of intense activity and restructuring. The "Functions4U" initiative introduced the `StaticPlugin` architecture, enabling a more modular approach to external dependencies—a feature immediately utilized to fully refurbish the `OfficeAutomation` package. The `Controls4U` package gained the ability to set backgrounds for `StaticFrame` and improved its speed and documentation. Outreach and education were also in focus, with a massive update to the official website's "GSoC Ideas" section, reflecting the community's ambitious roadmap for the upcoming Google Summer of Code season.

## References
- [1] 0d6ba7cb5 — Bazaar: Uniq package for single-instance applications introduced (micio, 2011-02-05)
- [2] 92603f6eb — Bazaar: Uniq implementation completed for Windows (micio, 2011-02-07)
- [3] 2ac4fc2dc — Web: Socket: per-socket error handling introduced (cxl, 2011-02-20)
- [4] ea05b6ec1 — Web: Extended Socket::Data error handling for SSL (rylek, 2011-02-22)
- [5] f6952c9aa — RichEdit: QTF/RTF clipboard support for single objects (rylek, 2011-02-11)
- [6] 3a2f06a74 — CtrlCore: X11 urgency hint support added (cxl, 2011-02-17)
- [7] b1589c8cb — Bazaar: Protect macros made multi-thread safe (micio, 2011-02-08)
- [8] 8f467d656 — CtrlLib: Refactored PopUpTable for improved animation (cxl, 2011-02-18)
- [9] 82676c663 — Functions4U: StaticPlugin architecture introduced (koldo, 2011-02-12)
- [10] dae434017 — OfficeAutomation: Refurbished using StaticPlugin (koldo, 2011-02-25)
- [11] 31cb381aa — PostgreSQL driver updated to use Web package (cxl, 2011-02-08)
- [12] 6996db373 — CtrlLib: Splitter width moved to style definition (unodgs, 2011-02-11)
- [13] 7bc1cbca5 — CtrlLib: ArrayCtrl updated for Option visual styles (rylek, 2011-02-05)
- [14] 5df755139 — Core: ConvertDate::Truncate and related helpers (cxl, 2011-02-10)
- [15] 8c3fea3bc — Bazaar: CtrlProp enhancements and uniform naming (kohait, 2011-02-11)
- [16] aa6164429 — CtrlLib: Improved ColorPopup animation (cxl, 2011-02-18)
- [17] 1ebc70b60 — CtrlCore: EncodeRTF adds page size, margins, and borders (rylek, 2011-02-14)
- [18] f52545f7a — Media: RasterPlayer promoted to releases (cxl, 2011-02-25)
- [19] 8cfbeffad — Core: Fixed direct-only linking for Ubuntu Natty (cxl, 2011-02-25)
- [20] 3a4fcdcef — Bazaar: Alternative Multithreading update (Mindtraveller, 2011-02-09)
