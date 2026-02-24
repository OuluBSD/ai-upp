# Skylark Uploads and fork-safe Random (2013-08)
**Date Span:** 2013-08-01 to 2013-08-31

August 2013 was a month of significant functional expansion for the **Skylark** web framework and a deep hardening of the core library's foundational primitives. The most prominent addition to the web stack was the introduction of native **progress handling** and **array shared variables**. By allowing the framework to track and report the state of long-running operations—particularly multi-file uploads—U++ enabled the creation of much more responsive and professional web interfaces. This was immediately demonstrated in the new `SkylarkUpload` example, which showcased a modern, multi-file upload application utilizing session variables for progress tracking rather than legacy static maps.

The core library reached a new peak of reliability with the introduction of a **fork-safe Random** number generator. By ensuring that child processes created via `fork()` receive unique random seeds, the framework eliminated a common source of cryptographic and logic errors in multi-process Linux applications. This was supported by the new `SeedRandom` utility and a refactor of the `Sort` family to include `SortByKeys` and `SortByValues`, providing more expressive ways to organize associative data.

TheIDE and its professional toolchain continued their climb toward full modern standards. The environment was upgraded with **C++11 keyword highlighting**, ensuring that developers leveraging next-generation language features received the same visual feedback as traditional C++. Support for **MSC12 (Visual Studio 2013)** was also landed, keeping the framework current with the latest Microsoft compiler releases. The **Layout Designer** gained a much-requested feature: the ability to sort layout items by their screen position, significantly streamlining the management of complex, multi-widget forms.

User interface ergonomics and database integration remained a focus. The **Sql** layer was refined so that `S_*` table structures can now dynamically `Get(column)` by index or name, bridging the gap between static record mappings and dynamic data access. `CtrlLib` saw refinements across its core widget set: `ColumnList` now correctly updates scrollbars after manual width/height adjustments, `DropList` added `SetValueConvert` support, and `EditNumber` was hardened with several community-contributed fixes. The month closed with the addition of the **ADT/List.h** package to the Bazaar, providing a new doubly-linked list implementation for specialized container needs.

## References
- [1] 86ed99f16 — Skylark: Progress handling and array shared variables added (micio, 2013-08-01)
- [2] a2c93584c — examples: SkylarkUpload multi-file upload web application (micio, 2013-08-01)
- [3] 17e92e4b1 — Core: Random number generator made fork-safe (cxl, 2013-08-07)
- [4] 82404cca4 — Core: SeedRandom utility introduced (cxl, 2013-08-07)
- [5] d539ef436 — ide: C++11 keywords syntax highlighting added (cxl, 2013-08-25)
- [6] 7f59ce607 — ide: Support for MSC12 (Visual Studio 2013) builder (cxl, 2013-08-25)
- [7] 222568da8 — ide: Layout designer: sort items by position (cxl, 2013-08-21)
- [8] 4d1da4405 — Sql: S_* table structs gain Get(column) method (cxl, 2013-08-02)
- [9] 952e96694 — Core: SortByKeys and SortByValues introduced (cxl, 2013-08-19)
- [10] 1f37512e4 — Core: ValueMap::RemoveKey method added (cxl, 2013-08-20)
- [11] fcc76f2d0 — CtrlLib: ColumnList updates scrollbars on item resize (cxl, 2013-08-06)
- [12] 789805538 — CtrlLib: DropList adds SetValueConvert; plugin/jpg adds EXIF orientation (cxl, 2013-08-08)
- [13] 42d6936a0 — CtrlLib: TreeCtrl now utilizes GetMinSize for widgets (cxl, 2013-08-18)
- [14] d78cd99c1 — Core: HttpRequest always sends Content-Length for POST/PUT (cxl, 2013-08-09)
- [15] eed6758d3 — Core: Socket::Connect now provides error descriptions (cxl, 2013-08-11)
- [16] aa3cee055 — ide: Recursion limit for assist template parameter resolving (cxl, 2013-08-11)
- [17] a2ca885eb — CtrlLib: ErrorOK utility introduced (cxl, 2013-08-17)
- [18] 864ac7cfe — Bazaar: ADT/List.h doubly-linked list added (novo, 2013-08-23)
- [19] f3675a58f — Draw: Support for flagCFONTSYS (cxl, 2013-08-25)
- [20] 3130f89ff — TabBar: Critical crash fix (cxl, 2013-08-25)
