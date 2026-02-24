# SQL Introspection and the Thread-Safe Id (2011-12)
**Date Span:** 2011-12-01 to 2011-12-31

The final month of 2011 was a period of high-level architectural refinement, particularly in how U++ bridges the gap between database schemas and user interfaces. The most significant advancement was the introduction of **SQL Introspection** capabilities. By enabling the framework to query its own `.sch` schema definitions at runtime, U++ gained the ability to automatically generate UI controls. The new `SqlCtrls` suite can now create and bind specialized editors based on database column types without manual declaration, a feature showcased in new automatic-SCH reference examples. This was supported by the arrival of `Sql::JoinRef`, which simplified the expression of complex table relationships, and the ability to fetch entire database rows directly into a `ValueMap`.

The core library's foundation was significantly strengthened with a major refactor of the **Id** class. Transitioning to a `String`-based implementation and optimized for multi-threading using Thread-Local Storage (TLS), the new `Id` system provided better performance and safety for concurrent applications. This change was deep-reaching, requiring refinements in `ArrayCtrl` and other core components to accommodate the new identity model. The library also gained the `force_inline` macro to aid in micro-optimizations, while `String::Cat` underwent further performance tuning.

TheIDE and the build system reached new levels of professional versatility. The environment's intelligence was bolstered by the addition of **error highlighting** for erroneous files and packages in the browser, making it easier to pinpoint build failures in large projects. Custom build steps were enhanced with the `$(FILEDIR)` macro and more precise directory pointers, while the Hydra build engine's thread limit was increased to 64 to leverage contemporary multi-core workstations. Documentation was also a focus, with major updates to the Assist++ and Topic++ manuals, and the introduction of a new language selector for the official website.

In the Bazaar and community-driven space, the **Controls4U** package added the `StarIndicator` for rating-style inputs, and the **XMLMenu** system was refined with support for custom command strings and embedded frames. The **Rainbow** project continued its steady progress with fixes for text clipboard handling and coordinate alignment in the **WinGL** backend. Networking was further hardened with improved HTTP compliance in `XmlRpc` and per-thread SQL option support, ensuring that U++ remained a top-tier choice for complex, secure, and data-intensive applications as it entered 2012.

## References
- [1] 5b58dd5b8 — SqlCtrl: Automatic creation based on schema introspection (cxl, 2011-12-16)
- [2] cebc2c14a — Sql: JoinRef first iteration (cxl, 2011-12-07)
- [3] ad58d98f2 — Sql: Row fetching into ValueMap support (cxl, 2011-12-19)
- [4] 10094501a — Core: Id refactored to String-based, TLS optimized (cxl, 2011-12-08)
- [5] 37cbeeddb — ide: Error highlighting for files and packages (cxl, 2011-10-02)
- [6] 5f3994b73 — ide: Custom build steps add $(FILEDIR) macro (cxl, 2011-12-06)
- [7] 35083af23 — Core: force_inline introduced; String::Cat optimized (cxl, 2011-12-02)
- [8] d5e505280 — Controls4U: StarIndicator introduced (koldo, 2011-12-02)
- [9] a6b59dd32 — Sql: SqlSession::ThrowOnError introduced (cxl, 2011-12-19)
- [10] 988e51f30 — Sql: Per-thread SQL options and refactored cursors (cxl, 2011-12-10)
- [11] 1d22b28a4 — Sql: Sql::Attach/Detach and SetSession (cxl, 2011-12-15)
- [12] 127a3fcd3 — uppweb: Language selector introduced (dolik, 2011-12-13)
- [13] d6ba2c4c9 — ide: Convert selection to ASCII; structured flags (rylek, 2011-12-21)
- [14] d127febd9 — CtrlLib: FileSel refreshes system places on open (unodgs, 2011-12-18)
- [15] d4ca68b65 — Rainbow: Fixed text clipboard issues (cxl, 2011-11-10)
- [16] ec25f1eee — WinGL: Fixed texture coordinate alignment (unodgs, 2011-12-03)
- [17] 8fa529d0c — Sql: Trace log replaces placeholders with actual parameters (cxl, 2011-12-08)
- [18] b3c465813 — ide: Navigator bar items for layouts selectable as text (cxl, 2011-12-22)
- [19] 4ac174862 — ide: Topic++ sorts files for all.i consistency (cxl, 2011-12-17)
- [20] b20657f7c — SysInfo: 64-bit compatibility updates (koldo, 2011-12-08)
