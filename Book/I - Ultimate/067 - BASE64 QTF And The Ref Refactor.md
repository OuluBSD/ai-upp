# BASE64 QTF and the Ref Refactor (2013-11)
**Date Span:** 2013-11-01 to 2013-11-30

November 2013 was a month of deep data structure modernization and professional toolset expansion for the U++ ecosystem. The most significant architectural shift occurred within the **serialization and document layers**. The **QTF (Quick Text Format)** engine was upgraded to use **BASE64 encoding for embedded objects**, a move that dramatically improved the portability and robustness of rich text files containing images or binary data. Parallel to this, the core **Ref** system was fundamentally refactored. This overhaul provided a more unified and type-safe way to handle generic references across the framework, expanding support to **ValueMap**, **ValueArray**, **Complex**, and the **Geom.h** types. This effort was seamlessly integrated into the database layer, where the canonical **S_ table structures** were updated to leverage the new reference model for more efficient data binding.

TheIDE and its developer intelligence reached new heights of visual feedback. The environment introduced native **JsonView and XmlView** improvements, providing a hierarchical and interactive way to inspect complex data structures directly within the project workbench. **Assist++** was extended to support **.iml (image list) files**, bringing intelligent autocompletion and navigation to the framework's primary graphical resource format. The **Layout Designer** also became more flexible, with its code generator now capable of placing double-quotes around elements to ensure compatibility with specialized string-handling conventions.

Core library efficiency and system-level control were further refined. The **XML indentation logic** was refactored to produce more human-readable outputs, and the **INI system** was enhanced with `DefaultIniFileContent` and `CurrentIniFileContent` for better configuration management. Core time handling was bolstered with the `AlwaysTime` and `DayEnd` helpers, immediately utilized by the `EditDateTime` control to provide better end-of-day boundary handling. For high-performance logging and diagnostics, the `Debug.cpp` core was hardened for **AMD64 CPUs**, and `TextCtrl::Load` underwent a series of aggressive optimizations to minimize load times for large text buffers.

User interface ergonomics continued their steady march toward professional excellence. `RichTextView` was upgraded with **triple-click paragraph selection**, aligning its behavior with standard word processors, and `RichEdit` gained support for **Null color cell backgrounds**, enabling more sophisticated table designs. The **PdfDraw** engine achieved a significant reduction in file size by implementing **compression for repeated images**, a vital optimization for document-heavy applications. The month closed with refinements to the **GTK backend**, specifically addressing key description issues for non-alphanumeric characters, and the introduction of the **EditDateDlg** for standardized date entry.

## References
- [1] 623d1d1a0 — RichText: QTF now utilizes BASE64 encoding for objects (cxl, 2013-11-23)
- [2] c789b048d — Core: Ref system refactored; SQL S_* structures updated (cxl, 2013-11-03)
- [3] a555aaf8e — ide: JsonView and XmlView visualization improvements (cxl, 2013-11-11)
- [4] db8112040 — ide: Assist++ adds support for .iml image list files (cxl, 2013-11-20)
- [5] f821f175b — Core: XML tag indentation refactored for readability (cxl, 2013-11-14)
- [6] 5ae3e47fe — Draw: PdfDraw implements compression for repeated images (cxl, 2013-11-21)
- [7] 93d544d91 — CtrlLib: RichTextView adds triple-click paragraph selection (cxl, 2013-11-06)
- [8] bd660400c — RichEdit: Support for Null color in table cell backgrounds (cxl, 2013-11-19)
- [9] ed655d1fc — Core: DefaultIniFileContent and CurrentIniFileContent added (cxl, 2013-11-04)
- [10] 697d44360 — Core: ConvertTime adds AlwaysTime and DayEnd helpers (cxl, 2013-11-27)
- [11] ca57f97f9 — CtrlLib: TextCtrl::Load performance optimization (cxl, 2013-11-29)
- [12] 76d32f9c9 — ide: Layout Designer generator adds double-quote option (cxl, 2013-11-22)
- [13] 52534f01a — ide: Ids completion support added (cxl, 2013-11-23)
- [14] 84ced1005 — CtrlLib: EditDateDlg introduced (cxl, 2013-11-29)
- [15] 7725d8b64 — Core: Debug.cpp hardened for AMD64 architectures (cxl, 2013-11-17)
- [16] e12527eb3 — CtrlLib: DropList adds NoWheel option (cxl, 2013-11-08)
- [17] 3a36180d3 — Core: Ref support for Geom, ValueMap, and Complex (cxl, 2013-11-05)
- [18] 7fe105901 — CtrlCore: GTK fixed KeyDesc for non-alnum characters (cxl, 2013-11-25)
- [19] 5f759fddf — CtrlCore: WM_HOTKEY handling fixes (cxl, 2013-11-29)
- [20] 5bfb4a3bb — SysInfo: VMware support fixes (koldo, 2013-11-22)
