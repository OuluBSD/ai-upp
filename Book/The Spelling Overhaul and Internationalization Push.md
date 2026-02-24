# The Spelling Overhaul and Internationalization Push
**Date Span:** 2009-11-01 to 2009-11-30

### Modern Spelling Infrastructure
Overhauled the `RichEdit` spellchecker with a new `.scd` format and the `MakeSpellScd` utility, significantly reducing memory usage. Added UTF-8 support and introduced the `.udc` user dictionary format.

### Repository-wide Format Migration
Implemented mandatory escaping of >127 characters in `.t` (translation) files to ensure cross-platform source portability. Migrated every `.iml` (image list) file in the entire repository to a new, more efficient format.

### UI and Database Ergonomics
Introduced multiline popups and the `Select*` utility family for file selection. `SqlExp` mappings (`S_` structures) became natively comparable, and Assist++ gained support for PostgreSQL-specific auto-increment columns.

### Connectivity and Core
Launched the `Urr` (UDP Request-Response) package in the Bazaar. Refined `CParser` with parameter-less error throwing and landed critical stability fixes for `FileMapping`. Added string trimming utilities (`TrimLeft`, `TrimRight`, `TrimBoth`) to `EditString`.
