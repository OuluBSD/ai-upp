# GUI Locking and the NewDraw Nest (2009-05)
**Date Span:** 2009-05-01 to 2009-05-31

May 2009 witnessed a fundamental breakthrough in U++ multi-threading: the introduction of standardized GUI locking. By implementing `Ctrl::Lock` and `Ctrl::Call`, the framework finally provided a robust, cross-platform mechanism for safely interacting with the user interface from non-main threads. This allowed for background window creation, deletion, and dedicated event loops in secondary threads—a feature that significantly improved the responsiveness of complex, data-heavy applications. To support this new paradigm, `GuiSleep` and other core timing functions were made thread-aware, and a suite of `GuiMtTest` cases was developed to ensure stability across Win32 and X11 backends.

Simultaneously, an intensive graphical refactor was underway in the "newdraw" development nest. This effort focused on unifying the core `Draw` and `SystemDraw` implementations, merging `PLATFORM_XFT` into the main `PLATFORM_X11` path, and optimizing font metrics. The `Painter` package also matured, with serialization fixes for `RichValue` and `ValueArray`, and the integration of `Painting` tests into the core `Drawing` test suite. The framework's visual ergonomics were further enhanced by the addition of the high-quality `Skulpture` theme to the Bazaar, alongside improvements to the GTK Chameleon backend for dark-mode support.

TheIDE continued its march toward developer "omniscience." The `Ctrl+Click` shortcut was introduced, allowing users to jump directly from a symbol usage to its definition—a feature that quickly became indispensable for navigating large codebases. The "Find in files" tool was upgraded with a dedicated output console, preventing search results from cluttering the main build log. Performance was also a focus: a new `PackagePath` cache was implemented to resolve a persistent "key lag" issue, making the editor feel significantly more responsive during heavy indexing.

The database layer saw unified scripting support with the arrival of `SqlPerformScript`, designed to replace platform-specific alternatives like `ODBCPerformScript`. `MSSQL` gained support for `SQLDEFAULT`, and `SqlBinary` was refactored to use the `SqlCompile` engine, finally bringing it into full compatibility with PostgreSQL. Community contributions flourished with the addition of the `GoogleTranslator` library and significant documentation updates for the `SysInfo` and `OfficeAutomation` packages.

## References
- [1] ec1ca78c6 — CtrlCore: Ctrl::Lock MT in Win32 (cxl, 2009-05-16)
- [2] 5e99fa920 — Ctrl::Call in X11 and EventLoops in non-main threads (cxl, 2009-05-17)
- [3] f037798ca — Win32: Ctrl::Call and non-main thread window support (cxl, 2009-05-17)
- [4] c07c50dca — theide: Ctrl+Click onto symbol jumps to definition (cxl, 2009-05-15)
- [5] 2c6026d21 — theide: Find in files now has separate output console (cxl, 2009-05-30)
- [6] 7e4c2e8a3 — PackagePath cache to resolve theide key lag (cxl, 2009-05-12)
- [7] a69fbf7e0 — newdraw nest for development of new draw (cxl, 2009-05-10)
- [8] d9bad030b — SqlPerformScript unified database scripting (cxl, 2009-05-25)
- [9] c7137b20e — SqlBinary refactored for PGSQL compatibility (cxl, 2009-05-25)
- [10] 025374340 — Skulpture theme added to Bazaar (cbpporter, 2009-05-19)
- [11] 25216d97c — GoogleTranslator library + examples (tojocky, 2009-05-29)
- [12] f3b181774 — SysInfo: Documentation and hardware detection improvements (koldo, 2009-05-28)
- [13] d40afb3fd — Refactored CodeNavigator (cxl, 2009-05-01)
- [14] ea4448d99 — Serialization fixes for Drawing, Painting, and RichValue (cxl, 2009-05-02)
- [15] 16bf9cc58 — PLATFORM_XFT merged into PLATFORM_X11 (cxl, 2009-05-07)
- [16] 74c780126 — Support for MSC8 (Visual C++ 2005) (koldo, 2009-05-07)
- [17] 82dd4aa45 — Array/Vector: InsertPick added (cxl, 2009-05-10)
- [18] d090db097 — theide: Highlighting of .sch files (cxl, 2009-05-22)
- [19] 11f22c400 — theide: Fixed NOBLITZ problem (cxl, 2009-05-28)
- [20] ef636b41d — LineEdit fix for CJK in Ubuntu (cxl, 2009-05-30)
