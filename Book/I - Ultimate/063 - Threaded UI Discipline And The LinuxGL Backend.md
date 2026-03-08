# Threaded UI Discipline and the LinuxGL Backend (2013-07)
**Date Span:** 2013-07-01 to 2013-07-31

July 2013 was a month of rigorous architectural discipline and the expansion of U++'s high-performance graphical reach. The most profound structural shift was the enforcement of **new multi-threading (MT) rules** within `CtrlCore`. Recognizing that cross-platform UI stability is best achieved through strict thread affinity, the framework officially **banned window creation and event loops in non-main threads**. This "main thread only" policy for core UI operations, implemented across Win32, X11, and GTK, eliminated a vast category of race conditions and deadlock scenarios. The `GuiLock` reference and related multi-threading documentation were immediately updated to reflect this new paradigm, ensuring that developers transitioned to safe, secondary-thread processing patterns.

The **Rainbow** project reached a new peak of cross-platform acceleration with the launch of the **LinuxGl** backend. This initial version provided a native OpenGL-accelerated GUI path for Linux, complementing the previous year's WinGL work. To support this hardware-accelerated future, the core **Color** class was enhanced with **alpha channel support** (specifically for OpenGL contexts), and `WinGL` received its own suite of GUI locks and conforming blur shaders. These additions positioned U++ as one of the few toolkits capable of running a full desktop UI suite entirely within an OpenGL or Framebuffer context across both major operating systems.

Connectivity and web service integration remained a high priority. The **Skylark** framework and the core `HttpHeader` parser underwent intensive refinement to resolve **SCGI (Simple Common Gateway Interface) issues**, particularly regarding Apache and Nginx header compatibility. This effort was supported by the Bazaar's **Scgi** package, which was refactored to remove all legacy `Web` package dependencies, achieving a cleaner, `Core`-based architecture. The **Protect** and **Updater** packages also completed their transition away from the legacy web stack, signaling the final phase of the framework's modular consolidation.

User interface ergonomics continued their steady march toward professional perfection. `CtrlLib` introduced the **EditNumber** control, providing a specialized, type-safe input field for numeric data. **ArrayCtrl** and **HeaderCtrl** were upgraded with improved support for hidden and dynamically reorganized columns, and a new `WhenHeaderLayout` callback allowed applications to respond to user-driven column changes in real-time. Visual fidelity was also hardened with fixes for X11 window background rendering and improved KDE compatibility within the GTK+ Chameleon backend. The month closed with the addition of **Euskara (Basque)** translations for the system information suite, further expanding the framework's accessibility to global developers.

## References
- [1] f0935e1f9 — CtrlCore: New MT rules banning window/event loops in non-main threads (cxl, 2013-07-06)
- [2] a2bd8491a — Rainbow: LinuxGl OpenGL-accelerated backend initial version (unodgs, 2013-07-11)
- [3] 364b2c88d — CtrlLib: EditNumber control introduced (cxl, 2013-07-11)
- [4] a74fd2942 — Core: Alpha channel support added to Color class for OpenGL (unodgs, 2013-07-17)
- [5] 10964f85e — Skylark: Critical SCGI and Apache header fixes (cxl, 2013-07-16)
- [6] 74d22d9cf — Bazaar: Scgi package refactored to remove legacy Web dependency (micio, 2013-07-07)
- [7] 0758a8d83 — UI: ArrayCtrl/HeaderCtrl support for hidden/reorganized columns (cxl, 2013-07-10)
- [8] 77ad1e9ca — CtrlLib: ArrayCtrl::WhenHeaderLayout callback introduced (cxl, 2013-07-09)
- [9] 1851fe470 — WinGL: Added GUI locks and compliant blur shaders (unodgs, 2013-07-11)
- [10] a44dba31f — Core: Http redirection logic refined to switch to GET (cxl, 2013-07-12)
- [11] 900ba11ca — CtrlCore: X11 background rendering optimized with CWBackPixmap (cxl, 2013-07-16)
- [12] 8d23a99c6 — CtrlLib: Improved KDE support in FileSel (cxl, 2013-07-29)
- [13] f2a55b863 — Core: SetSysTime implementation for Win32 fixed (cxl, 2013-07-10)
- [14] 52d5ca1d2 — Sql: ExportSch now uppercasing table names consistently (cxl, 2013-07-19)
- [15] 9344c42e5 — CtrlLib: HeaderCtrl serialization refactored (cxl, 2013-07-21)
- [16] b3deb0be6 — ArrayCtrl: Automatic line clearing before setting Vector data (cxl, 2013-07-16)
- [17] 26bf0bba9 — Core/SSL: Fixed potential memory leak in handshake (cxl, 2013-07-22)
- [18] 419f2a246 — Core: String::ReverseFind fixed for empty string edge case (cxl, 2013-07-25)
- [19] 03680d92b — ArrayCtrl: Alt-click on header copies individual widths (cxl, 2013-07-19)
- [20] 91e89dfa0 — SysInfo: Basque (Euskara) translation added (koldo, 2013-01-19)
