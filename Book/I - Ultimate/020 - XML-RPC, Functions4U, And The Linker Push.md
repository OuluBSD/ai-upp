# XML-RPC, Functions4U, and the Linker Push (2009-12)
**Date Span:** 2009-12-01 to 2009-12-31

The final month of 2009 was defined by a surge in high-level utility packages and a significant push into binary optimization. The arrival of the `XmlRpc` package provided U++ developers with a standardized, easy-to-use implementation for remote procedure calls over XML, complete with comprehensive client and server examples. This was a vital addition for building distributed systems and interoperating with web services of the era.

Parallel to this, the community-driven "4U" initiative exploded in scope with the introduction of `Functions4U` and `Controls4U` in the Bazaar. These packages aggregated dozens of practical helpers—from `RemoveAccents` and `BSDiff/BSPatch` binary diffing to specialized UI controls like `StaticClock` (with its new resource-efficient "auto" mode) and `Meter`. The `SysInfo` package also reached a new milestone, adding desktop recording and extending its low-level keyboard and mouse simulation functions to Linux, achieving parity with the existing Win32 features.

Architecturally, the `Core` library underwent a major cleanup. `LanguageInfo` was completely refactored to be more robust and maintainable, resolving long-standing issues with system registry interaction and cross-platform locale detection. The `PdfDraw` backend was improved to ensure better compatibility with various PDF readers by avoiding zero character codes in text output. On the Windows side, a specialized module for Win32 service protocol support was added, simplifying the creation of background system services.

TheIDE and the build system received a powerful optimization: support for GCC `--gc-sections`. By enabling the linker to discard unused functions and data, U++ applications achieved significantly smaller binary sizes. This was supported by new "link options" fields in the build method configuration, providing developers with finer control over the final linking phase. The environment also became more resilient, with `AutoSetup` now recognizing the Win32 SDK 7.0 and `EditField` context menus becoming fully customizable, allowing developers to extend standard text fields with application-specific actions.

## References
- [1] 6dbea1df2 — New XmlRpc package (cxl, 2009-12-20)
- [2] bf3f4990b — reference: XmlRpcCall, XmlRpcServer, XmlRpcClient (cxl, 2009-12-20)
- [3] 793753cb6 — Functions4U: New package in Bazaar (koldo, 2009-12-08)
- [4] ccce8e4a9 — Controls4U: New package in Bazaar (koldo, 2009-12-08)
- [5] 8ea1022dd — SysInfo: Added desktop recording (koldo, 2009-12-08)
- [6] 059e0af94 — SysInfo: Keyboard and Mouse functions supported in Linux (koldo, 2009-12-25)
- [7] 38a45656b — ide: Developing gc-sections support (cxl, 2009-12-30)
- [8] 47d2ac92a — ide: build method link options support (cxl, 2009-12-30)
- [9] ee3d0b35a — Core: LanguageInfo completely refactored (cxl, 2009-12-13)
- [10] 57e849290 — PdfDraw: Fixed PDF reader compatibility issues (cxl, 2009-12-22)
- [11] 23bfb66bd — A simple Win32 service protocol support module (rylek, 2009-12-27)
- [12] e25a6be46 — CtrlLib: EditField context menu exposed and overridable (cxl, 2009-12-03)
- [13] bc0abc6d1 — Controls4U: New "auto" mode in StaticClock (koldo, 2009-12-23)
- [14] a6946dbed — Functions4U: Added BSDiff/BSPatch (koldo, 2009-12-29)
- [15] 2b606742e — CtrlLib: Splitter::Remove and enhancements (cxl, 2009-12-30)
- [16] 643bc28dd — TheIDE: Autosetup recognizes Win32 SDK 7.0 (cxl, 2009-12-20)
- [17] 52e17de39 — MSSQL: BIGINT added to OleDB schema definitions (rylek, 2009-12-29)
- [18] b2305363b — Sql: PGSQL SqlBinary fix (cxl, 2009-12-08)
- [19] 71c794ce3 — Core: ValueMap - added String key variants (cxl, 2009-12-21)
- [20] 2047a968e — Functions4U: Added RemoveAccents() (koldo, 2009-12-25)
