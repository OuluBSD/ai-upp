# The Standalone umk and the Firebird Driver (2011-08)
**Date Span:** 2011-08-01 to 2011-08-31

August 2011 was a definitive month for U++ build automation and the continued expansion of its enterprise database suite. The most significant architectural shift was the emergence of the "real" **umk**—the U++ Make utility—as a standalone, reduced-footprint tool. While the framework had always possessed a build engine, this new iteration was optimized for headless environments and command-line efficiency. It introduced support for "inline" assembly paths, customizable build methods, and a new "silent mode" for integration into large-scale automated pipelines. This transformation completed the framework's journey toward a truly decoupled build infrastructure, allowing developers to compile complex U++ assemblies without ever launching a graphical interface.

The database layer crossed another major milestone with the introduction of the initial **Firebird/Interbase driver**. This addition provided U++ applications with native connectivity to one of the most prominent open-source relational databases, further rounding out a suite that already included robust support for PostgreSQL, Oracle, SQLite, and MySQL. To support this expanding ecosystem, `SqlExp` was refined to handle union results with parentheses for sorting, and the `MySql` driver gained `GetTransactionLevel` support.

Scientific computing became even more integrated as the **Eigen** matrix algebra library was promoted from a general Bazaar package to a formal core plugin (`plugin/Eigen`). This move simplified installation and ensured that high-performance linear algebra was readily available to all U++ projects. Simultaneously, the community-driven **Controls4U** package reached a new level of utility by absorbing specialized controls like `SliderCtrlX` and the `StarIndicator`, consolidating various UI experiments into a single, well-maintained collection.

TheIDE received a powerful new documentation tool: **T++ to HTML export**. This feature allowed developers to instantly convert their rich text Topic++ documentation into standard web pages, facilitating the generation of online manuals directly from the source code. The editor itself became more "culturally aware," with the **CodeEditor** now regarding accented characters as valid identifier letters, a vital change for developers in non-English speaking regions. Professional polish continued with `DropList` gaining full Unicode support and `GridCtrl` adding support for three-state options, while the **Rainbow WinGL** backend continued its steady climb toward production readiness.

## References
- [1] c611fa851 — 'real' umk standalone make utility introduced (cxl, 2011-08-05)
- [2] 77ec98884 — umk: support for inline assembly and build methods (cxl, 2011-08-06)
- [3] 34db65ca4 — Sql: Initial Firebird/Interbase driver added (novo, 2011-08-30)
- [4] 509ac7b38 — Eigen: Promoted to core plugin/Eigen (koldo, 2011-08-01)
- [5] 8b07483ad — theide: T++ documentation export to HTML (cxl, 2011-08-04)
- [6] 5e9b7e1c8 — Controls4U: Added SliderCtrlX and StarIndicator (koldo, 2011-08-20)
- [7] ff50b51cb — GridCtrl: Added ThreeStateOption support (unodgs, 2011-08-29)
- [8] 3e7c3f1a5 — ide: CodeEditor regards accented characters as ID letters (cxl, 2011-08-28)
- [9] 2304dd39e — CtrlLib: DropList now supports Unicode (cxl, 2011-08-30)
- [10] 9cd52f49b — Core: XML:IsAttr helper added (unodgs, 2011-08-01)
- [11] 8a67522b6 — Oracle: Added default POSIX names for OCI libraries (rylek, 2011-08-08)
- [12] 3000308a8 — CtrlCore: Chameleon synced after parameter changes (cxl, 2011-08-15)
- [13] 2f20381fd — CtrlCore: SyncCh moved to Ctrl constructor (cxl, 2011-08-16)
- [14] 31df61a2f — Painter: RectPath now returns *this for chaining (cxl, 2011-08-13)
- [15] 2bbd5b470 — Functions4U: SaveImage, PrintImage, and DrawRectLine added (koldo, 2011-08-13)
- [16] bd9a46849 — RichText: 
 support in DeQtf; fixed ParseQtf packing (rylek, 2011-08-09)
- [17] 8c319eedb — Bazaar: CtrlPos and CtrlFinder gain WhenBar support (kohait, 2011-08-30)
- [18] 4acc76ace — Bazaar: CtrlFinder supports filtering of controls (kohait, 2011-08-01)
- [19] a85f5d4cb — Rainbow: Ongoing development of WinGL backend (unodgs, 2011-08-15)
- [20] bc967252d — Web: fixed reading long SAPI requests in httpsrv (rylek, 2011-08-26)
