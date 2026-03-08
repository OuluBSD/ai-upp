# The Multi-Threaded Draw Refactor and Native D&D (2012-11)
**Date Span:** 2012-11-01 to 2012-11-30

November 2012 was a month of deep architectural renovation, specifically targeting the graphical subsystem's concurrency and the framework's integration with desktop shells. The most significant effort was a **major refactor of the Draw and CtrlCore packages**. This overhaul focused on resolving subtle multi-threading (MT) race conditions across all backends, including Win32, X11, and the Rainbow project's WinGL. To aid in maintaining this new level of thread-safety, the framework introduced specialized **ASSERTs to detect global variable widgets and missing GuiLock** calls—common sources of instability in complex UI applications. This hardening was immediately validated with the `MtDraw` unit test and a new `MtRpc` reference example.

Integration with the operating system reached a professional milestone with the introduction of **native file support in the clipboard and Drag & Drop (D&D)**. By implementing `GetClipFiles` for both Win32 and X11, U++ applications gained the ability to seamlessly exchange file lists with system explorers like Windows Explorer or GNOME Files. This was supported by refined `PasteClip` semantics and improved handling of "files" as a primary clipboard format, significantly enhancing the framework's utility for file management and productivity software.

The database and serialization layers continued their steady maturation. `SqlExp` reached a new level of developer ergonomics with the ability to perform **Insert and Update operations using ValueMap parameters**, effectively automating the mapping between UI data and database columns. Serialization reached a cleaner state as `Xmlize` and `Jsonize` methods were moved directly into container classes like `Array` and `Vector`, resulting in a more intuitive, object-oriented API. The **Skylark** framework also received improved error handling and the `Http::Finalize` method for better control over the request-response lifecycle.

TheIDE and professional toolchains were refined for high-efficiency workflows. The environment added a "Mimic case" option to the find dialog and improved the **Assist++** system to provide correct line-number information for database schema (`.sch`) files. Syntax highlighting was further polished for nested templates (fixing the `>>` bracket parsing issue) and broadened for managed environments with improved CLR and Objective C support. In the Bazaar, the **OCE** (OpenCascade) package underwent a massive cleanup and synchronization, while the **HelpViewer** added browser-style "Back" and "Forward" navigation, bringing its user experience in line with modern documentation standards.

## References
- [1] bd87a04fd — Draw, CtrlCore: Major refactor of the Draw subsystem (cxl, 2012-11-11)
- [2] a16cf6c45 — CtrlCore: Resolved multi-threading (MT) issues in Draw (cxl, 2012-11-15)
- [3] 48fa0dd5e — CtrlCore: ASSERTs to detect global widgets and missing GuiLock (cxl, 2012-11-28)
- [4] 32228e486 — CtrlCore: GetClipFiles for native clipboard file lists (cxl, 2012-11-05)
- [5] 59e993ba1 — CtrlCore: 'files' support in X11 Drag & Drop (cxl, 2012-11-05)
- [6] eab97075d — SqlExp: Insert and Update with ValueMap parameters (cxl, 2012-11-09)
- [7] 50716a53a — Core: Xmlize/Jsonize of containers moved to container methods (cxl, 2012-11-01)
- [8] 0d8002fa2 — Skylark: Improved error handling (cxl, 2012-11-26)
- [9] 110d99262 — ide: "Mimic case" option in Find dialog added (cxl, 2012-11-26)
- [10] 0cecd9eff — ide: Assist++ adds correct line numbers for .sch files (cxl, 2012-11-04)
- [11] 163955414 — ide: Fixed parsing of >> as nested template brackets (cxl, 2012-11-28)
- [12] 455f92095 — Bazaar: HelpViewer adds Back and Forward buttons (micio, 2012-11-03)
- [13] 134a151c9 — Bazaar: OCE package cleanup and upstream sync (micio, 2012-11-19)
- [14] 4d2194c7b — Bazaar: PolyXML adds load/save progress callbacks (micio, 2012-11-28)
- [15] 304df59cb — Core: TrimLeft/TrimRight with prefix/suffix variants (cxl, 2012-11-10)
- [16] 231b6cdc8 — Core: BOM functions add def_charset parameter (cxl, 2012-11-08)
- [17] a9b5a6d19 — reference: Multi-threaded RPC client/server example (unodgs, 2012-11-17)
- [18] 0490e0821 — Core: HTTP handling improved for >2GB content (cxl, 2012-11-19)
- [19] d12fb1eb8 — plugin/Sqlite3: Upgraded to version 3.7.8 (cxl, 2012-11-04)
- [20] 844e2956d — CtrlCore: InstallPanicBox implemented for Win32 (cxl, 2012-11-28)
