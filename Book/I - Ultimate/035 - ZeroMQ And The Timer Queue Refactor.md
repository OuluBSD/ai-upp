# ZeroMQ and the Timer Queue Refactor (2011-03)
**Date Span:** 2011-03-01 to 2011-03-31

March 2011 was a month of deep architectural refinement and the introduction of next-generation messaging patterns to the U++ ecosystem. The most significant structural change occurred within the core event loop: a complete refactor of the **Timer queue** in `CtrlCore`. By switching to a model where repeated events are rescheduled *after* their callbacks execute, the framework effectively eliminated a class of recursion bugs that previously plagued long-processing tasks. This change, supported by a flurry of corner-case tuning for both Win32 and X11, resulted in a more predictable and stable user interface under heavy computational load.

The framework's connectivity options leaped forward with the introduction of the **ZeroMQ (ZMQ)** library wrapper in the Bazaar. ZeroMQ provided developers with a high-performance, asynchronous messaging stack, enabling complex patterns like request-reply, pub-sub, and multicast with minimal latency. This was complemented by refinements to the `ScgiServer`, which gained a configurable `listenCount` parameter, and improvements to the `OleDB` backend, which now correctly supported the fetching of multiple `BLOB` columns from a single table.

TheIDE received several high-impact professional upgrades. The help and documentation system gained the ability to export entire topic groups directly to **PDF**, making it easier to generate offline manuals. The build system was also made more isolated and robust by implementing unique output directories per assembly—appending the assembly name to the output path to prevent binary collisions between different project configurations. Furthermore, a new **SrcUpdater** system was introduced, featuring a persistent timer to keep development environments in sync with upstream changes.

The Bazaar continued to be a hotbed of experimentation. A very early "alpha" implementation of **Python** integration began to appear, signaling the framework's future interest in scripting languages. The **LogCtrl** was introduced, providing a way to redirect the entire internal U++ logging facility (`RLOG`, `DLOG`, etc.) to a UI control for real-time in-app diagnostics. The suite of live-UI-editing tools reached a peak of maturity with **CtrlPos** and **CtrlMover** gaining support for reordering controls, changing parent-child relationships via drag-and-drop, and improved deep search capabilities within complex layouts. Linux support remained a priority with fixes for the emerging Ubuntu Natty Narwhal, specifically addressing `libnotify` API changes and `gdk-pixbuf` include paths.

## References
- [1] 820deb433 — CtrlCore: TimerProc refactored to prevent recursion (cxl, 2011-03-19)
- [2] fed0c3ac8 — Bazaar: ZeroMQ (ZMQ) library and examples introduced (tojocky, 2011-03-06)
- [3] 159292b1e — theide: Export whole topic groups to PDF (cxl, 2011-03-02)
- [4] c8a19dd5a — theide: Unique output directories per assembly (cxl, 2011-03-06)
- [5] 23726018d — theide: New source updating system (SrcUpdater) (dolik, 2011-03-25)
- [6] a182ca9f2 — Bazaar: LogCtrl redirects U++ logging to a UI control (kohait, 2011-03-31)
- [7] 15fe03fb1 — Bazaar: Initial alpha Python integration and test cases (kohait, 2011-03-30)
- [8] 1fd9cc319 — Bazaar: CtrlPos reordering and parent change support (kohait, 2011-03-22)
- [9] 873b37cd9 — Web: ScgiServer Run adds listenCount parameter (cxl, 2011-03-25)
- [10] bde0dfac5 — OleDB: Fixed support for multiple BLOB columns (rylek, 2011-03-29)
- [11] eb6dcc4fe — GridSql: Sql package automatically included via flag (unodgs, 2011-03-20)
- [12] 8cdf08df0 — X11TrayIcon: LibNotify API change for Ubuntu Natty (cxl, 2011-03-08)
- [13] 7814d266c — lpbuild: Added Natty Narwhal build support (dolik, 2011-03-06)
- [14] fbe0dc28d — EditField: GetCaretRect introduced (cxl, 2011-03-12)
- [15] 9004bbc0e — plugin/png: Fixed transparency (tRNS) interpretation (rylek, 2011-03-12)
- [16] e24ec33d4 — ArrayCtrl: Move() method made public (cxl, 2011-03-18)
- [17] 66c4ee912 — SqlExp: SQLite3 operator| and Coalesce fixes (cxl, 2011-03-06)
- [18] 1762408ff — Ole: Resolved clashing Font declarations (rylek, 2011-03-04)
- [19] 0c4c0cf2f — theide: GCC builder puts libraries at the end of the link list (cxl, 2011-03-04)
- [20] 6adfb0a1b — Web: Fixed crash on Socket::Clear for picked sockets (rylek, 2011-03-01)
