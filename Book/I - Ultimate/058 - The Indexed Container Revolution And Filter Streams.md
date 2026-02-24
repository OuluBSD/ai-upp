# The Indexed Container Revolution and Filter Streams (2013-02)
**Date Span:** 2013-02-01 to 2013-02-28

February 2013 was a transformative month for the U++ core library, marked by a massive expansion of the container suite and the introduction of advanced stream processing. The most significant development was the launch of the **Indexed Container** family. Led by the move of **InVector** and **InArray** into the canonical `Core` library, the framework introduced high-performance indexed structures that provide logarithmic access to elements. This was immediately followed by the introduction of **SortedIndex**, **SortedVectorMap**, and **SortedArrayMap**, providing developers with a complete set of sorted associative containers. To ensure production-grade performance, the **InVectorCache** was developed, and the entire container suite underwent a rigorous "sizeof" optimization pass to minimize memory footprint.

Stream processing reached a new level of versatility with the introduction of **FilterStream** and **FilterStreams**. This architectural addition provided a standardized, layered approach to data processing, allowing developers to wrap standard streams with transparent filters for tasks like compression, encryption, or character set conversion. This was supported by a new suite of unittests and reference examples. Parallel to this, the core `Stream` interface was refactored to support **memory blocks larger than 2GB**, transitioning internal offsets from 32-bit to 64-bit compatibility—a vital requirement for modern high-capacity data processing.

The framework's journey toward full **64-bit parity** reached a major milestone this month. Led by the "4U" community initiatives, a concerted effort was made to ensure that `Controls4U`, `Functions4U`, `ScatterCtrl`, and `ScatterDraw` were fully operational on 64-bit platforms. This effort included hundreds of targeted fixes for Win64 and POSIX-64 warnings, ensuring that U++ remained a top-tier choice for the 64-bit era. Even specialized packages like `OfficeAutomation` were refurbished for 64-bit reliability.

Professional tooling and connectivity continued their steady advancement. TheIDE was refined with a dedicated icon for `.ddl` files and an optimized console output that no longer choked on large datasets. The **Skylark** web framework received improved error handling and a fundamental change in variable management: the `Http` object now defaults to overwriting variables rather than appending them, simplifying state management in complex web apps. The database layer was hardened with a critical fix for **ODBC Client 11.0** crashes, and the **RegExp** plugin was enhanced with native `Replace` methods. In the Bazaar, the **DXF** package matured with support for arcs and bulges in polylines, further cementing U++'s utility in technical drawing and engineering.

## References
- [1] 8617797a9 — Core: InVector moved to Core; MemoryProfile fixed (cxl, 2013-02-10)
- [2] 84b2c33ee — Core: InArray introduced (cxl, 2013-02-10)
- [3] 44d599832 — Core: SortedIndex introduced (cxl, 2013-02-11)
- [4] 93dc3cac0 — Core: SortedVectorMap and SortedArrayMap introduced (cxl, 2013-02-16)
- [5] f1c132057 — Core: InVectorCache introduced for performance (cxl, 2013-02-23)
- [6] 156acf2cb — Core: FilterStream and FilterStreams suite introduced (cxl, 2013-02-19)
- [7] 02544bc7a — Core: Stream support for memory blocks > 2GB (cxl, 2013-02-18)
- [8] a7adad0e5 — Core: Container sizeof optimizations (cxl, 2013-02-16)
- [9] 009ab9b21 — Controls4U: Now runs on 64 bits (koldo, 2013-02-07)
- [10] e01bd536c — ScatterCtrl: Now runs on 64 bits; bug fixes (koldo, 2013-02-07)
- [11] 1b8804df5 — Skylark: Improved error handling (cxl, 2013-02-09)
- [12] c34ed46eb — Skylark: Http overwrites variables by default (cxl, 2013-02-09)
- [13] a16cf0b98 — ODBC: Fixed crash issue with client 11.0 (cxl, 2013-02-05)
- [14] 60d67eb1d — plugin/RegExp: Replace methods introduced (cxl, 2013-02-19)
- [15] 88320c24a — Image: Serialize big image support added (cxl, 2013-02-07)
- [16] dbb222b6e — Bazaar: DXF adds Arc and bulge to polyline (micio, 2013-02-02)
- [17] a04b22196 — ide: Assigned icon for .ddl files (cxl, 2013-02-18)
- [18] 8664fabb9 — ide: Optimized Console output for large data (cxl, 2013-02-16)
- [19] a22e3a4b8 — Core: Index constructor from Vector made explicit (cxl, 2013-02-26)
- [20] 6fc64a144 — Core: Stream::GetPtr introduced (cxl, 2013-02-20)
