# Header/Footer Support and the RichEdit Floating Mode
**Date Span:** 2013-10-01 to 2013-10-31

### Professional Document Processing
Launched native **Header and Footer support** in `RichText` and `RichEdit`, enabling the creation of multi-page documents with consistent metadata. This was complemented by the new **Floating mode**, providing a zoomable, paper-like interface for a superior editing experience.

### Core Precision and JSON Dates
The **CParser** was hardened with comprehensive **integer and floating-point overflow checks**. The **JSON engine** was upgraded to natively handle **Date and DateTime** types, and the core library gained **UTC leap seconds** logic and the `EasterDay` function.

### Advanced Connectivity
`HttpRequest` gained **KeepAlive** and **CommonProxy** support, streamlining proxy-aware persistent connections. The SQL engine reached better parity with database standards by adding `CURRENT_DATE` and `CURRENT_TIMESTAMP` support.

### Tooling and GLES Acceleration
TheIDE's code editor introduced **thousands separator highlighting**, and **IconDes** gained a dedicated text tool. The **Rainbow** project reached a new frontier with the development of **GLESDraw**, extending hardware acceleration to OpenGL ES devices.
