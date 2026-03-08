# The Great Web Consolidation and Skylark Expansion
**Date Span:** 2012-09-01 to 2012-09-30

### Removal of legacy Web package
Formally removed the legacy `Web` package from `uppsrc` and TheIDE, finalizing the migration of networking primitives to the `Core` library. This prompted a framework-wide update of examples and references to utilize the modernized `TcpSocket` and `HttpRequest` APIs.

### Skylark Modularization: Pack
Launched **SkylarkPack**, a system for grouping library handlers into reusable modules, alongside a new advanced tutorial (`Skylark12`). A comprehensive deployment guide was added to support professional Skylark production environments.

### Core Efficiency and SQL Bulk
Hardened `TcpSocket::Accept` against preforking issues and optimized `MassInsert` for native bulk performance in PostgreSQL and MySQL. The `CParser` lexer was refactored to use exceptions for better resilience in development tools.

### IDE Transparency and UI Polish
TheIDE published formal documentation for the **.upp package format** and added support for **pkg-config** in release scripts. `EditField` gained the **WhenPasteFilter** callback, and `MenuBar` was refined to prevent owner-specific menu duplication.
