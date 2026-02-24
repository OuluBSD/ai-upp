# The Headless Build and Modular Core
**Date Span:** 2009-04-01 to 2009-04-30

### Headless umk and Build Separation
TheIDE's build logic was formally separated from the GUI (`ide/core`), enabling a truly headless version of the U++ make utility (`umk`). This allowed for seamless integration into automated CI/CD environments.

### MSSQL and Database Growth
Launched the dedicated `MSSQL` package based on ODBC and introduced `SqlBinary` support. Maturation of the database layer was supported by exhaustive documentation for `Path.h` and `LocalProcess.h`.

### UI Mergers and SysInfo
Merged `MultiList` into `ColumnList` and added stacking/sorting to `TabBar`. The `SysInfo` package received significant Linux updates for window/mouse handling and screen capture.

### TheIDE Context Awareness
Enhanced `Alt+J` to jump directly to the layout designer and added variable value tooltips to the PDB debugger. Extended platform support to OpenBSD and NetBSD, and introduced Debian 64-bit nightly builds.
