# The Docking Promotion and the Rainbow Horizon (2011-09)
**Date Span:** 2011-09-01 to 2011-09-30

September 2011 was a month of significant consolidation and expansion for U++, marked by the promotion of major components to the core library and the continued push into next-generation graphics. The most prominent event was the official move of the **Docking** package from the Bazaar to `uppsrc`. By becoming a canonical package, the sophisticated window docking and management system reached full framework maturity, providing all U++ developers with a professional-grade solution for complex, multi-pane application layouts. This transition was supported by exhaustive reference examples and internal optimizations, including a strategic relocation of `TimerProc` calls to ensure reliable docking behavior under heavy UI load.

The **Rainbow** project continued its rapid evolution, expanding its reach into hardware-accelerated backends and embedded displays. While the **WinGL** backend for Windows continued its steady maturation, the month saw the first functional tests for the **LinuxFb** (Linux Framebuffer) and the initial shot of the **SDLFb** backend. This diversification of the Rainbow architecture signaled U++'s intent to become a truly universal GUI toolkit, capable of running natively on everything from high-end workstations to lean, headless, or SDL-based embedded environments.

TheIDE received a specialized productivity boost with the introduction of native **Objective C support**, broadening the framework's appeal for cross-platform developers targeting Apple-adjacent technologies. The environment's graphical tools were also enhanced: the `.iml` (image list) designer gained the ability to import whole external `.iml` files and export collections directly to PNGs, streamlining asset management. On the build side, the **BRC** (Binary Resource Compiler) was refactored to generate byte arrays instead of string constants, bypassing long-standing size limitations in legacy compilers like Visual C++ 7.1.

Security and core architectural features saw a parallel push for robustness. The **Protect** package reached a milestone with its encryption macros becoming fully multi-thread safe and gaining locale-aware server-side logic. **PolyXML** was extended to support polymorphic maps and the `One` container, further simplifying the serialization of complex, dynamic object hierarchies. Core library ergonomics were improved with the addition of `String::GetCharCount` and fixes for UTF-8 character length calculations in the `Format` engine. Connectivity was also bolstered by the addition of `PUT` support in `HttpClient`, and the Bazaar welcomed the **VLCPlayer** package, providing a powerful alternative for multimedia applications.

## References
- [1] 20f3d667c — Docking: Moved from Bazaar to canonical uppsrc (cxl, 2011-09-02)
- [2] 24565d4ea — theide: Objective C support introduced (cxl, 2011-09-09)
- [3] 60c6d288d — Rainbow: LinuxFb and SDLFb initial tests (cxl/kohait, 2011-09-17)
- [4] 607455976 — Web: HttpClient now supports PUT (cxl, 2011-09-14)
- [5] fc925386e — Ide: .iml designer can import/export whole collections (cxl, 2011-09-22)
- [6] b1b7f2389 — UI: Improved support for dark visual themes (cxl, 2011-01-30)
- [7] 5e9255afb — BRC: Changed C generation to byte arrays for VC71 compatibility (rylek, 2011-09-26)
- [8] 1aac4ba6f — Core: String::GetCharCount introduced (cxl, 2011-09-07)
- [9] c6a5d9692 — Core: Format fixed for UTF-8 character lengths (cxl, 2011-09-09)
- [10] b5189c8cb — Bazaar: Protect macros made multi-thread safe (micio, 2011-02-08)
- [11] acdcf735e — Bazaar: PolyXML support for polymorphic maps (micio, 2011-09-08)
- [12] caf84c189 — Bazaar: VLCPlayer and test application introduced (sergeynikitin, 2011-09-19)
- [13] 96de5520e — Core: Tuple introduced (cxl, 2011-09-06)
- [14] 19999e0ee — Bazaar: Freetype2 library packaged for U++ (micio, 2011-09-17)
- [15] 97247371f — Bazaar: FTGL OpenGL font rendering library added (micio, 2011-09-17)
- [16] 85ad19b94 — CtrlCore: X11 support for _NET_WM_PID and _NET_WM_PING (cxl, 2011-09-04)
- [17] a3e19cc4f — Bazaar: PolyXML adds One support (micio, 2011-09-11)
- [18] 17ea71ed0 — Rainbow: Ongoing development of WinGL backend (unodgs, 2011-09-26)
- [19] 5987030f5 — plugin/tif updated to version 1.43 (cxl, 2011-09-26)
- [20] a2e636f3f — Scatter: Added Stroke(), Mark(), and data selection (koldo, 2011-09-28)
