# Core Optimizations and Multipart Web Support (2013-06)
**Date Span:** 2013-06-01 to 2013-06-30

June 2013 was a month of significant internal data structure optimization and the arrival of advanced web protocol features. The core library reached a new peak of efficiency with a major overhaul of **XmlNode**. By refactoring the internal representation of XML elements, the framework achieved an average **40% reduction in memory size** for XML trees—a vital optimization for memory-intensive document processing. This was accompanied by refinements to the **InVector** container, resolving 64-bit portability warnings and ensuring stable performance during element removal.

The framework's web and networking capabilities were substantially bolstered. **HttpRequest** gained native **multipart support**, enabling U++ applications to easily handle complex form data and file uploads. The `Socket` class, and by extension the **Skylark** web framework, were upgraded to support **listening on specific IP addresses**, providing developers with finer control over network service binding in multi-homed environments. These features were supported by improved error handling and the adoption of modern RESTful patterns introduced in previous months.

Architectural decoupling reached a milestone with the refactoring of the **Docking** package. The system was re-engineered to **remove platform-dependent code**, moving its logic closer to the core library and ensuring consistent behavior across Win32, X11, and the emerging Rainbow backends. This effort also resolved persistent issues with tool windows being incorrectly resized when docked. Simultaneously, **CtrlCore**'s mouse handling was refined with the introduction of `NoIgnoreMouse` and `GetCaptureCtrl`, providing more predictable interaction logic for complex UI widgets.

Database and UI components continued their steady climb toward professional excellence. The **PostgreSQL driver** was enhanced with support for **partial indexes**, and the `SqlExp` engine was tuned to handle extreme date limits more robustly. The user interface layer saw a flurry of refinements: **GridCtrl** gained the `GotoFirstEdit` method and improved summary row scrolling, while **ArrayCtrl**'s data retrieval was made more flexible. Visual polish was rounded out by improved **bold font handling** in `CtrlCore`, enhanced filesize formatting in `CtrlLib`, and a community-contributed patch for GTK menu styling, ensuring that U++ applications maintained a modern look and feel on the latest Linux distributions.

## References
- [1] a64ff3cb9 — Core: XmlNode size optimized by 40% (cxl, 2013-06-19)
- [2] 225195c61 — Core: HttpRequest multipart support introduced (cxl, 2013-06-29)
- [3] 756297693 — Docking: Refactored to remove platform-dependent code (cxl, 2013-06-10)
- [4] 8cf647f6b — Core/Skylark: Socket::Listen on specific addresses (cxl, 2013-06-13)
- [5] 7ea60e0be — Postgresql: Partial index support added (cxl, 2013-06-27)
- [6] 34effe325 — CtrlCore: Improved Bold font handling (cxl, 2013-06-26)
- [7] 02d470a84 — CtrlCore: New mouse methods NoIgnoreMouse and GetCaptureCtrl (cxl, 2013-06-10)
- [8] df34a00f0 — GridCtrl: GotoFirstEdit method added (unodgs, 2013-06-05)
- [9] 0b4a38464 — CtrlLib: Improved filesize formatting (cxl, 2013-06-28)
- [10] 2c3ae24cd — CtrlLib: ChGtk menu styling patch (cxl, 2013-06-30)
- [11] 56738c89a — Docking: Fixed tool window resizing issues (cxl, 2013-06-26)
- [12] 425562ef9 — Sql: PostgreSQL Date::Low limited to 1-1-1 (cxl, 2013-06-26)
- [13] ce92a0748 — Core: InVector Remove fixed for empty containers (cxl, 2013-06-04)
- [14] 95a377444 — GridCtrl: Fixed popup for cells containing AttrText (unodgs, 2013-06-05)
- [15] 94419dbd9 — CtrlLib: Calendar and DropDate fixes for month-end clicks (cxl, 2013-06-14)
- [16] 28de6f24c — CtrlLib: ArrayCtrl::GetLine made more flexible (cxl, 2013-06-26)
- [17] 56ad657b5 — ide: SVN integration issues resolved (cxl, 2013-06-13)
- [18] def4f041a — CtrlLib: NotNull support added to DropList (.usc) (cxl, 2013-06-11)
- [19] 12d14fc2e — uppdev: Ongoing core development updates (cxl, 2013-06-03)
- [20] 7efe42e8b — cosmetics: UI and code style refinements (cxl, 2013-06-03)
