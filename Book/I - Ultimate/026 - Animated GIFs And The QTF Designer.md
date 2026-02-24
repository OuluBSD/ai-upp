# Animated GIFs and the QTF Designer (2010-06)
**Date Span:** 2010-06-01 to 2010-06-30

June 2010 was a month of creative and functional expansion for U++, introducing powerful new media capabilities and sophisticated internal tooling. The framework's graphical subsystem reached a delightful milestone with the introduction of native **Animated GIF support**. This was not merely about static frames; the framework gained a comprehensive multi-page raster architecture, allowing for the handling of complex image sequences with support for subimage rectangles and various frame disposal methods (including "Restore to previous"). This was immediately demonstrated in the `RasterMultiPage` example, which showcased the framework's ability to handle both GIF and multi-page TIFF formats with ease.

TheIDE received a high-impact productivity tool: the **QTF Designer**. Quick Text Format (QTF) has always been the framework's internal standard for rich text, but its manual creation was often tedious. The new designer provided a visual environment for crafting rich text resources, making it significantly easier to create documentation, help topics, and sophisticated UI labels. This period also saw TheIDE's package selector become more intelligent, sorting packages by "directory closeness" to the main project, effectively surfacing relevant subpackages and nests first.

Hardware and system introspection capabilities reached a professional peak with major updates to the `SysInfo` package. U++ applications could now monitor CPU temperature and battery status via ACPI on both Linux and Windows. Network diagnostics were also bolstered with standardized MAC address retrieval. To resolve long-standing include conflicts between U++ and system headers (particularly Xlib), a significant effort was made to isolate and fix `.h` interferences in `SysInfo`, `Functions4U`, and `Controls4U`.

The core library continued its steady march toward completeness. `bool` was formally registered as a `Value` type, and strings gained the `ReverseFind` method. Networking saw improvements with `FIELD_TYPE_TIMESTAMP` support in MySQL and the removal of unimplemented `Socket::Reuse` methods. The `TabBar` package underwent a rigorous consolidation; the interface was cleaned up, and the experimental `TabBarCtrl` was removed in favor of a more polished, standard-compliant implementation. Community outreach was also at an all-time high, with the official website launching a comprehensive Tutoring manual and a new series of video tutorials, alongside the addition of Simplified Chinese translations.

## References
- [1] 022f37593 — Draw, plugin/gif: Animated GIF support introduced (cxl, 2010-06-18)
- [2] e745d2714 — theide: QTF designer launched (cxl, 2010-06-08)
- [3] e7d9ac73d — reference: RasterMultiPage example for GIF/TIFF sequences (koldo, 2010-06-19)
- [4] 14c56b51f — SysInfo: Added CPU temperature and ACPI battery monitoring (koldo, 2010-06-04)
- [5] c63e82274 — SysInfo: MAC address retrieval added (koldo, 2010-06-08)
- [6] e5810875c — theide: Package sorting by directory closeness (cxl, 2010-06-15)
- [7] 66c451a93 — PdfDraw: Basic support for fill patterns in PDF (rylek, 2010-06-08)
- [8] 5de2ea2e3 — Core: bool Value type registration (cxl, 2010-06-03)
- [9] 296e44e50 — Core: [W]String::ReverseFind introduced (cxl, 2010-06-03)
- [10] c567bf41c — Core: int64 support in Format (cxl, 2010-06-20)
- [11] aeb9bbfae — TabBar: consolidated interface and sorting (mrjt, 2010-06-25)
- [12] 932359527 — SysInfo: Resolved system header interferences (koldo, 2010-06-13)
- [13] ce198c8ba — Core: New DeXml variants for strings with nulls (cxl, 2010-06-14)
- [14] f5e294410 — theide: Console apps print exit code on completion (cxl, 2010-06-28)
- [15] 128b86d7b — MySql: FIELD_TYPE_TIMESTAMP support (cxl, 2010-06-24)
- [16] 6a46efde2 — uppweb: Tutoring manual and video tutorials launched (koldo, 2010-06-24)
- [17] c857d01aa — uppweb: Simplified Chinese translation added by Bonami (koldo, 2010-06-09)
- [18] 06b14db16 — Functions4U: GatherTpp for documentation automation (koldo, 2010-06-10)
- [19] eeb6f4629 — Splitter: WhenSplitFinish callback added (cxl, 2010-06-02)
- [20] 758519c26 — CtrlLib: ToolButton::Label improvements (cxl, 2010-06-13)
