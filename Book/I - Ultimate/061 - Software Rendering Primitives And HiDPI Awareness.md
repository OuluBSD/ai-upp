# Software Rendering Primitives and HiDPI Awareness (2013-05)
**Date Span:** 2013-05-01 to 2013-05-31

May 2013 was a month of deep graphical exploration and the first major steps toward modern high-resolution displays. The framework's internal drawing capabilities underwent a significant expansion with the development of **SDraw** and the **DDARasterizer**. Designed to provide high-quality software rendering, the `DDARasterizer` introduced sophisticated handling for "fat line" joins and polylines, bringing anti-aliased, vector-quality graphics to non-accelerated backends. This push for graphical precision was immediately integrated into **IconDes**, which transitioned to using the new rasterizer and gained an **AutoAliasing** function, allowing icon designers to produce sharper assets with less manual effort.

A landmark shift in display technology was addressed with the first targeted **HiDPI fixes for Win32**. As high-density displays began to enter the market, U++ started adapting its coordinate systems and rendering logic to ensure that applications remained crisp and correctly scaled on these new panels. This effort was supported by refinements to the `CtrlCore` layout engine and the introduction of the `NoLayoutZoom` fix, ensuring that project templates maintained visual integrity across varying DPI settings.

Core library and connectivity features reached new levels of professional utility. The framework gained **GCC stack trace** support, providing developers with much-needed diagnostic information during Linux-side crashes. The XML engine was enhanced with `RegisterEntity` support and the new `TryLoadFromXML` helper, while `Jsonize` was extended to handle `int16` types. Networking was hardened with **per-thread language settings**, essential for internationalized servers, and the **MySQL** driver achieved better parity by implementing `mysql_library_init` and `mysql_library_end` calls.

TheIDE and its suite of professional tools were refined for maximum productivity. The **Layout Designer** gained a "Duplicate layout" feature and improved scrollbar handling during item copying. The **Qtf Designer** was upgraded with native splitter support, and the environment's **AutoSetup** was modernized to recognize and configure the **Visual C++ 2012** toolchain. Community initiatives continued to bridge the gap between U++ and other specialized fields: **ScatterCtrl** achieved full compatibility with the GTK backend, and **MAPIEx** was refined for better Outlook folder management. The month closed with the official move of the **Eigen** matrix library into the canonical `uppsrc` collection, signaling its status as a foundational pillar for scientific computing in U++.

## References
- [1] 8825606e3 — Draw: SDraw software rendering introduced (cxl, 2013-05-19)
- [2] 5cd7ce611 — Draw: DDARasterizer; IconDes transitions to software rasterization (cxl, 2013-05-12)
- [3] 837a01cf0 — CtrlCore: Initial HiDPI fixes for Win32 (cxl, 2013-05-21)
- [4] 2af64e88b — Core: GCC stack trace support added (cxl, 2013-05-03)
- [5] 3ea6bfe39 — ide: AutoSetup adds support for Visual C++ 2012 (cxl, 2013-05-28)
- [6] 9631651dd — ide: Duplicate layout feature added to designer (cxl, 2013-05-25)
- [7] 3c5adca76 — ide: AutoAliasing function in IconDes (cxl, 2013-05-01)
- [8] bfe64aa97 — MySQL: Native library initialization and termination (cxl, 2013-05-26)
- [9] 6b08e4c86 — ScatterCtrl: Achieved GTK backend compatibility (cxl, 2013-05-26)
- [10] 238fbf6c3 — Core: XML RegisterEntity support added (cxl, 2013-05-15)
- [11] 349db62c6 — Core: TryLoadFromXML utility introduced (cxl, 2013-05-25)
- [12] 9a1dcdd11 — Core: Jsonize support for int16 (cxl, 2013-05-14)
- [13] 6635d3a24 — Draw: DDARasterizer FatLine joins fixed and improved (cxl, 2013-05-15)
- [14] b1cdb85e4 — CtrlCore: Win32 SystemDraw optimized with CompatibleDC (cxl, 2013-05-16)
- [15] 0a971b7e7 — plugin/jpg: CMYK support added (cxl, 2013-05-26)
- [16] 07ae2d7ef — RichEdit: Copy tool enabled in ReadOnly mode (cxl, 2013-05-26)
- [17] 85e21ef0e — CtrlLib: Parametrized UI update system (cxl, 2013-05-23)
- [18] 66433d6af — GridCtrl: WhenSorted callback added (unodgs, 2013-05-08)
- [19] 8c42ebdc6 — MAPIEx: Improved OpenSubFolder() for Outlook (koldo, 2013-05-21)
- [20] 42f88f553 — CtrlCore: Continued development of Rainbow backend (cxl, 2013-05-29)
