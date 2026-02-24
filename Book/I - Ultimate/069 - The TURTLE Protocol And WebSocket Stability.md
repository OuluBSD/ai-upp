# The TURTLE Protocol and WebSocket Stability (2014-01)
**Date Span:** 2014-01-01 to 2014-01-31

January 2014 was a month of foundational innovation in remote GUI technology and the beginning of a major push into modern web standards. The most ambitious development was the launch of the **TURTLE** project (Thin Ultimate Remote Terminal Layer Engine). Envisioned as a high-performance, VNC-like protocol tailored specifically for U++ applications, TURTLE aimed to provide a seamless way to project desktop-grade GUIs onto remote web browsers and thin clients. This effort was immediately supported by a refactor of the **WebSocket** implementation, ensuring that the real-time communication channel required for TURTLE was stable and performant. Simultaneously, **WebWord** reached a new level of functionality, demonstrating the framework's ability to host complex, interactive document editing entirely within a browser context.

The core library's foundational data handling and system diagnostics reached new levels of professional completeness. The **XMLParser** was enhanced with `TagElseSkip` and `LoopTag` methods, simplifying the logic required to parse complex or repetitive document structures. The library also gained **IsInf** and **IsFin** helpers for floating-point precision, and the **NOI18N** flag was introduced to allow developers to bypass internationalization for specialized performance-critical code. System diagnostics were bolstered by the introduction of **CppDemangle**, providing more readable stack traces and symbol information across both Windows and POSIX platforms. Networking was further hardened with the **Socket core now robustly handling EINTR signals**, a vital fix for long-running Linux services.

TheIDE and its professional build system achieved a major milestone with the stabilization of the **All Shared Build** strategy for both Linux and Windows (MSC). By allowing the entire framework and application code to be built as a suite of dynamic libraries, U++ enabled significantly faster development cycles and reduced executable sizes for complex modular systems. The environment's editor was also hardened to handle extreme edge cases, including a patch specifically addressing stability issues when encountering **150MB single lines** of text. The **IconDes** editor was refined to retain scrollbar positions during editing sessions, and the **PDB debugger** received critical fixes for modern Windows environments.

User interface ergonomics and specialized plugins continued their steady climb toward enterprise-grade robustness. **GridCtrl** and **DropGrid** were modernized with full UTF8 support and standardized NULL-to-empty-string conversions, ensuring a more consistent behavior in data-heavy forms. The **EditText** control gained a productivity feature: displaying the character count when a `maxlen` is defined. Professional polish was rounded out by the **Extended error dialog API**, and the **UnZip** plugin was upgraded to support **central directory** parsing, significantly improving its compatibility with modern archive formats. The month closed with a major documentation drive for the **ScatterCtrl** and **ScatterDraw** suites, ensuring that U++'s scientific visualization tools were well-supported by high-quality examples and tutorials.

## References
- [1] f8ea64c20 — Rainbow: TURTLE remote GUI engine development started (cxl, 2014-01-01)
- [2] 48e58356e — Core: WebSocket support refactored and stabilized (cxl, 2014-01-01)
- [3] d22290345 — ide: All Shared Build support stabilized for Linux (micio, 2014-01-24)
- [4] ff22c0910 — ide: All Shared Build support for MSC toolchain (micio, 2014-01-30)
- [5] 0b86d1aa6 — Core: XMLParser adds TagElseSkip and LoopTag (cxl, 2014-01-08)
- [6] 1ec0a007b — Core: CppDemangle and improved diagnostic Name() (cxl, 2014-01-10)
- [7] 8333c8540 — Core: Socket handling for EINTR signals (cxl, 2014-01-21)
- [8] 9b3c26fc3 — plugin/zip: UnZip adds central directory support (cxl, 2014-01-27)
- [9] 54b6f2c9a — Core: IsInf and IsFin floating-point helpers added (cxl, 2014-01-18)
- [10] c1dbe2d96 — Core: NOI18N flag introduced for specialized builds (cxl, 2014-01-13)
- [11] 62708828b — ide: Patched editor issue with 150MB lines (cxl, 2014-01-21)
- [12] f414c1d02 — ide: IconDes retains scrollbar position (cxl, 2014-01-31)
- [13] ca8ab8b10 — CtrlLib: EditText displays character count for maxlen (cxl, 2014-01-28)
- [14] 1a626d1ff — CtrlLib: Extended error dialog API introduced (cxl, 2014-01-17)
- [15] 050a7cd83 — GridCtrl: Encoding transitioned to UTF8 (koldo, 2014-01-10)
- [16] 6ac7c687b — ScatterCtrl: Legend table and properties improvements (koldo, 2014-01-11)
- [17] 2dc81d0dd — Core/Rpc: faultString XML escaping fix (cxl, 2014-01-15)
- [18] a47f464ac — CtrlCore: GtkWnd IsCompositedGui implementation (cxl, 2014-01-06)
- [19] d99e39135 — Sql: S_type now supports GetWidth (cxl, 2014-01-10)
- [20] 9766e8d1c — Core: LoadStream error handling refactored (cxl, 2014-01-27)
