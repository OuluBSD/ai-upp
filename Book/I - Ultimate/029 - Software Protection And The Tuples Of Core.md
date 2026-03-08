# Software Protection and the Tuples of Core (2010-09)
**Date Span:** 2010-09-01 to 2010-09-30

September 2010 was a month of significant expansion for the U++ core library and a surge in security-focused tools within the Bazaar. The core library reached a new level of expressive power with the introduction of the `Tuple` template, providing a standardized way to group heterogeneous data elements. This was accompanied by the addition of `ReverseFind` to both `String` and `WString`, and a vital ergonomic change: all `Add` methods in `Vector` and `Array` were updated to return a reference to the newly added element, streamlining the common pattern of adding and then immediately configuring an object.

The Bazaar saw the launch of the **Protect** package, a sophisticated software protection and encryption tool designed to obfuscate and secure sensitive code sections. Initially supporting RC4 and later transitioning to the more modern Snow2 encryption, `Protect` allowed for the encryption of function bodies and variable declarations, complete with key verification and obfuscation. This security push was further bolstered by the `StreamCypher` package, providing dedicated classes for stream-based data encoding and decoding.

User interface components received substantial functional and aesthetic upgrades. `HelpViewer` was introduced as a dedicated help window class featuring native Table of Contents (TOC) handling, significantly improving the built-in documentation experience. `GridCtrl` continued its maturation with improvements to its property-grid mode, better `SyncCtrls` logic, and new "before-change" callbacks for rows and columns. Visual responsiveness was enhanced by a deep refactor of the `Animate` system, resulting in much smoother transitions for `DropList`, `ColorPopUp`, and `DropTree` controls.

Graphical and media support was hardened with a focus on professional imaging standards. The `plugin/png`, `tif`, and `jpg` packages were updated to correctly handle DPI (Dots Per Inch) metadata, and the `GetDPI()` and `SetDPI()` methods were formally documented. The `RasterPlayer` demo reached maturity, offering a comprehensive way to visualize animated GIFs and multi-page rasters with full transparency support. Connectivity remained a priority, with `XmlRpc` gaining improved `int64` handling and specialized error overloads, while the PostgreSQL driver received hardened auto-reconnect logic to better handle transient network failures.

## References
- [1] 96de5520e — Core: Tuple introduced (cxl, 2010-09-06)
- [2] d67de42b5 — Bazaar: Protect software protection tool introduced (micio, 2010-09-19)
- [3] a13d1318d — Bazaar: StreamCypher encoding/decoding classes (micio, 2010-09-29)
- [4] 5d8cca787 — HelpViewer: TOC-aware help window class (micio, 2010-09-01)
- [5] dcd13319e — Core: Vector/Array Add methods return T& (cxl, 2010-09-16)
- [6] fc6bed392 — CtrlLib: Improved Animate system for smooth transitions (cxl, 2010-09-10)
- [7] 6f7f9ac46 — Graphics: DPI info handling fixed for PNG/TIFF/JPG (cxl, 2010-09-17)
- [8] 4fccf86ab — Media: RasterPlayer demo for animated clips (koldo, 2010-09-09)
- [9] 693b2b2d1 — PGSQL: Autoreconnect improvement (cxl, 2010-09-17)
- [10] 6de637e70 — XmlRpc: Improved int64 handling (cxl, 2010-09-15)
- [11] dab65ac0a — GridCtrl: SyncCtrls and SetCtrl improvements (unodgs, 2010-09-01)
- [12] 9f6630f31 — GridCtrl: Added WhenBeforeChangeRow/Col (unodgs, 2010-09-12)
- [13] 478c24555 — Bazaar: Tree serialization and Xmlize support (kohait, 2010-09-06)
- [14] cfecef441 — theide: QTF designer edits current selection (dolik, 2010-09-08)
- [15] 07969f24f — Functions4U: Linux Dl support and LaunchFile improvements (koldo, 2010-09-07)
- [16] 19b3c09d0 — SysInfo: GetAdapterInfo added; GetMacAddress deprecated (koldo, 2010-09-07)
- [17] 296e44e50 — Core: ReverseFind for [W]String (cxl, 2010-09-03)
- [18] 10de1d84d — Draw: AttrText::NormalInk added (cxl, 2010-09-16)
- [19] 66fb93882 — GridCtrl: FindCol(const Id&) and Hidden() fixes (unodgs, 2010-09-27)
- [20] 233a79bb7 — Protect: RC4 replaced with Snow2 encryption (micio, 2010-09-29)
