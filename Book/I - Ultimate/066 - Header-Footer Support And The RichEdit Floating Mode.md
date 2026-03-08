# Header/Footer Support and the RichEdit Floating Mode (2013-10)
**Date Span:** 2013-10-01 to 2013-10-31

October 2013 was a transformative month for U++'s document processing capabilities and the technical precision of its core library. The most significant feature addition was the introduction of native **Header and Footer support** in the `RichText` and `RichEdit` packages. This architectural expansion allowed U++ developers to create professional multi-page documents with consistent branding and metadata, further bolstered by the arrival of the **Floating mode** in `RichEdit`. This new visual mode provided a zoomable, paper-like interface for document editing, significantly improving the user experience for word-processing applications. These document features were complemented by improvements to **EncodeQtf** and the introduction of `GetHeaderPtr` / `GetFooterPtr` for programmatic document manipulation.

The core library's foundations reached new levels of precision and safety. The **CParser** was upgraded with comprehensive **integer and floating-point overflow checks**, ensuring that custom parsers and data loaders remained robust against malformed or malicious input. The **JSON engine** was also hardened with native **Date and DateTime support**, enabling U++ applications to exchange temporal data more effectively with modern web services. Core time handling was further refined with the addition of **UTC leap seconds** logic and the new `EasterDay` calculation function. For system-level flexibility, the **INI configuration system** was enhanced to support environment variable expansion (parsing `$VARIABLES`), providing better deployment versatility for server-side tools.

Networking and connectivity remained a primary focus. **HttpRequest** gained support for **KeepAlive** and **CommonProxy** settings, streamlining the creation of persistent, proxy-aware web connections. The **Skylark** framework received critical signal handling fixes, and the SQL expression engine reached better parity with modern database standards by adding native support for `CURRENT_DATE` and `CURRENT_TIMESTAMP`. In the Bazaar, **MAPIEx** achieved full compatibility with the **MinGW** toolchain, opening up Outlook automation to a broader range of Windows developers.

TheIDE and its suite of professional tools continued their steady advance. The environment's intelligence was bolstered by **thousands separator highlighting** in the code editor, making large numeric constants significantly more readable. The **IconDes** editor was upgraded with a dedicated **text tool**, and the **Layout Designer** was refined to better handle item duplication and navigation via `Alt+Left/Right`. The **Rainbow** project also reached a new milestone with the development of **GLESDraw**, extending the framework's high-level hardware acceleration to OpenGL ES devices, supported by the arrival of the **Glew** library within the core drawing stack.

## References
- [1] d5b166704 — RichText/RichEdit: Header and Footer support introduced (cxl, 2013-10-23)
- [2] 90368d7d8 — RichEdit: Floating (zoomable paper) mode introduced (cxl, 2013-10-28)
- [3] 935ce18d3 — Core: CParser adds integer overflow checks (cxl, 2013-10-21)
- [4] 4c75f84cf — Core: CParser adds floating-point overflow checking (cxl, 2013-10-24)
- [5] e51439e78 — Core: JSON adds native Date/DateTime support and UTC leap seconds (cxl, 2013-10-06)
- [6] f09028702 — Core: .ini system parses environment variables ($VAR) (cxl, 2013-10-18)
- [7] d0b6dfa59 — Core: EasterDay calculation function added (cxl, 2013-10-30)
- [8] 052235e6f — Core: HttpRequest::KeepAlive support added (cxl, 2013-10-24)
- [9] c06923577 — Core: HttpRequest::CommonProxy support introduced (cxl, 2013-10-23)
- [10] 07bd9d4a7 — SqlExp: CURRENT_DATE and CURRENT_TIMESTAMP support (cxl, 2013-10-06)
- [11] 2da4535f7 — CodeEditor: Thousands separator highlighting in numeric literals (cxl, 2013-10-11)
- [12] 8c735c456 — ide: IconDes adds dedicated text tool (cxl, 2013-10-28)
- [13] a5eca711a — GLDraw: Glew added to the core drawing stack (cxl, 2013-10-16)
- [14] 6f45d6254 — Rainbow: GLESDraw (OpenGL ES) development started (cxl, 2013-10-16)
- [15] c1d9ea836 — Bazaar: MAPIEx achieves MinGW toolchain compatibility (koldo, 2013-10-21)
- [16] 4b20f0a1c — GridCtrl: InsertColumn method and joined-cell fixes (unodgs, 2013-10-19)
- [17] c2de19113 — Core: GetMonths(Date, Date) utility added (cxl, 2013-10-15)
- [18] a67030a4b — Rainbow: SDL20GL adds SDL color cursor support (cxl, 2013-10-30)
- [19] 0bc4bbf21 — SysInfo: GetCpuTemperatureHWMON() optimization (koldo, 2013-10-30)
- [20] 0239c9742 — CtrlLib: WithDropChoice reacts to mouse wheel (cxl, 2013-10-03)
