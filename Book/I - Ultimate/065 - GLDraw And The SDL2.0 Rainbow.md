# GLDraw and the SDL2.0 Rainbow (2013-09)
**Date Span:** 2013-09-01 to 2013-09-30

September 2013 was a month of massive graphical expansion and the beginning of a major shift in U++'s hardware-accelerated strategy. The most significant technological addition was the launch of the **GLDraw** package. Envisioned as a high-level, cross-platform hardware acceleration layer, `GLDraw` provided U++ developers with a standardized way to utilize OpenGL for rendering complex UI and data visualizations. This effort was immediately supported by the `GLDrawDemo` reference and a suite of unittests, along with the introduction of **geometry shader and attribute support** in the `CoreGl` foundation. Simultaneously, the **Rainbow** project reached a new frontier with the development of the **SDL20** and **SDL20GL** backends, leveraging the power of the new SDL 2.0 library to provide native graphics acceleration across Windows and Linux.

The core library's foundational process management received a major upgrade with the expansion of **LocalProcess** and **AProcess**. New methods like `CloseRead`, `CloseWrite`, and the specialized `Start2`/`Read2` variants allowed U++ applications to **read stderr separately** from stdout, providing much-needed control for building complex CI/CD tools and system utilities. The networking suite was also refined: **TcpSocket::Listen** now defaults to a sensible `listen_count` of 5, and the **Skylark** web framework gained the `Http::GetPeerAddr()` method for reliable client IP identification.

The database layer, `SqlExp`, reached a new peak of expressive power. `SqlMassInsert` was upgraded to natively handle **ValueMap** parameters and the `SelectAll` directive, while new mass loading variants were added to simplify the ingestion of large datasets. The engine also gained support for **Insert...From...GroupBy...Having** constructs, and the canonical `S_*` table structures were enhanced with `Get()` and `GetColumnIds()` methods, further bridging the gap between static definitions and dynamic data access. The introduction of `Select(...).From()` provided a more intuitive alternative to the legacy `Get` syntax for complex queries.

TheIDE and its professional toolchain continued their steady climb toward perfection. The environment gained a vital productivity feature: **Alt+I now jumps to type definitions** if a method is not declared, significantly streamlining navigation in large templates or third-party libraries. The editor's intelligence was also bolstered by **SQL syntax highlighting for .ddl files** and improved handling of `wchar_t` and Java keywords. For developers targeting mobile or specialized Linux environments, the addition of the **DroidFonts** plugin and **FT_fontsys** provided a high-quality, professional typographical foundation. The month closed with critical fixes for **OfficeAutomation**, ensuring that Word and Excel processes are correctly terminated upon application shutdown.

## References
- [1] a452f3e8c — uppsrc: GLDraw high-level OpenGL acceleration introduced (cxl, 2013-09-16)
- [2] bc171f438 — Rainbow: Developing SDL20 and SDL20GL backends (cxl, 2013-09-29)
- [3] ef80e82ca — Core: LocalProcess adds separate stderr reading (Start2, Read2) (cxl, 2013-09-04)
- [4] 800a4a3e7 — Sql: SqlMassInsert adds ValueMap and SelectAll support (cxl, 2013-09-04)
- [5] 50de50a36 — Sql: SqlExp adds GroupBy and Having support for Insert-From (cxl, 2013-09-17)
- [6] 19ddf6cb8 — Sql: SqlExp adds Select(...).From() syntax variant (cxl, 2013-09-15)
- [7] 870906e7f — Skylark: Http::GetPeerAddr() method added (dolik, 2013-09-21)
- [8] 2b34b1d49 — ide: Alt+I jumps to type definition for undeclared methods (cxl, 2013-09-19)
- [9] 8e0354853 — ide: SQL syntax highlighting added for .ddl files (cxl, 2013-09-13)
- [10] bc9b4f37e — CoreGl: Added geometry shaders and attributes support (unodgs, 2013-09-08)
- [11] 22785cb30 — plugin: FT_fontsys and DroidFonts added (cxl, 2013-09-16)
- [12] 7cc9e93f5 — SysInfo: GetProcessCPUUsage and window-caption process lookup (koldo, 2013-09-14)
- [13] 26f2f20a7 — OfficeAutomation: Word/Excel process termination fixes (koldo, 2013-09-21)
- [14] a708185fc — Draw: SDraw adds PutImage with Color support (cxl, 2013-09-15)
- [15] e64ab168a — Core: LocalProcess adds CloseRead and CloseWrite (cxl, 2013-09-02)
- [16] 89372297a — Core: LocalProcess Start correctly closes duplicated handles (cxl, 2013-09-03)
- [17] 435df550a — Core: Scalar Date operations (Set/Get) improved (cxl, 2013-09-14)
- [18] a80115854 — AESStream: Integrated with Core/SSL (koldo, 2013-09-13)
- [19] 25ae39e18 — CtrlLib: FileSel UI and "Local disk" cosmetics (cxl, 2013-09-15)
- [20] 9456775d2 — Core: GetIniFile utility introduced (cxl, 2013-09-30)
