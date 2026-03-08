# Core Sort Refactor and CAD Foundations (2012-08)
**Date Span:** 2012-08-01 to 2012-08-31

August 2012 was a month of deep algorithmic refinement and the expansion of the U++ ecosystem into specialized engineering domains. The most fundamental core advancement was a complete refactor of the **Sort algorithm**. By optimizing the internal logic to avoid "all equal" pathological cases—which previously could lead to O(n²) performance or recursion depth issues—the framework achieved more predictable and robust data processing. This change was supported by a suite of new unittests and cross-compiler fixes to ensure parity between GCC and MSC builds.

The Bazaar saw a major leap in engineering utility with the introduction of the **DXF package**. Designed for writing CAD (Computer-Aided Design) files, this package provided U++ developers with a high-level API for generating technical drawings compatible with industry-standard software. The implementation arrived with sophisticated features: automatic bounding-box calculation for "zoom to extents" saving, robust layer and linetype management, and transform-aware polyline scaling. To ensure professional quality, it featured full internationalization support (Italian and German) and standardized character conversion to the Windows-1252 charset for legacy CAD compatibility.

The **Skylark** web framework continued its rapid march toward professional maturity. The development focus shifted toward exhaustive documentation, particularly for the **Renderer** and the **Witz** template engine. Functional fixes addressed session table configuration via INI files and resolved specific `witz` parsing quirks. TheIDE itself became more responsive during this period, with a "holiday batch" of optimizations that improved environment startup times and introduced an **auxiliary thread for lazy icon loading** in `FileSel` on Win32. This change ensured that the UI remained fluid even when browsing directories with complex file associations.

Database and serialization layers reached new levels of enterprise readiness. **MySQL** support was expanded to include multi-column unique constraints, providing developers with finer control over data integrity at the schema level. `SqlMassInsert` gained the `NoUseTransaction` option, offering more flexibility for bulk operations within larger, manually managed transaction blocks. Core library hardening continued with fixes for `INI_INT` default value handling and a critical update to the **JSON engine**, which now correctly escapes key strings, ensuring full compliance with the JSON specification for complex map structures.

## References
- [1] d92758041 — Core: Sort algorithm refactored and optimized (cxl, 2012-08-03)
- [2] e724b7c34 — Core: Sort algorithm avoids 'all equal' pathological cases (cxl, 2012-08-29)
- [3] 0c3824d8b — Bazaar: DXF package for CAD file writing introduced (micio, 2012-08-11)
- [4] 5c3bb71a9 — Bazaar: DXF adds GetBoundingBox and zoom-to-extents (micio, 2012-08-12)
- [5] 8a6e1ad46 — Skylark: Renderer documentation launched (cxl, 2012-08-05)
- [6] 3313a27eb — Skylark: Witz template engine fixes (cxl, 2012-08-26)
- [7] 4643c6f47 — theide: Startup performance and lazy icon loading thread (cxl, 2012-08-25)
- [8] 947ef307e — MySql: Support for multi-column unique constraints (cxl, 2012-08-28)
- [9] 0799e7e85 — Sql: SqlMassInsert adds NoUseTransaction option (cxl, 2012-08-28)
- [10] fbdc32e50 — Core: JSON properly escapes key strings (cxl, 2012-08-29)
- [11] 6f1c4c45d — Core: Fixed INI_INT default value handling (cxl, 2012-08-26)
- [12] 407c44a10 — RichText: Resolved deadlock situation (cxl, 2012-08-04)
- [13] 174c4d8e2 — Core: GetIniKey disallowed outside APP_MAIN in debug (cxl, 2012-08-09)
- [14] f3fecf9c4 — Bazaar: DXF fixed block reference rotation angles (micio, 2012-08-13)
- [15] 8f91efe80 — Bazaar: DXF scales polyline width with transform matrix (micio, 2012-08-14)
- [16] ad9e19446 — theide: Gdb_MI2 value inspectors removed for instability (micio, 2012-08-05)
- [17] 7445ac34c — theide: SelectPkg refactored and fixed (cxl, 2012-08-26)
- [18] e00505767 — Skylark: .ini session_table issue fixed (cxl, 2012-08-28)
- [19] 31df61a2f — Painter: RectPath returns *this for chaining (cxl, 2011-08-13)
- [20] 9cd52f49b — Core: XML:IsAttr helper added (unodgs, 2011-08-01)
