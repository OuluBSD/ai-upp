# GTK Maturity and the REST of Core
**Date Span:** 2013-01-01 to 2013-01-31

### GTK Backend Milestone
The **Rainbow GTK backend** achieved professional parity with the addition of native **TrayIcon**, global **SystemHotKey** handling, and **SetSurface** rendering. It also gained robust support for file Drag & Drop and frame extent calculations.

### RESTful Networking and JSON
`HttpRequest` was upgraded to support the full range of REST methods (HEAD, DELETE, etc.), and `Jsonize` was refined to handle large integers with JavaScript-compliant precision. Core numerical accuracy was boosted to 16 digits.

### UI and Memory Optimization
`FileSel` now allows multiple directory selection in `SelectDir` mode. `Vector::Insert` was optimized to reduce its memory reserve, and `RealizeDirectory` was hardened for network path reliability.

### Framework Stability
Landed critical multi-threading fixes for X11 and GTK backends. The **Skylark** web framework was refined with session and identity fixes, and **TcpSocket** achieved better BSD compatibility.
