# The Multi-Media IDE and Threaded Locales (2012-06)
**Date Span:** 2012-06-01 to 2012-06-30

June 2012 was a month of significant functional diversification for TheIDE and a deeper push into threaded architecture within the core library. TheIDE underwent a transformation from a source code editor into a more comprehensive development workbench with the introduction of native **multi-media viewing and editing**. The environment gained the capability to view `.png`, `.jpg`, `.gif`, and `.bmp` files directly, and even achieved the ability to edit and create `.png` assets within the workspace. This shift was accompanied by the expansion of syntax highlighting to include **Javascript, CSS, and C#**, reflecting U++'s increasing role in web-adjacent and multi-language projects. A vital quality-of-life feature, "Save file on window deactivation," was also added to prevent data loss during frequent context switching.

The core library's foundational support for multi-threading reached a new level of sophistication. A major architectural change made **language settings per-thread**, ensuring that multi-threaded server applications could handle concurrent requests in different locales without interference. The `Thread` class was also hardened, now automatically catching `Exc` and `ExitExc` exceptions to prevent silent thread crashes. Furthermore, `Thread::Priority` was implemented for POSIX platforms, providing cross-platform parity for high-performance task scheduling.

Configuration management was revolutionized with the introduction of the **INI parameter helpers**: `INI_BOOL`, `INI_STRING`, and `INI_INT`. These macros provided a standardized, declarative way to define application settings that are automatically loaded from configuration files, immediately leveraged to modernize the tracing logic in `HttpRequest`. This period also saw the arrival of the `LOG_` conditional log helper, streamlining diagnostic output in complex systems.

The database and specialized utility layers continued their steady maturation. `MySQL` gained robust `WhenReconnect` support, ensuring stable connections for long-running services, and `SqlLoadTable`/`SqlLoadColumn` were introduced to simplify bulk data retrieval. PostgreSQL was upgraded to support **hex blobs**, maintaining compatibility with version 9.0+. In the Bazaar, the **Tcc** (Tiny C Compiler) integration reached a new milestone, now allowing Tcc instances to call one another, enabling complex runtime-compiled logic chains.

## References
- [1] d38949e7d — ide: Native viewing of PNG, JPG, GIF, and BMP files (cxl, 2012-06-20)
- [2] 69a8fea3d — ide: Native PNG editing and creation (cxl, 2012-06-21)
- [3] 0257c6fe8 — ide: Javascript syntax highlighting introduced (cxl, 2012-06-20)
- [4] 53f5a3b13 — ide: CSS syntax highlighting support (cxl, 2012-06-27)
- [5] 3f950111d — ide: C# syntax highlighting added (rylek, 2012-06-26)
- [6] 52fc30693 — Core: Language settings became per-thread (cxl, 2012-06-19)
- [7] 423409da1 — Core: Thread now catches Exc and ExitExc (cxl, 2012-06-11)
- [8] 3c2194aad — Core: Thread::Priority implemented for POSIX (cxl, 2012-06-08)
- [9] 74c558a40 — Core: INI parameter helpers (INI_BOOL, INI_STRING, INI_INT) (cxl, 2012-06-28)
- [10] 76a6f44a1 — Core: LOG_ conditional log helper introduced (cxl, 2012-06-29)
- [11] ca0e70b11 — MySQL: WhenReconnect support added (cxl, 2012-06-03)
- [12] c492aaed2 — Sql: SqlLoadTable and SqlLoadColumn introduced (cxl, 2012-06-08)
- [13] dd8fb364d — PostgreSQL: Support for hex blobs (cxl, 2012-06-26)
- [14] 38970db43 — Tcc: Native support for cross-instance calls (koldo, 2012-06-03)
- [15] a078e848b — ide: Save file on window deactivation option (cxl, 2012-06-20)
- [16] 5f95c7402 — RichText: styles for paragraph ruler (dotted, dashed) (cxl, 2012-06-19)
- [17] 6f0d44b3b — Core: GetTimeZoneText; SMTP time zone fixes (cxl, 2012-06-20)
- [18] 99e5a88d3 — ide: FindInFiles dialog accepts empty search strings (rylek, 2012-06-15)
- [19] 4a534d9ad — CtrlLib: LineEdit scroll enhancements (cxl, 2012-06-08)
- [20] 60bb7914d — Painter: Fixed text length logic for char pointers (cxl, 2012-06-12)
