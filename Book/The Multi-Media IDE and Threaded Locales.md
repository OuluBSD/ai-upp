# The Multi-Media IDE and Threaded Locales
**Date Span:** 2012-06-01 to 2012-06-30

### Multi-Media IDE Workbench
Transformed TheIDE into a comprehensive media workbench with native viewing support for PNG, JPG, GIF, and BMP files, along with built-in PNG editing. Expanded syntax highlighting to include Javascript, CSS, and C#, and added "Save on deactivation" for better context switching.

### Threaded Locales and Core Concurrency
Re-engineered the core library to support **per-thread language settings**, essential for multi-threaded internationalized servers. `Thread` was hardened to catch `Exc/ExitExc` exceptions, and `Thread::Priority` was implemented for POSIX platforms.

### Declarative Configuration: INI Helpers
Introduced the **INI_BOOL**, **INI_STRING**, and **INI_INT** macros for standardized, declarative configuration management. This was accompanied by the `LOG_` conditional logging helper to streamline diagnostic output.

### Database Stability and Tcc
Landed `WhenReconnect` for MySQL and hex blob support for PostgreSQL. The **Tcc** (Tiny C Compiler) integration reached a milestone by enabling native cross-instance calls for complex runtime-compiled logic.
