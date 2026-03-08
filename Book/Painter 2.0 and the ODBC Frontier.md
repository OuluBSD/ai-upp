# Painter 2.0 and the ODBC Frontier
**Date Span:** 2009-02-01 to 2009-02-28

### Painter 2.0 Overhaul
A massive rewrite of the `Painter` package landed, introducing version 2.0. Key features included Bezier curve approximators, path stroking, line dashing, clipping, and the breakthrough addition of subpixel rendering for sharper text and graphics.

### ODBC Connector Arrival
U++ expanded its database support with the first working version of the ODBC connector, enabling cross-platform access to a wide range of enterprise data sources. Targeted fixes for SQLite3 and Oci8 improved Win64 compatibility.

### UI and Tooling Ergonomics
`ArrayCtrl` was refined with cursor restoration after header-sorting and new integrity checks. `Splitter` gained transparency, and GTK Chameleon improved its handling of dark themes. TheIDE updated `IconDes` with definable keys and added support for Unicode files with BOM.

### Platform and Core Stability
Landed initial Win64 fixes across the framework. Implemented specialized rounding for subpixel rendering and optimized curve approximation logic for the new software rasterizer.
