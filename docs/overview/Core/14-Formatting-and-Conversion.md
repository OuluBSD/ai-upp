# Formatting And Conversion

## What this covers
This file explains Core's formatting and scanning helpers, the `Convert` family, and the formatter registry used by `Format(...)`.

## Formatting layer
[`uppsrc/Core/Format.h`](../../../uppsrc/Core/Format.h) and [`uppsrc/Core/Format.cpp`](../../../uppsrc/Core/Format.cpp) provide:

- integer formatters such as `FormatIntBase`, `FormatIntHex`, `FormatIntRoman`
- floating-point formatters such as `FormatE`, `FormatF`, `FormatG`, `FormatDouble`
- date/time formatting with `FormatDate` and `FormatTime`
- printf-style formatting through `Format(...)`, `VFormat(...)`, and `Sprintf(...)`

The formatter system is registry-based. `RegisterFormatter(...)` installs handlers per value type and format ID, and `Format.cpp` registers a large default set lazily in `sRegisterFormatters()`.

## Semantics
`Format(...)` is not just `printf` text substitution. It operates on `Vector<Value>` and can dispatch by runtime `Value` type plus formatter id.

That is why the same formatting pipeline can handle:

- integers and floats
- strings and wide strings
- `Date` and `Time`
- `Value`-carried rich types through their string form or registered formatter

## Conversion layer
[`uppsrc/Core/Convert.h`](../../../uppsrc/Core/Convert.h) and [`uppsrc/Core/Convert.cpp`](../../../uppsrc/Core/Convert.cpp) provide the scan/validate side.

There are two levels:

- low-level scanners such as `ScanInt`, `ScanInt64`, `ScanDouble`, `ScanFloat`
- object-style converters such as `ConvertInt`, `ConvertDouble`, `ConvertDate`, `ConvertTime`, `ConvertString`, `MapConvert`, `JoinConvert`, and `FormatConvert`

The `Convert` base API is explicit:

- `Format(const Value&)`
- `Scan(const Value&)`
- `Filter(int chr)`

That makes converters suitable both for parsing and for character-by-character UI validation.

## Validation behavior
The typed converters encode range and nullability constraints in code:

- `ConvertInt` and `ConvertInt64` enforce min/max and optional `NotNull`
- `ConvertDouble` tracks decimal-comma behavior and pattern formatting
- `ConvertDate` and `ConvertTime` validate ranges and defaults
- `ConvertString` enforces max length and trimming rules

Validation errors are represented as `ErrorValue(...)`, not exceptions, in normal converter workflows.

## Localization interaction
Formatting and conversion are language-aware in places:

- `Format(int language, ...)` exists directly in `Format.h`
- `ConvertDouble` detects decimal-comma behavior from localized formatting
- date and time formatting rely on language data and current-language settings

## Current vs legacy
This is a current and central subsystem. Some convenience wrappers are marked deprecated in headers, but the formatter registry and `Convert` classes are active Core infrastructure.

## See also
- [12-Time.md](12-Time.md)
- [13-Value.md](13-Value.md)
- [17-Localization.md](17-Localization.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
