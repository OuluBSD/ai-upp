# The DropTree and the 64-bit Push
**Date Span:** 2009-10-01 to 2009-10-31

### UI Innovation: DropTree
Introduced the `DropTree` control, a powerful hybrid of dropdown selection and hierarchical tree views. This was supported by the `DisplayWithIcon` helper in `Draw` and ergonomics updates to `SliderCtrl` and `HeaderCtrl`.

### Infrastructure Maturity
Steady progress on 64-bit compatibility with targeted typecasts and logic refinements. Core library improvements included robust `FileMapping` error handling in Linux and the addition of `GetUtcTime`.

### Professional Database Layer
`SqlExp` gained `InsertNoKey` for auto-increment fields, while PostgreSQL support reached new heights with `GetInsertedId` and `ISERIAL`. Error diagnostics for shared library loading and OCI8 connections were significantly improved.

### Deployment and TheIDE
Added RPM packaging support via `upp.spec` and improved Debian package building. TheIDE's package selector was updated to show application icons, and the PDB debugger became more context-aware with live variable tooltips.
