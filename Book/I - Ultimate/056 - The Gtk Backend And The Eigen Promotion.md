# The Gtk Backend and the Eigen Promotion (2012-12)
**Date Span:** 2012-12-01 to 2012-12-31

The final month of 2012 was defined by a major push into modern Linux integration and the formal promotion of high-performance scientific libraries. The **Rainbow** project reached a critical milestone with the intensive development of the **Gtk backend**. This new architecture, designed to provide a truly native and modular interface for Gtk-based environments, saw the rapid implementation of core features: **GdkPixbuf** based image handling, native cursor support, skinning integration, and foundational support for primary selections and **Drag & Drop (D&D)**. This effort signaled U++'s transition toward a more decoupled GUI abstraction, where backends can be swapped or optimized without affecting the high-level toolkit.

The framework's scientific suite reached full canonical status as **Eigen** was moved from the plugins directory to the core `uppsrc` collection. This promotion reflected the library's status as an indispensable tool for U++ developers working on matrix algebra and advanced mathematical algorithms. To ensure production quality, Eigen was upgraded to version 3.1.2, accompanied by updated demos and exhaustive documentation. Simultaneously, the **ScatterDraw** and **ScatterCtrl** packages received major updates, including the beginning of a comprehensive documentation drive and the addition of basic menus for real-time plot manipulation.

The core library and database Expression engine continued their steady climb toward enterprise-grade robustness. The `Sql` script parser was upgraded to support the **PostgreSQL $$ notation**, simplifying the execution of complex multi-line stored procedures. Core library optimizations were abundant: `String::Cat` was further tuned, and the `Id` class was optimized for multi-threading using Thread-Local Storage (TLS). A vital ergonomic addition was the introduction of the **DRAWTEXTLINES** option in `Draw::GetInfo`, providing finer control over text rendering metrics.

TheIDE and professional toolchains were refined for high-efficiency workflows. Custom build steps were enhanced with the **$(FILEDIR)** macro, and the environment achieved better resilience by automatically backing up configuration files. Translation support reached a new peak with the addition of **Dutch (nl-nl)** and updated **Brazilian Portuguese (pt-br)** translations. The month closed with a major update to the "4U" community initiatives: `Functions4U` and `SysInfo` underwent deep restructuring to improve cross-platform compatibility, and `Controls4U` achieved a milestone where its translations now work consistently across all supported languages.

## References
- [1] 25e2740c3 — Rainbow: Initial implementation of the Gtk backend (cxl, 2012-12-04)
- [2] 6aeb26e84 — Rainbow: Gtk backend adds D&D drop implementation (cxl, 2012-12-31)
- [3] ac593d693 — uppsrc: Eigen matrix library promoted to canonical Core (cxl, 2012-12-08)
- [4] 4aca0cbe5 — plugin/Eigen: Upgraded to version 3.1.2 (koldo, 2012-12-28)
- [5] 3e2656e53 — Sql: Support for PGSQL $$ string quoting in scripts (cxl, 2012-12-03)
- [6] f8434a4d3 — Draw: DRAWTEXTLINES option added to GetInfo (cxl, 2012-12-02)
- [7] 5f3994b73 — ide: Custom build steps gain $(FILEDIR) macro (cxl, 2011-12-06)
- [8] ee3d86703 — i18n: Dutch (nl-nl) translation added (cxl, 2012-12-09)
- [9] e90ce8af2 — i18n: Brazilian Portuguese (pt-br) translation updated (cxl, 2012-12-09)
- [10] facdd3e8c — ScatterDraw: Improvements and documentation drive (koldo, 2012-12-28)
- [11] 20fe30a1b — Functions4U: Major maintenance and feature update (koldo, 2012-12-28)
- [12] 673d16ef7 — SysInfo: Major update for cross-platform hardware detection (koldo, 2012-12-28)
- [13] e388b4a20 — Controls4U: Standardized translation support (koldo, 2012-12-28)
- [14] cddea4322 — Rainbow: Gtk backend skinning support added (cxl, 2012-12-30)
- [15] 6d9502eba — Rainbow: Gtk backend adds primary selection support (cxl, 2012-12-30)
- [16] 1aa3cf393 — Draw: Image::GetAuxData made public (cxl, 2012-12-12)
- [17] e380243e2 — CtrlLib: ArrayCtrl::OverrideCursor method added (cxl, 2012-12-11)
- [18] 7f651003c — RichText: Fixed issues with zero-sized objects (cxl, 2012-12-25)
- [19] d5e505280 — Controls4U: StarIndicator introduced (koldo, 2011-12-02)
- [20] 1c23564b4 — Core: Fixed ValueMap::Add(0, ...) edge case (cxl, 2012-12-01)
