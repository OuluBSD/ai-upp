# Small Value Optimization and the HTTP Server
**Date Span:** 2012-05-01 to 2012-05-31

### The Full HTTP Stack
Introduced the **HttpResponse** class and the **HttpServer** reference example, completing U++'s native web stack. This enabled the creation of standalone web services with built-in SSL and cookie support, further hardened by the `XmlizeByJsonize` bridge and improved SCGI handling.

### Core Modernization and Diagnostics
Officially **dropped support for MSC 7.1**, enabling a cleaner, more modern C++ codebase. Core library diagnostics were bolstered with `LOGHEX`, `DUMPHEX`, and `AsString(T*)` for pointer logging, while `TcpSocket` gained a global timeout setting.

### Professional UI Persistence
`Font`, `Image`, and `Painting` types were updated to natively support **Xmlize** and **Jsonize**, allowing for the easy serialization of complex graphical states. The **Scatter** package was promoted to canonical status, and `NoLayoutZoom` was fixed for high-DPI reliability.

### Bazaar and Tooling
Launched the **FSMon** (File System Monitor) package for real-time change tracking and achieved OpenCV compatibility in **Controls4U**. TheIDE gained "Insert as C string" and published the formal specification for the **.upp package format**.
