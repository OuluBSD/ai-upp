# The Web Core Migration and C++11 Readiness
**Date Span:** 2012-04-01 to 2012-04-30

### Web Integration into Core
A major architectural consolidation moved the entire **Web** package foundation—including `TcpSocket`, `HttpRequest`, and `SSL` support—into the canonical `Core` library. This provided all U++ applications with high-performance, built-in HTTP/HTTPS capabilities.

### C++11 Readiness
Updated the framework's source code to formally support the **C++0x (C++11)** standard. This ensured long-term compatibility with modern compilers and allowed U++ developers to begin utilizing next-generation language features.

### GDB MI2 Debugger Overhaul
The **Gdb_MI2** interface for Linux debugging underwent a massive cleanup, introducing specialized **pretty printers** for `Point`, `Rect`, and `Size` types, and achieving robust multi-threaded thread-stop control.

### WinGL and Data Polish
The **WinGL** backend for Rainbow added perspective view support and timer refinements. Core serialization was hardened with improved **Jsonize** mapping for String keys and better error handling for invalid JSON payloads.
