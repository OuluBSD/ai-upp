# The FileSel Places and XML-RPC Refinement
**Date Span:** 2010-03-01 to 2010-03-31

### FileSel Modernization
Standardized the `FileSel` dialog with a new "places" bar for common folders. This was supported by robust cross-platform detection of Downloads, Media, and Home directories across Win32 and X11.

### XML-RPC and PolyXML Maturity
`XmlRpc` gained a short-circuit mode and refined error handling, while `PolyXML` added support for class grouping and internationalized descriptions. `XmlNode` was updated for multiline strings and `XmlParser` for non-destructive peeking.

### Core and Platform Unicode
`CtrlCore` achieved native Unicode support for command-line and environment access on Windows. Multi-threading saw critical fixes for X11 panic handling and the use of thread-safe system time variants in POSIX.

### UI Interactivity and Bazaar
The `Scatter` package received mouse zoom and scroll capabilities. `RichEdit` began supporting image file drops, and `TabCtrl` was refined with slave-based management methods. The framework expanded its global reach with new Spanish and Italian translations.
