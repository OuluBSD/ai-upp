# Localization

## What this covers
This file documents Core's language and translation support: language IDs, runtime language switching, localized formatting metadata, and the `t_()` translation path.

## Translation layer
[`uppsrc/Core/i18n.h`](../../../uppsrc/Core/i18n.h) defines the low-level translation API:

- `t_GetLngString`
- `GetLngString`
- `SetCurrentLanguage`
- `GetCurrentLanguage`
- `LoadLngFile`
- `SaveLngFile`
- `GetLngSet`

The `t_()` helper comes from the included `t_.h` and is intended for string literals. The comments in `i18n.h` are explicit about that cache-oriented assumption.

## Language IDs
[`uppsrc/Core/Lang.h`](../../../uppsrc/Core/Lang.h) defines compact 4-letter language codes through `LNG_(...)` and `LNGC_(...)`, plus helpers such as:

- `LNGFromText`
- `LNGAsText`
- `GetLNGCharset`
- `SetLNGCharset`

This means localization is not only string-table lookup. Language identity also carries charset information.

## Runtime language state
Core supports changing the active language at runtime:

- `SetLanguage(int)` / `SetLanguage(const char*)`
- `GetCurrentLanguage()`
- `GetCurrentLanguageString()`
- `GetSystemLNG()`

The translation implementation in [`uppsrc/Core/t.cpp`](../../../uppsrc/Core/t.cpp) also refreshes date formatting when the current language changes.

## `LanguageInfo`
`LanguageInfo` is the richer locale metadata object. It includes:

- English and native names
- thousand separator and decimal point
- date and time formats
- month/day names
- comparison and indexing-letter callbacks

It also exposes localized formatting helpers such as `FormatInt`, `FormatDouble`, `FormatDate`, and `FormatTime`.

## Semantics and tradeoffs
- translation is literal-ID based, not message-format-resource based
- language/charset info is centralized in Core rather than delegated to OS locale APIs alone
- collation and index-letter logic are pluggable through function pointers in `LanguageInfo`

## Current vs legacy
The translation and language-info layer is current. Some older naming remains, especially around `Lng` functions and compatibility constants, but the subsystem is active and tied directly into formatting and text comparison.

## See also
- [04-Strings-and-Text.md](04-Strings-and-Text.md)
- [12-Time.md](12-Time.md)
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
