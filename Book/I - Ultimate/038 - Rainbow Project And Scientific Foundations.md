# Rainbow Project and Scientific Foundations (2011-06)
**Date Span:** 2011-06-01 to 2011-06-30

June 2011 was a month of profound architectural ambition and the expansion of U++ into the realms of high-performance science and engineering. The most significant development was the formal launch of the **Rainbow** project. Envisioned as a unified, platform-agnostic GUI backend architecture, Rainbow represented a fundamental rethink of how U++ interacts with graphical displays. The intensive initial push focused on the creation of a dedicated **Framebuffer (FB) backend**, enabling U++ applications to render directly to a raw memory buffer or a Linux framebuffer device. This effort, which saw dozens of commits throughout the month, laid the groundwork for running full U++ GUI applications on embedded systems and specialized hardware without the overhead of X11 or Win32.

Simultaneously, the framework's analytical capabilities were significantly bolstered by the arrival of two powerhouse libraries in the Bazaar: **Eigen** and **OpenCV**. The integration of Eigen provided U++ developers with high-performance C++ templates for linear algebra, matrices, and related mathematical algorithms. Parallel to this, the OpenCV (Open Source Computer Vision Library) package brought advanced image processing and computer vision capabilities to the ecosystem. Together, these additions transformed U++ into a viable platform for scientific computing, robotics, and industrial automation.

TheIDE continued its evolution toward supporting a broader range of professional toolchains. A major addition was the **Open Watcom** builder support, providing a bridge to a classic and powerful compiler suite. The environment's intelligence was also refined: the topic editor began inserting code items in the language of the current topic, and the "include assist" feature was improved to better handle complex path resolutions. On the Windows side, the framework achieved better integration with modern OS features, adding `IsWin7` detection and switching to backpainted rendering by default on Windows 7 and newer to ensure a flicker-free user experience.

Core architectural improvements and database refinements remained a steady constant. The Oracle backend received a significant upgrade with the implementation of `OracleClob`, providing a robust interface for reading and writing Character Large Objects in both single-character and UTF8 modes. `SqlExp` was tuned to wrap union results in parentheses, allowing for much-needed sorting of combined datasets. Networking was also hardened, with `HttpClient` adding support for HTTP 100 "Continue" replies and critical fixes for default HTTPS port handling. User interface components like `GridCtrl` reached new levels of maturity, gaining callbacks for column reordering and specialized header font controls.

## References
- [1] 42f88f553 — CtrlCore: Initial development of Rainbow unified GUI architecture (cxl, 2011-06-29)
- [2] 60c6d288d — Rainbow: Framebuffer (FB) backend implementation (cxl, 2011-06-17)
- [3] 0d02c213a — Eigen: Matrix algebra and math algorithms introduced (koldo, 2011-06-01)
- [4] 7e269d5f8 — OpenCV: Computer vision presentation and package (koldo, 2011-06-01)
- [5] f67f94001 — theide: Open Watcom builder support added (cxl, 2011-06-12)
- [6] 6e4d2747e — Oracle: OracleClob implemented for CLOB read/write (rylek, 2011-06-08)
- [7] 86a0e1b94 — CtrlCore: Backpainted by default in Windows 7+ (cxl, 2011-06-08)
- [8] b2251dfe7 — theide: Topic editor context-aware code insertion (cxl, 2011-06-02)
- [9] 0939abd35 — Web: HttpClient supports status code 100 'Continue' (rylek, 2011-06-18)
- [10] 66bf215fd — GridCtrl: WhenChangeOrder callback added (unodgs, 2011-06-26)
- [11] 72f3bddde — GridCtrl: SetHeaderFont and header font in popups (unodgs, 2011-06-26)
- [12] 9ba68a4b1 — Draw: AdjustColors fixed for premultiplied alpha (cxl, 2011-06-13)
- [13] 731332873 — CtrlCore: Added bool support to Ref (cxl, 2011-06-18)
- [14] 33a280f1f — Web: Default HTTPS port fix and SSL leak workarounds (rylek, 2011-06-23)
- [15] 96c2c70e1 — Bazaar: BoostPyTest works under Linux with more controls (kohait, 2011-06-22)
- [16] 817f34a1d — Oracle: Improved typeless NULL handling in Sql::GetColumn (rylek, 2011-06-12)
- [17] 62de0eadd — CtrlLib: Fixed crash in multiselect when no cursor exists (cxl, 2011-06-16)
- [18] d5a64e34e — theide: Improved include assist (cxl, 2011-06-25)
- [19] 0334e338f — uppbox: MakeInstall4 support for Rainbow (cxl, 2011-06-19)
- [20] 90b98ca69 — Draw: AddRefreshRect extracted from X11 Invalidate (cxl, 2011-06-25)
