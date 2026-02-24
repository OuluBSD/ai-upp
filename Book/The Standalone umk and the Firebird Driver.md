# The Standalone umk and the Firebird Driver
**Date Span:** 2011-08-01 to 2011-08-31

### Standalone umk Make Utility
Launched the "real" **umk** as a standalone, reduced-footprint build tool. Supporting inline assembly, relative paths, and silent modes, this transformation provided a powerful command-line interface for complex U++ assemblies.

### Firebird Database Driver
Introduced the initial version of the **Firebird/Interbase** driver, expanding U++'s native database connectivity suite. This was supported by refinements in `SqlExp` for sorting union results and improved Oracle OCI library detection on POSIX.

### Scientific and Math Consolidation
Promoted **Eigen** to a core plugin (`plugin/Eigen`), streamlining its use for high-performance matrix algebra. The **Controls4U** collection was expanded with `SliderCtrlX` and the `StarIndicator`.

### Documentation and IDE Polish
TheIDE gained **T++ to HTML export**, enabling instant web generation of documentation. The **CodeEditor** was updated to recognize accented characters as identifier letters, and `DropList` received full Unicode support.
