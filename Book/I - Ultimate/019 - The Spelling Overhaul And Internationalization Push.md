# The Spelling Overhaul and Internationalization Push (2009-11)
**Date Span:** 2009-11-01 to 2009-11-30

November 2009 was a month of significant internal data format modernization and a major push toward robust internationalization. The spelling checker subsystem in `RichEdit` underwent a complete transformation. A new `.scd` (spelling checker data) file format was introduced, accompanied by the `MakeSpellScd` utility to generate them. This new format dramatically reduced both the on-disk size of dictionary files and the runtime memory footprint of the application. Furthermore, `RichEdit` was updated to support a UTF-8 based format, and the system began prioritizing the new `.udc` user dictionary format, signaling a cleaner separation between system and user data.

The framework's commitment to global compatibility was further demonstrated by a massive conversion of translation (`.t`) and image list (`.iml`) files. A new saving policy for `.t` files was implemented in TheIDE, which now automatically escapes characters with codes greater than 127. This ensured that translation sources remained portable across different text editors and platforms without risk of encoding corruption. Simultaneously, every `.iml` file in the repository—spanning core libraries, examples, and tutorials—was migrated to a new, more efficient format.

The user interface layer gained several sophisticated new capabilities. `CtrlLib` introduced multiline popups that leverage the `GridDisplay` system for rich content rendering, and the `FileSel` family was expanded with new utility functions like `SelectFile` and `Select*` variants. `TreeCtrl` was enhanced to return the IDs of newly inserted nodes during drag-and-drop operations, and `EditString` gained native support for `TrimLeft`, `TrimRight`, and `TrimBoth`. TheIDE also became smarter about database-driven projects, with Assist++ now recognizing PostgreSQL-specific `SERIAL` and `ISERIAL` columns in schema files.

On the architectural and networking fronts, the Bazaar welcomed the `Urr` (UDP Request-Response) package, providing a lightweight alternative to TCP-based protocols for simple command-and-control tasks. The database layer reached a new milestone in developer ergonomics: `S_` structures (SQL row mappings) became natively comparable using the `==` and `!=` operators, and the `Sql::InsertNoNulls` helper was added to simplify the creation of dense records. Core stability was also reinforced with critical fixes to `FileMapping` and the addition of a parameter-less `ThrowError()` method to `CParser`, streamlining error handling in custom DSLs.

## References
- [1] 7719c8097 — RichEdit: New .scd speller format with reduced footprint (cxl, 2009-11-10)
- [2] b0a293994 — RichEdit: UTF-8 support; MakeSpellScd utility (cxl, 2009-11-02)
- [3] ea2c2f305 — theide: .t files now escape >127 characters for portability (cxl, 2009-11-15)
- [4] d316aaed3 — uppsrc: All .iml files migrated to new format (cxl, 2009-11-16)
- [5] c86b1c3a1 — Added multiline popups using GridDisplay (unodgs, 2009-11-01)
- [6] b333a94ee — CtrlLib: FileSel based Select* utility family (cxl, 2009-11-02)
- [7] 4929b370c — bazaar: Added Urr (UDP request-response) package (cxl, 2009-11-20)
- [8] 542146f13 — theide: Assist++ recognizes PGSQL SERIAL columns (cxl, 2009-11-10)
- [9] 00b3c57f1 — Sql: S_ structures now == and != comparable (cxl, 2009-11-29)
- [10] 0abacbd83 — Sql: Added Sql::InsertNoNulls (cxl, 2009-11-27)
- [11] 6ec88ac62 — Core: FileMapping fixes for write flag and size (rylek, 2009-11-15)
- [12] 827cfb592 — CtrlLib: TrimLeft/Right/Both for EditString (cxl, 2009-11-29)
- [13] 69e9d692e — CtrlLib: TreeCtrl InsertDrop returns inserted node IDs (cxl, 2009-11-12)
- [14] e070bbed2 — Core: CParser::ThrowError() without parameters (cxl, 2009-11-25)
- [15] 905c17ae6 — RichEdit: StyleManager made public (cxl, 2009-11-07)
- [16] 37753b4ad — Popups: Summary updates on cell value changes (unodgs, 2009-11-09)
- [17] c39ab328b — GeomCoords: fixed charset problem in FormatDegree (cxl, 2009-11-19)
- [18] 4c7db817d — CtrlLib: HeaderCtrl serialization fixed (cxl, 2009-11-18)
- [19] f45e57ac9 — reference: New Socket examples (cxl, 2009-11-15)
- [20] 94c9a2c76 — RichEdit: Signal arrow for labeled paragraphs (cxl, 2009-11-09)
