# ETag Caching and the Error-Aware IDE (2012-10)
**Date Span:** 2012-10-01 to 2012-10-31

October 2012 was a month of professional hardening for the U++ ecosystem, closing out several long-standing feature requests and bringing advanced web performance techniques to the **Skylark** framework. The most significant addition to the web stack was native **ETag caching support**. By allowing Skylark applications to serve content with Entity Tags, the framework enabled browser-side caching that significantly reduced bandwidth and server load for static or slowly-changing resources. This was supported by the maturation of the `SkylarkPack` module system and the introduction of the `witz` numeric indices fix, ensuring that the template engine remained robust for complex data presentations.

TheIDE reached a new level of developer awareness with the implementation of **package and file error highlighting**. For the first time, the project browser began visually flagging erroneous files and packages directly in the tree, allowing developers to identify build issues at a glance without scrolling through long console logs. The environment's intelligence was further bolstered by improvements to the GDB interface and the introduction of the "Copy File Path" shortcut. The build system was also refined to handle project templates more efficiently, specifically resolving issues with release builds of packages containing only `.icpp` files.

Core library efficiency and networking were primary targets for refinement. The **LRUCache** (Least Recently Used Cache) system received a series of internal improvements to better handle high-frequency access patterns. A vital change was made to the networking core: `TcpSocket` was updated to **prefer IPv4 over IPv6** during connection attempts, resolving subtle latency issues in environments with incomplete IPv6 configurations. The library also gained the `findarg` utility and `JsonArray::operator bool`, simplifying the handling of dynamic JSON structures. On the Windows side, `Ws2_32.lib` was formally integrated into the `Core` package, further streamlining the transition of networking primitives out of the legacy `Web` bundle.

The database layer continued its march toward seamless schema management. The **MySQL driver** was upgraded to support `ALTER TABLE ADD COLUMN` with the same syntax as SQLite3, and it now correctly handles column type changes during automated database upgrades. `ODBC` gained the `EnumDSN()` method, simplifying the discovery of system-wide data sources. User interface ergonomics were also polished: `EditField` was refactored to support characters below code 32, and spin editors were unified under the new **WithSpin** template, providing a more consistent and maintainable implementation for numeric input fields.

## References
- [1] 7cfb7a29f — Skylark: ETag caching support introduced (cxl, 2012-10-07)
- [2] 37cbeeddb — theide: Error highlighting for files and packages (cxl, 2012-10-02)
- [3] 1a69d8f04 — Core: TcpSocket prefers IPv4 over IPv6 in Connect (cxl, 2012-10-23)
- [4] 9a63e4f29 — Core: LRUCache improvements and fixes (cxl, 2012-10-22)
- [5] 07f9521bc — CtrlLib: Spin edit fields refactored using WithSpin template (cxl, 2012-10-19)
- [6] 6d17a35d0 — MySQL: Schema upgrades support column type changes (cxl, 2012-10-02)
- [7] 6f0277ff3 — ODBC: EnumDSN() for data source discovery added (cxl, 2012-10-14)
- [8] a5a367feb — Core: findarg utility introduced (cxl, 2012-10-30)
- [9] ad0e3252c — Core: JsonArray gained operator bool (cxl, 2012-10-10)
- [10] 53ff87acf — Core: Ws2_32.lib formally added to Core package (cxl, 2012-10-12)
- [11] 439abb0af — theide: Fixed release builds for .icpp-only packages (cxl, 2012-10-08)
- [12] 76877dd83 — theide: Copy File Path feature added (cxl, 2011-11-07)
- [13] 8feb98e37 — CtrlLib: DisplayPopup uses Tip of master control (cxl, 2012-10-02)
- [14] 3a80e8c6b — CtrlLib: EditField supports characters < 32 (cxl, 2012-10-08)
- [15] 19af722d1 — Bazaar: LogCtrl made buffered for performance (kohait, 2012-10-03)
- [16] 6011e1051 — Bazaar: Multi-control selection in CtrlFinder/Pos (kohait, 2012-10-13)
- [17] 472418452 — Docking: Fixed closing of DockCont (cxl, 2012-10-07)
- [18] a9909cae1 — GridCtrl: SetRaw and GetRaw added (unodgs, 2012-10-24)
- [19] 70c16bba4 — MySQL: varchar limit increased to 65536 (cxl, 2012-10-26)
- [20] cb9a77c50 — CtrlCore: GetMaxSize fixed for multi-monitor setups (cxl, 2012-10-30)
