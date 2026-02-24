# GUI Locking and the NewDraw Nest
**Date Span:** 2009-05-01 to 2009-05-31

### GUI Multi-threading Breakthrough
Introduced `Ctrl::Lock` and `Ctrl::Call`, providing a robust mechanism for safe UI interaction from non-main threads. This enabled background window management and event loops in secondary threads across Win32 and X11.

### The "NewDraw" Refactor
Intensive development in the `newdraw` nest focused on unifying the core `Draw` hierarchy and optimizing font metrics. `PLATFORM_XFT` was merged into `PLATFORM_X11`, and the `Skulpture` theme was added to the Bazaar.

### TheIDE Navigation and Performance
Launched `Ctrl+Click` for jumping to symbol definitions and a dedicated console for "Find in files" output. Implemented the `PackagePath` cache to eliminate editor key lag during heavy indexing.

### Database and Community Expansion
Unified database scripting with `SqlPerformScript` and achieved full PGSQL compatibility for `SqlBinary`. Community additions included the `GoogleTranslator` library and complete documentation for the `SysInfo` package.
