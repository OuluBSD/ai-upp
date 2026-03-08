# SCGI Servers and the Cypher Modularization
**Date Span:** 2010-10-01 to 2010-10-31

### SCGI and Web Performance
Introduced the `ScgiServer` package, enabling high-performance web integration with servers like Nginx. The system transitioned to a virtual-method event model and was supported by the `ScgiHello` reference example and Nginx sample configurations.

### Modular Cryptography: Cypher
Launched the **Cypher** package in the Bazaar, providing a modular, stream-based foundation for encryption. This architecture was immediately utilized to complete the `Protect` package's client/server web authentication schema.

### Core Architectural Maturation
`String::Replace` was introduced, and `Tuple` reached full utility as a natively `Moveable` container. `UrlDecode` was upgraded with unicode escape support, and `ValueArray`/`ValueMap` gained native `Xmlize` capabilities.

### UI Interactivity and Tooling
`Scatter` and `PlotterCtrl` received mouse-wheel zoom and dynamic cursor feedback. TheIDE's editor added `Shift+MouseWheel` for horizontal scrolling, and the build system added support for Ubuntu 10.10 and Visual Studio 2010.
