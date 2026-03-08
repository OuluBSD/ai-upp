# Headless Draw and the NewDraw Merge
**Date Span:** 2009-07-01 to 2009-07-31

### The NewDraw Revolution
A fundamental refactor unified the core `Draw` hierarchy, making the drawing system modular and platform-independent. This effort culminated in a truly "headless" `Draw` implementation, allowing complex graphical operations in non-GUI environments.

### Font Metrics and Documentation
Developed new, platform-independent font metrics for X11, Win32, and POSIX. Conducted an exhaustive documentation drive for the `Draw` package, covering `Image`, `DataDrawer`, and high-level `Drawing` classes.

### Painter and Multi-Threading
The `Painter` package added support for arcs, poly-polygons, and `PEN_*` styles. An alternative MultiThreading package and desktop recording features (`SysInfo`) were added to the Bazaar and community collection.

### Core and Database Refinements
`Vector::Clear` was optimized to aggressively deallocate memory, and support for the Blackfin CPU was landed. The database layer saw significant patches for PostgreSQL and new BLOB support for MySQL.
