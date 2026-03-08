# XML-RPC, Functions4U, and the Linker Push
**Date Span:** 2009-12-01 to 2009-12-31

### RPC and High-level Utilities
Launched the `XmlRpc` package for standardized remote procedure calls. The Bazaar saw a major expansion with the `Functions4U` and `Controls4U` packages, aggregating dozens of practical helpers and UI controls.

### Build Optimization: gc-sections
TheIDE and release scripts were updated to support GCC `--gc-sections`, enabling significantly smaller binary sizes by discarding unused code. New build method options provided finer control over the linking process.

### Core and System Refinements
Completely refactored `LanguageInfo` for better locale management. Added Win32 service protocol support and improved `PdfDraw` compatibility. `SysInfo` achieved keyboard/mouse feature parity on Linux.

### UI and Tooling Ergonomics
Exposed and made overridable the `EditField` context menu. `Splitter` gained a `Remove` method, and `ValueMap` was enhanced with String key variants. TheIDE's auto-setup was updated for Win32 SDK 7.0.
