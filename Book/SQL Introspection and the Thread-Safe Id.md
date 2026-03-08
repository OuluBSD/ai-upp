# SQL Introspection and the Thread-Safe Id
**Date Span:** 2011-12-01 to 2011-12-31

### Automatic UI from SQL Schemas
Introduced **SQL Introspection**, allowing the framework to query its own database definitions at runtime. This enabled the new `SqlCtrls` suite to automatically generate and bind UI editors based on schema metadata, supported by the new `JoinRef` utility.

### TLS-Optimized Identity System
The core **Id** class was completely refactored to be `String`-based and optimized for multi-threading using Thread-Local Storage (TLS). This deep architectural shift improved performance and safety for concurrent applications.

### TheIDE Error Awareness and Hydra
TheIDE gained visual **error highlighting** for problematic files and packages in the project browser. The Hydra build engine's thread limit was increased to 64, and custom build steps were enhanced with the `$(FILEDIR)` macro.

### Core Performance and Bazaar
Landed the `force_inline` macro and further optimizations for `String::Cat`. The Bazaar saw the addition of the `StarIndicator` to **Controls4U** and refinements to the **XMLMenu** system, while the official site launched a new language selector.
