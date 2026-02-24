# The Multi-Tab IDE and the Cross-Platform Updater (2011-01)
**Date Span:** 2011-01-01 to 2011-01-31

January 2011 was a landmark month for TheIDE's user experience and the framework's deployment capabilities. The most profound shift was the replacement of the legacy "QuickTabs" system with the more powerful and flexible **TabBar** package. This transition enabled persistent, multi-tab editing with native support for stacking and grouping, fundamentally changing the daily workflow of U++ developers. TheIDE's environment was further refined with a reorganized configuration dialog, support for conditional breakpoints, and a visual fix for "auto-splitting" editor views during compilation—a common frustration in earlier versions.

The Bazaar saw an explosion of deployment-focused tooling, led by the **Updater** package. This ambitious project provided a cross-platform web installer and updater system, capable of handling complex installation tasks like shell embedding, desktop icon creation, and file associations across Windows and Linux. Supporting this was the **SysExec** package, which introduced specialized functions like `SysStartAdmin()` and `SysExecAdmin()` to reliably execute processes with elevated privileges (sudo/Administrator) while maintaining correct environment passing—a vital requirement for modern installers on Vista and newer operating systems.

Security and core architectural features also matured significantly. The `Xmlize` serialization engine was enhanced with `Set/GetUserData` support, providing a more flexible way to store transient state alongside persistent data. The `SmtpMail` package was completely refactored, adding native support for character encoding in headers and recipient/sender names, while `HttpClient` was hardened with improved error handling and content-size management. Database developers gained support for `CLOB` columns in `.sch` files and a powerful new `ToString()` method for `S_` structures, enabling direct logging and dumping of SQL record mappings.

User interface ergonomics continued to reach new professional benchmarks. `ArrayCtrl` with embedded widgets was optimized to handle up to 10,000 controls with zero lag, and the `Splitter` component was liberated to support arbitrary widths. `RichEdit` was expanded with a new `ShrinkOversizedObjects` capability, ensuring that large images or embedded content remained within the viewable bounds of the editor. Community contributions flourished with the arrival of `ValueCtrl`, `CtrlMover`, and `CtrlPos`, providing high-level tools for dynamic UI construction and live repositioning of controls.

## References
- [1] d40eac361 — ide: Replaced QuickTabs with TabBar; reorganized Environment dialog (unodgs, 2011-01-01)
- [2] a4922a2e4 — Bazaar: Updater cross-platform web installer introduced (micio, 2011-01-14)
- [3] 7389d721a — Bazaar: SysExec introduced for elevated process execution (micio, 2011-01-17)
- [4] 4cac6b88d — ide: Support for conditional breakpoints added (dolik, 2011-01-08)
- [5] 8b108a8ce — CtrlLib: ArrayCtrl optimized for 10,000+ embedded widgets (cxl, 2011-01-16)
- [6] 7a2fc73a6 — SmtpMail: Refactored with header encoding and recipient names (cxl, 2011-01-27)
- [7] 3b60c537c — Core: Set/GetUserData support added to Xmlize (cxl, 2011-01-10)
- [8] e5cb70971 — Sql: S_ structures gain ToString() for DUMP/LOG support (cxl, 2011-01-15)
- [9] 92ef6e45e — ide: Support for CLOB columns in .sch files (cxl, 2011-01-26)
- [10] 3d79b68a9 — RichText: ShrinkOversizedObjects for RichTextView (cxl, 2011-03-04)
- [11] 18081bdb3 — CtrlLib: Splitter support for arbitrary widths (unodgs, 2011-01-06)
- [12] 4495aebbd — Core: Thread ID support added (cxl, 2011-01-12)
- [13] a232bc348 — Bazaar: ProductVersion class for app versioning (micio, 2011-01-29)
- [14] 9bdc5b459 — Bazaar: SudoLib completed for Linux process elevation (micio, 2011-01-20)
- [15] 915db7484 — Bazaar: SysExecGui password frontend introduced (micio, 2011-01-22)
- [16] 110cbba53 — Bazaar: ValueCtrl and CtrlProp UI building tools (kohait, 2011-01-26)
- [17] 8e4833584 — Bazaar: CtrlMover and live CtrlPos positioning (kohait, 2011-01-28)
- [18] 33ff7807e — Web: Fixed non-blocking sockets in Linux (cxl, 2011-01-20)
- [19] a9c20e04e — Draw: RASTER_8ALPHA fixed for non-premultiplied PNGs (cxl, 2011-01-08)
- [20] b1b7f2389 — UI: Fixed menu issues with dark themes in Ubuntu (cxl, 2011-01-30)
