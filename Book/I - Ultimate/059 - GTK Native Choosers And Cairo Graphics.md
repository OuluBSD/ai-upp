# GTK Native Choosers and Cairo Graphics (2013-03)
**Date Span:** 2013-03-01 to 2013-03-31

March 2013 was a month of significant graphical modernization and professional UI refinement, particularly for U++ applications running on Linux. The framework reached a major milestone in platform parity with the introduction of **native GTK+ file chooser support** in `FileSel`. By enabling the use of the host environment's standard dialogs, U++ applications achieved a more seamless visual and functional integration with modern Linux desktops. This was accompanied by the implementation of **GLCtrl for GTK**, providing a standardized way to host OpenGL content within Gtk-based windows, and exhaustive fixes for multi-monitor workarea calculations.

The **Rainbow** project's internal drawing capabilities were substantially bolstered with the introduction of **Cairo-based drawing operations**. `CtrlCore` gained support for cairo line dashing, `DrawPolyPolyPolygonOp`, and `DrawArcOp`, bringing professional-grade vector graphics features to the framework's software backends. This graphical push was supported by a comprehensive documentation drive for the newly introduced **SortedIndex** and **SortedMap** family, ensuring that the logarithmic-performance containers introduced in the previous month were well-understood by the developer base.

Core library ergonomics and stability continued their steady climb. The **INI parser** was refactored for better resilience, and the `decode` function was added to the core library—a vital utility for multi-branch value mapping. The library also gained 3- and 4-parameter versions of **min/max**, and the `Color` class was hardened against accidental boolean-to-color conversions through a specialized private constructor. Networking saw refinements in `Core/Rpc` with improved Fault reporting and the introduction of the `ContentType` property for better HTTP compliance.

TheIDE and its scripting engine, **ESC**, received several high-impact productivity upgrades. The macro system was enhanced to support more complex logic, and the environment achieved better consistency in MSC command-line generation. The **QTF Designer** was refined to better handle current selections, and `RichEdit` gained the ability to use the standard **I-Beam cursor** for text selection. Professional polish continued with the addition of **WhenPasteFilter** to `EditField` and the expansion of **Report** printing hints, while the Bazaar's **DXF** package added support for the `Point` entity, further rounding out its CAD capabilities.

## References
- [1] 390c79fdd — CtrlLib: FileSel now uses native GTK+ file chooser (cxl, 2013-03-05)
- [2] 6b5f78451 — GLCtrl: Implemented for GTK backend (cxl, 2013-03-24)
- [3] 9930ecef5 — CtrlCore: Added Cairo line dash support (cxl, 2013-03-12)
- [4] 9c7f18038 — CtrlCore: Added Cairo DrawArcOp implementation (cxl, 2013-03-12)
- [5] 4e2d7f803 — CtrlCore: Added Cairo DrawPolyPolyPolygonOp (cxl, 2013-03-12)
- [6] bcac944cb — Core: decode function introduced (cxl, 2013-03-23)
- [7] e8cf533ef — Core: min/max defined for 3 and 4 parameters (cxl, 2013-03-23)
- [8] aaf108264 — Core: SortedIndex documentation completed (cxl, 2013-03-04)
- [9] d2d598432 — Core: Color protected against boolean conversion traps (cxl, 2013-03-29)
- [10] 3fb687ecc — Core/Rpc: Improved fault reporting and ContentType support (cxl, 2013-03-14)
- [11] 4021b13b7 — ide: ESC macro engine enhancements (cxl, 2013-03-14)
- [12] 3691238c7 — CtrlLib: EditField adds ShowSpaces and WhenHighlight (cxl, 2013-03-28)
- [13] 8f66e2d09 — RichEdit: Switches to Image::IBeam for text cursor (cxl, 2013-03-23)
- [14] c369455fa — CtrlLib: Refined default MenuBar style (cxl, 2013-03-06)
- [15] 545fafa5d — Sql: SqlSelect::AsTable fixed for PGSQL (cxl, 2013-03-11)
- [16] d234ec9a0 — SqlCtrl: SqlConsole now utilizes FileSel (cxl, 2013-03-22)
- [17] 0241ad925 — RichText: HtmlObjectSaver for customized image output (rylek, 2013-03-25)
- [18] 2a43d9911 — Report: Added printing hints support (cxl, 2013-03-30)
- [19] 129581b75 — Bazaar: DXF package adds Point entity (micio, 2012-09-05)
- [20] 900f3e080 — CtrlLib: ArrayCtrl::Clear fixed for correct WhenSel timing (cxl, 2013-03-08)
