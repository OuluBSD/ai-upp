# Polymorphic XML and the Bazaar Revolution (2010-01)
**Date Span:** 2010-01-01 to 2010-01-31

January 2010 was a month of significant high-level expansion, particularly within the community-driven Bazaar and the framework's serialization capabilities. The most notable architectural addition was `PolyXML`, a polymorphic XMLizer. This new system provided a standardized way to serialize and deserialize complex C++ object hierarchies where the exact type of a derived object is only known at runtime. By integrating with a central `ClassFactory` and the `REGISTERCLASS()` macro, `PolyXML` simplified the handling of dynamic plugin systems and complex document formats, immediately bolstered by exhaustive documentation and refinement of its `CreateInstance` API.

The U++ Bazaar underwent a "revolution" in both content and visibility. The web-based community portal was redesigned to allow authors direct editing access, fostering a more collaborative environment. New high-impact packages arrived: `TabBarCtrl` provided a robust, callback-driven tabbed widget interface, while `UnitTest++` gained a dedicated documentation drive. Most importantly, the framework achieved "passive notification" parity on Linux with the integration of `libnotify` into the core `TrayIcon` class. This allowed U++ applications to display native desktop alerts on Gtk-based environments, complete with new `.deb` dependency requirements for `libnotify-dev`.

Networking and security saw critical upgrades with the introduction of **Digest Authentication** support. This provided a more secure alternative to Basic Auth for the framework's HTTP and web service clients. The `ftp` client's progress callbacks were also refined to align with the framework's standard `Gate2<int, int>` paradigm. Core library stability was reinforced with the `BREAK_WHEN_PICKED` debugging aid and a critical fix for `FindFile` to correctly handle international (i18n) characters in POSIX environments.

TheIDE and the framework's documentation reached new levels of interactivity. The help system was upgraded to highlight search results and allow for fluid up/down navigation between matches. The PDB debugger's string display was further improved, and `GUI_APP_MAIN` was updated to automatically handle command-line encoding conversions. Visually, the framework's outreach expanded with the announcement of the "UltimateBook" project, featuring a new web presence and sample covers, signaling a long-term commitment to comprehensive written documentation. Graphical refinements continued in the `Painter` package, which saw improved image filtering for downscaled assets and the completion of `DrawImageOp` to support all original `Draw` modes.

## References
- [1] 5bb635bb1 — PolyXML: Polymorphic XMLizer introduced (micio, 2010-01-13)
- [2] a3abe8aa9 — PolyXML: API refined to avoid Ctrl::Create clashes (micio, 2010-01-14)
- [3] 1c575935a — CtrlLib: libnotify integration for TrayIcon alerts (cxl, 2010-01-24)
- [4] 64d639153 — Bazaar: TabBarCtrl widget added (micio, 2010-01-17)
- [5] 04e8292db — Core: Digest authentication support added (rylek, 2010-01-27)
- [6] 389497da9 — TheIDE: Help system highlights search words with navigation (cxl, 2010-01-21)
- [7] 190cbf566 — Core: FindFile fixed for POSIX i18n characters (cxl, 2010-01-19)
- [8] a7e51f4fd — Core: BREAK_WHEN_PICKED debugging aid (cxl, 2010-01-24)
- [9] bc837f500 — Painter: Improved filtering for downscaled images (cxl, 2010-01-25)
- [10] 0f45b8005 — Painter: DrawImageOp completed for all Draw modes (cxl, 2010-01-25)
- [11] 947ea6707 — UltimateBook: Web page and project launch (koldo, 2010-01-20)
- [12] 9116f5d23 — CtrlCore: X11 SetAlpha support (cxl, 2010-01-23)
- [13] 164d4b49d — CtrlCore: GUI_APP_MAIN converts commandline encoding (cxl, 2010-01-29)
- [14] 21d5c4113 — FtpClient: progress callbacks use Gate2 (rylek, 2010-01-19)
- [15] 73f60a186 — Draw: Optimized missing glyph search with coverage bitmaps (cxl, 2010-01-29)
- [16] 17d744651 — SysInfo: Keyb_SendKeys adds Unicode support (koldo, 2010-01-12)
- [17] 87f2782d5 — CtrlLib: ArrayCtrl new Sort variants (cxl, 2010-01-28)
- [18] 7e9a3bbdf — CtrlLib: SliderCtrl::Jump added (cxl, 2010-01-28)
- [19] 50b2f915c — XML: XML output documented (cxl, 2010-01-02)
- [20] 4faac771a — Core: XmlParser documented (cxl, 2010-01-06)
