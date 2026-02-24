# The DropTree and the 64-bit Push (2009-10)
**Date Span:** 2009-10-01 to 2009-10-31

October 2009 was a month of refinement and infrastructure readiness for U++. The most visible addition to the UI toolkit was the introduction of `DropTree`, a combination of a dropdown list and a tree-view control, providing a powerful way to select hierarchical data in a compact space. This was accompanied by the `DisplayWithIcon` helper in the `Draw` package, simplifying the common task of rendering text paired with standard iconography. The `SliderCtrl` also saw significant ergonomic updates, including support for reversed orientation, improved focus visualization, and better integration with the framework's focus management.

Deep within the framework, a steady push toward 64-bit maturity continued. Several modules received targeted typecasts and logic adjustments to simplify 64-bit compilation across various compilers. The `Core` library saw improvements to `FileMapping`, correcting a Linux-specific bug where `mmap` error codes were being checked incorrectly. Time handling became more robust with the addition of `GetUtcTime` and specialized `Min`/`Max` methods for time conversions, ensuring that date and time widgets could propagate default values more effectively.

The database layer continued its rapid evolution. `SqlExp` gained `InsertNoKey`, a vital feature for handling tables with auto-incrementing primary keys. PostgreSQL support reached a new milestone with `GetInsertedId` support for tables using standard 'id' keys and the introduction of `ISERIAL` for integer-based serial columns. Diagnostics for database connections and shared library loading in Linux were also overhauled, providing developers with much clearer error messages when OCI8 (Oracle) or other external dependencies failed to load.

TheIDE and its surrounding deployment scripts were further refined for the professional developer. The package selector began displaying application icons, providing instant visual recognition for complex assemblies. Build infrastructure matured with the introduction of `upp.spec` for RPM-based Linux distributions and updates to the Debian package builder. Productivity was bolstered by HeaderCtrl column tooltips, multiline commit message support in SVN, and a specialized `InstallPanicMessageBox` for Windows console applications—ensuring that command-line tools reported errors to `stderr` rather than popping up blocking UI dialogs.

## References
- [1] b49c6731a — CtrlLib: DropTree introduced; Draw: DisplayWithIcon (cxl, 2009-10-02)
- [2] 801e583ad — reference: DropTree example (cxl, 2009-10-02)
- [3] f47f4abb7 — CtrlLib: SliderCtrl reversed orientation and focus polish (cxl, 2009-10-26)
- [4] 43a5c5396 — Typecasts to enable/simplify 64-bit compilation (rylek, 2009-10-22)
- [5] e73dab8d0 — Linux: FileMapping mmap error check fix (rylek, 2009-10-14)
- [6] 15d76fac5 — Core: GetUtcTime added (cxl, 2009-10-22)
- [7] 80a2b8472 — Core/CtrlLib: ConvertDate/Time propagate defaults to widgets (cxl, 2009-10-19)
- [8] 4616fc171 — SqlExp: InsertNoKey for auto-increment fields (cxl, 2009-10-14)
- [9] 7c3d4af14 — PostgreSQL: GetInsertedId support for 'id' keys (cxl, 2009-10-20)
- [10] 43a9e82dd — PGSQL: ISERIAL support (cxl, 2009-10-28)
- [11] 4ab331024 — Improved SO load and OCI8 connection diagnostics (rylek, 2009-10-20)
- [12] 91ae47508 — ide: Package selector shows app icons (cxl, 2009-10-10)
- [13] f42a71617 — uppbox: added upp.spec for RPM support (cxl, 2009-10-11)
- [14] 25d08a5b4 — HeaderCtrl: column tooltips added (rylek, 2009-10-26)
- [15] 12a46dee6 — Windows: Console panic messages redirected to stderr (rylek, 2009-10-26)
- [16] 86d4ce73a — Report: Select printer before rendering; paper size awareness (cxl, 2009-10-28)
- [17] 1693158ce — Core: Array::AppendPick fixed (cxl, 2009-10-07)
- [18] 021bbdfcf — .rpm packaging fixes (cxl, 2009-10-25)
- [19] 970c35fce — Core/ide: Fixed non-ASCII characters in environment (cxl, 2009-10-09)
- [20] e211a2903 — Web: AttachSocket implementation (cxl, 2009-10-02)
