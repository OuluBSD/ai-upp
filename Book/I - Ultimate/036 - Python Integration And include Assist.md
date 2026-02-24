# Python Integration and include Assist (2011-04)
**Date Span:** 2011-04-01 to 2011-04-30

April 2011 was a pivotal month for U++ scripting capabilities and the professional refinement of TheIDE's intelligence. The framework's interest in external scripting culminated in a major expansion of the **Py** package (formerly "Python") in the Bazaar. This effort, aimed at deep integration via `Boost.Python`, introduced the `BOOSTPY` flag and a suite of converters for fundamental U++ types like `String`, `Value`, and `ValueArray`. This allowed U++ applications to export their internal object models to a hosted Python environment, supported by the `PyConsoleCtrl` which gained history functionality, error redirection to the UI, and the ability to handle both single statements and complex script blocks.

TheIDE received a high-impact productivity boost with the introduction of **#include Assist**. This new feature extended the environment's autocomplete capabilities to file paths within preprocessor directives, supporting both system and local headers, including relative parent directory (`..`) navigation. This addition significantly streamlined the management of complex project structures. The build system was also hardened for modern toolchains, with critical fixes landed to ensure compatibility with GCC 4.6 and improvements to the GCC builder's library ordering logic.

Core library ergonomics were further refined with a consistent push for reference-returning APIs. Methods such as `Set`, `Insert`, and `InsertPick` in `Array` and `Vector` were updated to return `T&`, continuing the previous month's trend of simplifying object initialization. `ValueMap` received substantial internal improvements, and `CParser` was extended with a specialized `ReadInt(min, max)` helper. The database layer also matured, with `SqlExp` gaining parenthesis support for union results (enabling sorting of combined datasets) and `MySql` adding `GetTransactionLevel` support.

User interface components reached new levels of professional polish and cross-platform resilience. `TabCtrl` was enhanced to support QTF (rich text) in tab labels, and `LineEdit` gained a "ShowSpaces" mode for better visibility during whitespace-sensitive editing. The `TrayIcon` system was hardened against Windows Explorer crashes by utilizing the `TaskbarCreated` message for automatic recreation. At the graphical level, `plugin/png` received a critical fix for `tRNS` transparency interpretation, and `PdfDraw` was refined to resolve clashing font declarations when used alongside OLE headers. Community outreach also hit a milestone with the launch of the German translation for the official website, further expanding the framework's accessibility.

## References
- [1] d598ffeda — Bazaar: Python package renamed to Py; PyConsoleCtrl improved (kohait, 2011-04-07)
- [2] 559e82385 — Bazaar: Py prepared for Boost.Python with BOOSTPY flag (kohait, 2011-04-11)
- [3] bf8c5d3e5 — theide: #include Assist introduced (cxl, 2011-04-25)
- [4] 5a44fdd2d — Core: Array/Vector Set/Insert methods return T& (cxl, 2011-04-16)
- [5] 8210eb5e0 — SqlExp: Union results wrapped in parenthesis for sorting (cxl, 2011-04-05)
- [6] 14afc45a4 — CtrlLib: TabCtrl supports QTF in tab labels (cxl, 2011-04-14)
- [7] 20e63d435 — CtrlLib: LineEdit::ShowSpaces mode added (cxl, 2011-04-17)
- [8] b3c666dec — CtrlLib: TrayBar recreates after explorer.exe crash (cxl, 2011-04-27)
- [9] cc4ee3693 — MySql: GetTransactionLevel introduced (cxl, 2011-04-19)
- [10] b6aa6359e — XmlRpc: Extended structure compatibility for containers (cxl, 2011-04-14)
- [11] 58986b5fb — Core: Fixes for GCC 4.6 compatibility (cxl, 2011-04-10)
- [12] 9004bbc0e — plugin/png: Fixed transparency (tRNS) interpretation (rylek, 2011-04-12)
- [13] 7386be99a — Sql: 'binary' flag added to SqlColumnInfo (rylek, 2011-04-03)
- [14] b6ba76f0d — Core: CParser::ReadInt(min, max) added (cxl, 2011-04-22)
- [15] dde3e7519 — Core: Documentation for INITBLOCK/EXITBLOCK caveats (kohait, 2011-04-18)
- [16] bcccf35f9 — uppweb: German index page and translations launched (kohait, 2011-04-26)
- [17] 81add0b4f — examples: SDLexample fixed for WinMain and linkage (kohait, 2011-04-26)
- [18] 4334cd332 — bazaar: Urr adds SourceIp and SourcePort (kohait, 2011-04-26)
- [19] dfc907b3b — theide: Include assist adds '..' support (cxl, 2011-04-29)
- [20] 0c4c0cf2f — theide: GCC builder puts libraries at end of list (cxl, 2011-04-04)
