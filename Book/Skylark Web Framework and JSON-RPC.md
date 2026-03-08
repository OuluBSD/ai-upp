# Skylark Web Framework and JSON-RPC
**Date Span:** 2012-07-01 to 2012-07-31

### Skylark Framework Launch
Launched the **Skylark** web framework, providing a high-performance, template-driven environment for full-stack C++ web development. The initial release featured a comprehensive tutorial suite and standardized variable notations for sessions and cookies.

### Unified RPC: JSON and XML
The `XmlRpc` package was transformed into the unified **Rpc** suite, adding native **JSON-RPC** support. This enabled U++ applications to interoperate with modern web services using either XML or JSON through a consistent, high-level API.

### Refactored Configuration and INI
The **INI parameter system** was refactored to use double-checked locking, eliminating mutex overhead. New helpers like `INI_DOUBLE` and `INI_INT64` were introduced, along with support for memory-size suffixes (K, M, G, T).

### MacOS Patches and X11 Debugging
TheIDE received critical **MacOS X11 patches**, improving behavior on Apple workstations. Debugging was hardened with the `_DBG_Ungrab` mode for X11, and the Bazaar expanded with browser-like navigation in **HelpViewer** and context menus in **Scatter**.
