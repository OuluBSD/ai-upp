# Persistent Tabs and the SQL Renaissance (2009-06)
**Date Span:** 2009-06-01 to 2009-06-30

June 2009 was a month of consolidation and educational outreach for U++. The framework's database capabilities took center stage with the completion of a comprehensive SQL tutorial, while `SqlCommander` gained native support for PostgreSQL. This "SQL Renaissance" was supported by refinements to `SqlBinary`—which now correctly handled NULL for empty values—and the removal of obsolete internal implementation folders, signaling a cleaner, more stable database API.

TheIDE received several "sticky" productivity upgrades. File tabs became persistent across sessions, a change that significantly reduced friction when resuming work on complex projects. The SVN and `usvn` integration matured further with support for multiline commit messages and improved error reporting when external binary dependencies were missing. A major new feature was the ability to import an entire directory source tree into a package, automating what was previously a tedious manual task. Assist++ also extended its reach to schema files (`.sch`), bringing intelligent navigation and autocomplete to database definitions.

Graphical and UI components continued their upward trajectory. The `MultiList` package was promoted to the core `uppsrc` collection, and `TabBar` received major internal improvements, including fixed separator painting and better integration with the `Docking` framework's auto-hide logic. `RichText` underwent a significant architectural shift: painting was updated to use `RichObject::ToImage` primarily, effectively resolving long-standing transparency issues with embedded objects. A new 'text' rich object format was also introduced to broaden the scope of embedded content.

Linux and X11 stability remained a priority. Developers grappled with complex focus issues related to SCIM and XIM input methods, while also resolving visual artifacts in Drag & Drop operations under Compiz. The core library saw its own share of refinements, including a fix for `DirectoryExists` on Win32 drives and the standardization of CRLF pairs in XML output. The Bazaar continued to flourish with the addition of `BlueBar` and extensive updates to the `Skulpture` theme, while the `SysInfo` package gained more low-level hardware introspection functions.

## References
- [1] 2a3a18f42 — SQL tutorial finished (cxl, 2009-06-03)
- [2] e777f8dfd — Added PostgreSQL support to SqlCommander (rylek, 2009-06-02)
- [3] 725d07ba5 — theide: FileTabs now persistent (cxl, 2009-06-07)
- [4] 332ca4916 — theide svn: Support for multiline commit messages (cxl, 2009-06-07)
- [5] 43bc638dd — theide: Import directory source tree into package (cxl, 2009-06-15)
- [6] b7e96fd56 — MultiList promoted to uppsrc; TabBar improvements (mrjt, 2009-06-04)
- [7] c7196a682 — RichText: Painting changed to use RichObject::ToImage (cxl, 2009-06-24)
- [8] eeaff3e93 — RichText: Added 'text' rich object format (cxl, 2009-06-02)
- [9] cb33f077e — Assist++ now supports .sch files (cxl, 2009-06-09)
- [10] d6746e185 — CtrlCore: SCIM/XIM focus and position fixes (cxl, 2009-06-13)
- [11] 6161d0e0f — X11: Fixed Compiz artifacts in Drag & Drop (cxl, 2009-06-14)
- [12] a79b868d0 — Core: DirectoryExists fixed for Win32 drives (cxl, 2009-06-08)
- [13] 0d7107360 — XML now produces CRLF pairs (cxl, 2009-06-09)
- [14] f2680f641 — theide: exclamation if no svn binary found (cxl, 2009-06-03)
- [15] 4816b5c3e — Sql: NULL for empty SqlBinary; Painter: fixed DrawImageOp (cxl, 2009-06-05)
- [16] c27fa008a — SysInfo: added functions and updated doc (koldo, 2009-06-06)
- [17] 74b33fa4c — Skulpture updates; BlueBar added to Bazaar (cbpporter, 2009-06-25)
- [18] 401e0b7e3 — theide: saving file now keeps access rights in POSIX (cxl, 2009-06-22)
- [19] b256eeeba — EditField::NullText can now have an icon (cxl, 2009-06-25)
- [20] 9ec845ba3 — Progress display enhancements in the login dialog (rylek, 2009-06-29)
