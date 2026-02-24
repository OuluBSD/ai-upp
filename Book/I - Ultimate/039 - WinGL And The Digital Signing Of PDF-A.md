# WinGL and the Digital Signing of PDF/A (2011-07)
**Date Span:** 2011-07-01 to 2011-07-31

July 2011 was a month of deep graphics integration and the arrival of professional document security features within the U++ framework. The **Rainbow** project reached a significant milestone with the parallel development of two major backends: **WinGL** and **LinuxFb**. Led by the introduction of `WinGL`, U++ gained a dedicated OpenGL-based GUI backend for Windows, enabling hardware-accelerated UI rendering. Simultaneously, the `LinuxFb` backend achieved its "first working stage," supporting video, mouse, and keyboard input directly on the Linux framebuffer, including advanced features like virtual terminal (VT) console switching and `MEDIUMRAW` keyboard mode. This dual push positioned U++ as a uniquely versatile toolkit for both high-end accelerated desktops and lean embedded systems.

Professional document standards took a leap forward with the introduction of minimalistic **PDF/A support** and native **digital signing**. By leveraging the new `plugin/wincert` for Win32 certificate manipulation, the framework gained the ability to produce archive-standard PDF files with embedded digital signatures—a vital requirement for legal, financial, and government-grade software. This was supported by refinements to the `PdfDraw` engine and the core `Report` system, which now correctly handles table cell shading and improved span imports from RTF.

Core library ergonomics were further refined for modern C++ standards. The `empty()` method was added to all containers for STL compatibility, and the `Complex` number type was made natively `Value` compatible, supporting polyequal logic and easy conversion from standard numeric types. Architectural hardening was also a focus: `Thread` methods were updated with enhanced thread-ID support, and binary serialization achieved full cross-compiler compatibility between GCC and MSC by resolving `dword` definition mismatches. The `Painter` package was also hardened with support for an `Invert` attribute in subpixel mode and specialized `NaN` (Not-a-Number) handling to prevent rendering crashes.

The user interface toolkit continued its steady climb toward professional excellence. `GridCtrl` was made "OpenGL friendly" to support the new WinGL backend, and `FileSel` received a major performance tuning for its icon loading logic across both Win32 and X11. TheIDE became more expressive with the topic editor now supporting language-aware code insertion and the "Insert Color" tool gaining support for QTF-specific color codes. The Bazaar also expanded its reach with the addition of the **kissfft** plugin for fast Fourier transforms and the continued maturation of the **Py** (Python) integration, which now supports exporting a wide range of standard edit fields to hosted scripts.

## References
- [1] eb343f8eb — Rainbow: WinGL hardware-accelerated backend started (unodgs, 2011-07-01)
- [2] d0fab0c38 — Rainbow: LinuxFb first working stage with video/mouse/KB (kohait, 2011-07-20)
- [3] e5a6d9d07 — PDF: Minimalistic PDF/A support and digital signing (rylek, 2011-07-27)
- [4] 5332de033 — Core: Value-compatible Complex number type (cxl, 2011-07-06)
- [5] af7585109 — Core: empty() method for STL compatibility in containers (cxl, 2011-07-21)
- [6] f97c82e1d — Painter: Added Invert attribute and subpixel clipping fixes (cxl, 2011-07-03)
- [7] 82ffff32a — Core: Fixed binary serialization compatibility GCC vs MSC (cxl, 2011-07-10)
- [8] ee16211d3 — Core: IsNaN introduced; Painter ASSERTs for NaN values (cxl, 2011-07-10)
- [9] ff25c6a32 — Report: QtfReport introduced (cxl, 2011-07-11)
- [10] 6638dd419 — Oracle: 64-bit integer support in OCI8 (rylek, 2011-07-12)
- [11] 4d7308e99 — Oracle: OracleBlob/Clob upgraded to 64-bit handles (rylek, 2011-07-13)
- [12] 8beb8e9cc — Rainbow: LinuxFb adds VT console switch support (kohait, 2011-07-22)
- [13] 1d804fc01 — GridCtrl: Made grid more OpenGL friendly (unodgs, 2011-07-13)
- [14] 0fc3c5291 — CtrlCore: Improved end-session behavior in Win32 (cxl, 2011-07-23)
- [15] 624c5a8f8 — theide: InsertColor now supports QTF colors (cxl, 2011-07-29)
- [16] a4ef4118d — Bazaar: plugin kissfft (Fast Fourier Transform) added (kohait, 2011-07-31)
- [17] 782160493 — Bazaar: BoostPyTest exports standard edit fields (kohait, 2011-07-10)
- [18] 3cc0b6948 — CtrlLib: POSIX default printer reads size from CUPS (cxl, 2011-07-02)
- [19] 775e2ba3a — examples: UWord detects and handles .rtf extension (cxl, 2011-07-06)
- [20] 844ad8d12 — Rainbow: Framebuffer repaint optimization added (kohait, 2011-07-22)
