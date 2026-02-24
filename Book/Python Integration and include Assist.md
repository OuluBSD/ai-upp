# Python Integration and include Assist
**Date Span:** 2011-04-01 to 2011-04-30

### Hosted Python and BOOSTPY
Matured the **Py** integration in the Bazaar, leveraging `Boost.Python` to export U++ object models to a hosted scripting environment. Introduced the `PyConsoleCtrl` with execution history and specialized converters for `String`, `Value`, and `ValueArray`.

### Tooling: include Assist
Launched the **#include Assist** feature in TheIDE, providing intelligent path autocompletion for preprocessor directives. Supporting both system and local headers, this addition significantly improved navigation across complex project hierarchies.

### Professional UI Refinements
`TabCtrl` gained support for QTF (rich text) labels, and `LineEdit` introduced a "ShowSpaces" mode for whitespace-sensitive editing. The `TrayIcon` system was hardened to automatically recreate itself after Windows Explorer crashes.

### Core and Build Stability
Ensured full compatibility with GCC 4.6 and refined the GCC builder's library ordering logic. Core containers (`Array`, `Vector`) continued their transition to reference-returning APIs for `Set` and `Insert` methods.
