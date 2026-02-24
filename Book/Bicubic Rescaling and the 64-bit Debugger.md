# Bicubic Rescaling and the 64-bit Debugger
**Date Span:** 2013-04-01 to 2013-04-30

### High-Quality Image Rescaling
Introduced **RescaleBicubic** and **RescaleFilter** to the `Draw` package, enabling professional-grade image scaling within U++ applications. This was integrated into the **IconDes** editor, providing icon designers with bicubic filtering directly in TheIDE.

### Native 64-bit Debugging
Commenced development of the **native Win 64-bit debugger** for TheIDE, marking a significant leap in professional Windows development. The environment's editor also gained sophisticated **replace wildcard transformations** for rapid code refactoring.

### Core Networking and STL Parity
`IpAddrInfo` was upgraded with protocol selection (v4, v6, any), and `HttpRequest` was refined for better redirection handling. Core library containers achieved enhanced **STL compatibility** through the implementation of `iterator_traits`.

### UI Modernization
Standardized the framework with **GetVirtualScreenArea** for multi-monitor support and automated **MouseWheel propagation** to parent controls. `GridCtrl` and `Calendar` received native icons and cursors for a more modern look and feel.
