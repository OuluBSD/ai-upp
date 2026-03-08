# Skylark Uploads and fork-safe Random
**Date Span:** 2013-08-01 to 2013-08-31

### Skylark Progress Handling
Launched native **progress handling** for the Skylark web framework, enabling real-time tracking of long-running tasks like multi-file uploads. This was showcased in the new `SkylarkUpload` example, which transitioned from static maps to session-based progress tracking.

### Core Fork-Safety and Randomness
Introduced a **fork-safe Random** number generator, ensuring unique seeds across child processes in multi-process environments. This was supported by the `SeedRandom` utility and new `SortByKeys/Values` algorithms for associative data.

### Tooling and C++11 Highlighting
TheIDE added syntax highlighting for **C++11 keywords** and formal support for the **Visual Studio 2013 (MSC12)** toolchain. The Layout Designer was upgraded with the ability to sort items by their visual screen position.

### SQL and UI Hardening
`S_*` table structures gained a dynamic `Get(column)` method, and `ColumnList` was improved to correctly update scrollbars on item resize. `HttpRequest` was hardened to always send `Content-Length`, and `Socket::Connect` began providing detailed error descriptions.
