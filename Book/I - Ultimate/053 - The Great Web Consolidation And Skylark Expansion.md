# The Great Web Consolidation and Skylark Expansion (2012-09)
**Date Span:** 2012-09-01 to 2012-09-30

September 2012 was a month of significant structural simplification and the continued rise of U++'s next-generation web capabilities. The most profound organizational change was the formal **removal of the legacy Web package** from both `uppsrc` and TheIDE. With its core networking primitives (like `TcpSocket` and `HttpRequest`) now firmly rooted in the `Core` library, the framework achieved a much cleaner architecture. This consolidation triggered a massive update across the entire repository: canonical examples like `GoogleMaps`, `AddressBook`, and various SQL references were modernized to utilize the new `Core`-based networking APIs, effectively "burning the bridges" to the older, more fragmented web stack.

The **Skylark** web framework continued its aggressive expansion into professional territory. A major architectural addition was **SkylarkPack**, a system for grouping library handlers into reusable modules, supported by comprehensive new documentation. The framework's template engine was also refined, with the `$set()` function now capable of listing shared variables, and a new tutorial (`Skylark12`) was launched to cover advanced modular patterns. To support professional production environments, the framework added native documentation for **deploying Skylark applications**, ensuring that developers could move from development to production with standardized practices.

Core library and database efficiency reached new peaks. The **TcpSocket::Accept** logic was hardened against preforking issues, and `MassInsert` was specifically optimized for PostgreSQL and MySQL to leverage native bulk operations. The SQL expression engine gained the `Sqls::operator^`, and `CParser` continued its march toward robustness, with its internal lexer now throwing exceptions instead of asserting—a vital change for building resilient parsers in TheIDE. Networking was further polished with `TcpSocket::Listen` now defaulting to a sensible `listen_count` of 5, and `HttpClient` gained full support for JSON parameters and return values.

TheIDE and its surrounding deployment infrastructure were further professionalized. The environment added native documentation for the **.upp package format**, providing transparency into U++'s internal build definitions. The build system added support for **pkg-config** include files within release scripts, streamlining the handling of complex external dependencies on Linux. User interface ergonomics were also a focus: `EditField` gained the **WhenPasteFilter** callback, allowing for real-time data sanitization during clipboard operations, and `MenuBar` logic was refined to prevent duplicate menu instances from the same owner, ensuring a more stable and predictable UI experience.

## References
- [1] 1895836fb — ide: Legacy Web package formally removed (cxl, 2012-09-03)
- [2] 4c24235f9 — Skylark: SkylarkPack for grouped library handlers (cxl, 2012-09-09)
- [3] 1ba0f5f22 — docs: Deploying Skylark applications guide (dolik, 2012-09-30)
- [4] 36700069b — ide: Formal documentation for .upp file format (cxl, 2012-09-09)
- [5] 346c0e972 — Core: Fixed prefork issue in TcpSocket::Accept (cxl, 2012-09-03)
- [6] dda4dbff5 — Sql: MassInsert optimized for PGSQL and MySQL (cxl, 2012-09-11)
- [7] 37e4c669e — CtrlLib: EditField adds WhenPasteFilter callback (cxl, 2012-09-06)
- [8] 6cf239033 — Core/Rpc: Full support for JSON parameters and return values (cxl, 2012-09-14)
- [9] 2905cf16f — Scripts: Support for pkg-config include files added (cxl, 2012-09-07)
- [10] 9877cd2a1 — CtrlLib: MenuBar prevents double menus from same owner (cxl, 2012-09-29)
- [11] 0943d5803 — Skylark: Http::SetHeader and Http::ClearHeader added (cxl, 2012-09-09)
- [12] 3e1a9b2e3 — Skylark: Native SigUsr1 signal handler (cxl, 2012-09-09)
- [13] 7797e6786 — tutorial: Skylark12 advanced modularity tutorial (cxl, 2012-09-10)
- [14] 315f91230 — ide: Parser lexer now throws instead of ASSERTing (cxl, 2012-09-13)
- [15] 6e4b6797b — Draw: ImageMaker cache growth optimization (cxl, 2012-09-15)
- [16] 129581b75 — Bazaar: DXF package adds Point entity (micio, 2012-09-05)
- [17] 17f3f2d16 — Core: TcpSocket::Listen defaults to listen_count = 5 (cxl, 2012-09-22)
- [18] 081dcb54c — examples: GoogleMaps updated to use Core networking (cxl, 2012-09-22)
- [19] 698d6cc55 — reference: Native plugin/zip demonstration (cxl, 2012-09-26)
- [20] 9feade0a1 — Skylark: $set() function improved for shared variables (cxl, 2012-09-16)
