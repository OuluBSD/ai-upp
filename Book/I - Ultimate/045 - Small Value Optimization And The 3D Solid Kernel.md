# Small Value Optimization and the 3D solid Kernel (2012-01)
**Date Span:** 2012-01-01 to 2012-01-31

January 2012 was a month of intensive low-level optimization and the arrival of professional-grade 3D modeling capabilities within the U++ ecosystem. The most significant research effort was the development of **SVO_VALUE** (Small Value Optimization). By refactoring the core `Value` class to store small data types (like integers, booleans, and small dates) directly within the object's memory footprint rather than on the heap, the framework achieved substantial performance gains and reduced memory fragmentation. This effort was supported by exhaustive unittests and benchmarks, eventually resulting in the reintegration of the new `Value` engine under the `SVO_VALUE` conditional compilation flag.

The Bazaar saw a massive leap in engineering utility with the introduction of the **OCE** (OpenCascade Community Edition) package. By wrapping this powerful 3D solid modeling kernel, U++ gained the ability to represent, manipulate, and visualize complex geometric shapes and CAD data—a vital addition for industrial and scientific applications. Parallel to this, the **WinGL** backend for the Rainbow project continued its visual climb, adding support for painting through Framebuffer Objects (FBO), a new blur shader with grayscale support, and optimized children clipping.

Core library ergonomics were bolstered by several quality-of-life additions. The `Date` class gained the `operator++` and `operator--` methods, simplifying chronological arithmetic, while new `Random` number generator functions provided more versatility for simulations. Networking was hardened with **HttpsClient** now correctly utilizing the `CONNECT` method for proxies, and the `Web` package saw improvements in `HttpClient::GetBody` handling for error states. The database layer also matured, with `SqlMassInsert` gaining a `remove` option to streamline data synchronization tasks.

TheIDE and its surrounding deployment infrastructure reached new levels of instance awareness. Environment settings are now automatically saved upon closing the dialog and reloaded in all other active IDE instances, ensuring a consistent developer experience across multiple monitors or projects. The build system saw the launch of **lpbuild2**, a major refactor of the Linux distribution tools featuring local build capabilities and improved Debian packaging. The environment was further polished with support for multiple preconfigured build methods (.bm) on first run and a more informative "out of memory" panic that now displays requested block sizes to aid in debugging heap issues.

## References
- [1] 9d7144e63 — Core: SVO_VALUE (Small Value Optimization) development started (cxl, 2012-01-08)
- [2] 71740dc8d — Core: New Value reintegrated under SVO_VALUE flag (cxl, 2012-01-30)
- [3] 11cc4cff3 — Bazaar: OCE (OpenCascade Community Edition) 3D kernel added (micio, 2012-01-15)
- [4] b5711acfd — Rainbow: WinGL adds FBO painting and blur shader (unodgs, 2012-01-06)
- [5] 006f77997 — Web: HttpsClient correctly uses CONNECT for proxy (cxl, 2012-01-05)
- [6] 07a260bd6 — ide: Environment settings synchronized between instances (cxl, 2012-01-30)
- [7] 7bbf60ba4 — Core: Date gained operator++ and operator-- (cxl, 2012-01-09)
- [8] 5553b45f3 — Core: New Random number generator functions (cxl, 2012-01-18)
- [9] 6050e5d0e — lpbuild2: New Debian packaging and local build support (dolik, 2012-01-26)
- [10] b2f2cc2a8 — Core: Informative out-of-memory panic (cxl, 2012-01-26)
- [11] 239a3ed22 — Core: ScanDate utility added (cxl, 2012-01-19)
- [12] 37aa7419b — Sql: MassInsert adds remove option (cxl, 2012-01-19)
- [13] 9bf60dd35 — Core: FindFile::GetPath introduced (cxl, 2012-01-20)
- [14] 0ecc17b30 — WinGL: Texture alignment and grayscale blur (unodgs, 2012-01-09)
- [15] 188bda0ad — ide: MSC64 builder SSE2 logic refined (cxl, 2012-01-20)
- [16] aaef70550 — ide: CppBuilder restores relative '..' path support (cxl, 2012-01-12)
- [17] 8ea399a02 — ide: Support multiple preconfigured .bm on first run (dolik, 2012-01-26)
- [18] 24fef42f2 — ide: Fixed conflict and 'svn add' issues in usvn (cxl, 2012-01-28)
- [19] abc06ee3d — Web: HttpClient virtual destructor added (cxl, 2012-01-03)
- [20] 27bd4913f — Web: HttpClient::GetBody handle for error cases (cxl, 2012-01-31)
