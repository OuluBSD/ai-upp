# The Web Core Migration and C++11 Readiness (2012-04)
**Date Span:** 2012-04-01 to 2012-04-30

April 2012 was a month of deep architectural consolidation, marking the formal integration of high-level networking and security features into the framework's foundation. The most significant shift was the **migration of the Web package into Core**. Fundamental components like `TcpSocket`, `HttpRequest`, `IpAddrInfo`, and the `SSL` support layer were moved to the canonical `Core` library. This transformation provided all U++ applications with native, high-performance HTTP capabilities—including chunked transfer support, refined redirection logic, and built-in SSL/TLS encryption—without external package dependencies. The move was supported by the arrival of the `GuiWebCrawler` and `GuiWebDownload` reference examples, showcasing the framework's new "batteries-included" approach to the modern web.

The framework also reached a major language milestone with the first formal support for **C++0x (C++11)**. By updating core sources to be compliant with the emerging standard, U++ ensured that developers could begin leveraging modern C++ features like auto-typing and lambda expressions within their projects. Parallel to this, the **Jsonize** engine was further hardened with support for maps with String keys and critical fixes for negative number parsing, while `Value` serialization achieved full parity across complex nested structures.

The Linux development experience saw a "massive cleanup" of the **Gdb_MI2** debugger interface. This effort introduced professional-grade **pretty printers** for common U++ types like `Point`, `Rect`, and `Size`, separated types from values in the variable inspection tabs, and implemented robust asynchronous break commands. The interface also achieved better multi-threaded control, now ensuring that all threads are correctly stopped when a breakpoint is hit, providing a deterministic debugging environment for high-concurrency Linux applications.

Graphical and professional tooling continued their steady advance. The **WinGL** backend for the Rainbow project added support for perspective views and refined its internal timers, while `RichEdit` gained the `ClipZoom` method and improved table selection logic. TheIDE's text editor was polished with fixes for space-based indentation and better syntax highlighting for layout files. The framework's connectivity suite was also rounded out with the integration of **ZeroMQ (ZMQ)** and the refactoring of `SmtpMail` to support custom headers, while the build system added formal support for **Ubuntu 12.04 (Precise Pangolin)**, ensuring U++ remained current with the latest long-term support distributions.

## References
- [1] 8aace300c — Core/SSL: SSL support for TcpSocket and HttpRequest completed (cxl, 2012-04-14)
- [2] 4a31c8564 — U++: Source code updated to support C++0x (C++11) (cxl, 2012-04-19)
- [3] 408688d24 — theide: Gdb_MI2 debugger massive cleanup and bugfixes (micio, 2012-04-29)
- [4] cb3438d06 — theide: Gdb_MI2 adds pretty printers for Point, Rect, and Size (micio, 2012-04-29)
- [5] 6ad7fbb90 — Core: HttpRequest adds native chunked transfer support (cxl, 2012-04-01)
- [6] 4bd0b5bb1 — reference: GuiWebCrawler example introduced (cxl, 2012-04-16)
- [7] c57d9f322 — reference: GuiWebDownload example introduced (cxl, 2012-04-18)
- [8] 754f670cd — Core: XmlRpc over TcpSocket integrated into Core (cxl, 2012-04-24)
- [9] b707d3e72 — Core: LoadFromJsonFile and StoreToJsonFile added (cxl, 2012-04-19)
- [10] eaac833bc — Rainbow: WinGL adds support for perspective views (unodgs, 2012-04-24)
- [11] a2cdb45a7 — Core: ParseJSON fixed for negative numbers (cxl, 2012-04-19)
- [12] be2da80f7 — Core: LoadFromXMLFile returns false if file missing (cxl, 2012-04-19)
- [13] 2c949ecc5 — Core: HttpRequest adds specialized BEGIN phase (cxl, 2012-04-16)
- [14] 57a59b99e — Core: Container Set/Insert methods continue transition to T& (cxl, 2012-04-17)
- [15] 6189567f3 — ide: CodeEditor fixes for space-indented files (rylek, 2012-04-19)
- [16] 939cc4364 — CtrlLib: HeaderCtrl adds double-click and click callbacks (cxl, 2012-04-27)
- [17] d14fa94a3 — Sql: schema REFERENCES can be preset by Sql (cxl, 2011-11-16)
- [18] 1ad7e0541 — Bazaar: Updater adds MinGW and folder management support (koldo, 2011-10-13)
- [19] 82fdf80b0 — CtrlCore: EncodeRTF charset encoding fixes (cxl, 2012-04-05)
- [20] dc6362b18 — Draw: Fixed invalid underline issue in X11 (cxl, 2012-04-27)
