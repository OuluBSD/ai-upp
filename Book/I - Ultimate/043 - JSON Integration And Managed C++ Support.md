# JSON Integration and Managed C++ Support (2011-11)
**Date Span:** 2011-11-01 to 2011-11-30

November 2011 marked a major expansion of U++'s core data handling and its integration with the broader Microsoft development ecosystem. The most significant architectural addition was the introduction of native **JSON support** within the `Core` library. This new suite, featuring the `ParseJSON` engine and a dedicated reference example, allowed U++ applications to consume and produce JSON data with the same ease as XML. A critical part of this integration was the enhancement of the `Value` class, which can now directly access `ValueArray` and `ValueMap` elements using an intuitive indexing operator, effectively treating a nested `Value` tree as a native JSON object.

TheIDE achieved a long-awaited milestone with the introduction of **CLR (Common Language Runtime) support**. By enabling the compilation of managed C++ code, U++ gained the ability to interoperate directly with the .NET framework on Windows. This addition was carefully tuned to maintain performance, ensuring that high-performance features like SSE2 instructions were not automatically disabled when CLR was active. The environment's intelligence was also bolstered: layout files (`.lay`) now feature full syntax highlighting, and Assist++ was upgraded to provide precise line-number information for database schemas (`.sch`) and internal implementation files (`.icpp`).

The **Rainbow** project reached a new level of visual sophistication through the **WinGL** backend. Development focused on high-performance rendering techniques, including the implementation of **automatic atlas textures** to reduce state changes and the creation of smoother title bar gradients. The `RichText` system was also hardened, gaining the `CreatePaintingObject` method and improved PDF rendering capabilities, while `UWord` was refined to natively handle the `.rtf` extension for both loading and saving.

Core library ergonomics and globalization were further refined. Support for a **secondary translation (.tr) language** was added, providing a fallback mechanism for multi-lingual applications. The `CParser` was updated for better const-correctness and now correctly handles javascript-style unicode escapes (`\u`), a vital requirement for the new JSON engine. The Bazaar saw the first release of the **MapRender** suite—a comprehensive set of tools for GIS and mapping applications—and the `HelpViewer` was upgraded with standard browser-like "Back" and "Forward" navigation. Connectivity was also polished, with `HttpClient` gaining a refined `PUT` implementation and the `Sqlite3` plugin being upgraded to version 3.7.8.

## References
- [1] d3ad4971a — Core: Native JSON support and Value indexing introduced (cxl, 2011-11-04)
- [2] f2d1a18a8 — Core: JSON reference and ParseJSON introduced (cxl, 2011-11-05)
- [3] f29434b64 — theide: CLR (Managed C++) support introduced (cxl, 2011-11-06)
- [4] ee1c55543 — theide: Syntax highlighting for .lay files; Assist++ improvements (cxl, 2011-11-12)
- [5] 073843d1c — Rainbow: WinGL automatic atlas textures and bug fixes (unodgs, 2011-11-22)
- [6] 7ac2a419a — Bazaar: MapRender first release (Sc0rch, 2011-11-18)
- [7] be4af8828 — Core: Support for secondary .tr language added (cxl, 2011-11-07)
- [8] e13ae5cbb — Core: CParser adds Javascript-style Unicode escape support (cxl, 2011-11-11)
- [9] b6aa6359e — XmlRpc: Extended structure compatibility for containers (cxl, 2011-11-14)
- [10] d14fa94a3 — Sql: REFERENCES in .sch can be preset by Sql (cxl, 2011-11-16)
- [11] 455f92095 — Bazaar: HelpViewer adds Back and Forward buttons (micio, 2011-11-03)
- [12] 3383cff23 — theide: Virtuals and Goto now support partial matching (cxl, 2011-11-04)
- [13] 76877dd83 — theide: Copy File Path feature added (cxl, 2011-11-07)
- [14] 0e9893388 — CtrlLib/ide: Support for UTF8-BOM encoding added (cxl, 2011-11-13)
- [15] c5826d7b9 — Core: Support for decimal comma in double formatting (cxl, 2011-11-08)
- [16] b6494621f — WinGL: Smoother title bar gradients and texture filtering (unodgs, 2011-11-26)
- [17] d12fb1eb8 — plugin/Sqlite3: Upgraded to version 3.7.8 (cxl, 2011-11-04)
- [18] 8731fd5b6 — Core: ValueMap::Set(key, value) introduced (cxl, 2011-11-02)
- [19] 452430a58 — theide: Rename layout by double-clicking the name (cxl, 2011-11-16)
- [20] 301c1ca47 — Core: BlockStream zero-length read/write fixes (cxl, 2011-11-16)
