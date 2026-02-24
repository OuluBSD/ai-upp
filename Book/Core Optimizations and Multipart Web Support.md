# Core Optimizations and Multipart Web Support
**Date Span:** 2013-06-01 to 2013-06-30

### XmlNode Optimization
Achieved a major peak in core efficiency with a **40% reduction in XmlNode memory footprint**. This internal refactor significantly improved the framework's ability to handle large XML-based documents and data structures.

### Advanced Web: Multipart and Multi-IP
`HttpRequest` gained native **multipart support** for complex form handling and file uploads. The networking core was also enhanced to support **listening on specific IP addresses**, providing finer control for multi-homed server deployments.

### Platform-Agnostic Docking
The **Docking** package was re-engineered to remove platform-dependent code, ensuring consistent behavior across Win32, X11, and Rainbow backends. This refactor also resolved long-standing docked-window resizing issues.

### SQL and UI Hardening
Landed support for **partial indexes in PostgreSQL** and improved bold font handling in `CtrlCore`. The user interface toolkit was refined with better filesize formatting, GTK menu styling, and enhancements to the `GridCtrl` editing workflow.
