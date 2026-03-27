# Strings And Text

## What this covers
This file describes `String`, `WString`, Unicode/charset helpers, and the text-conversion rules that Core makes available across platforms.

## `String`
`String` is defined in [`uppsrc/Core/String.h`](../../../uppsrc/Core/String.h) and implemented in `String.cpp` plus `AString.hpp`.

Observed storage tiers:

- small tier: up to 14 bytes inline in the 16-byte object
- medium tier: heap-backed 32-byte block
- large tier: reference-counted heap storage with stored allocation size

This is not guesswork: `String0` stores `KIND = 14`, `SLEN = 15`, and comments say small strings are "up to 14 bytes". The code distinguishes `SMALL`, `MEDIUM`, and refcounted large forms explicitly.

Practical consequence: `String` is cheap for short ASCII/UTF-8 fragments and also efficient to copy when it holds shared large data.

## `WString`
`WString` is a different implementation, not just `String` with a wider character type.

Key facts from code:

- internal character type is `wchar`, and `Defs.h` defines `wchar` as `uint32`
- `WString` therefore stores 32-bit code points internally
- `WString` does not have inline small-string storage
- `WString0::SMALL = 23` is a minimum heap allocation size, not an inline threshold

`WString.cpp` also shows explicit bridges to and from `std::wstring`, adapting to platform `wchar_t` width:

- if `wchar_t` is 4 bytes, conversion is direct
- if `wchar_t` is 2 bytes, it converts through UTF-16 helpers

## Charset and Unicode support
Relevant files:

- [`uppsrc/Core/CharSet.h`](../../../uppsrc/Core/CharSet.h)
- [`uppsrc/Core/Utf.hpp`](../../../uppsrc/Core/Utf.hpp)
- [`uppsrc/Core/Utf.cpp`](../../../uppsrc/Core/Utf.cpp)
- `UnicodeInfo.cpp`, `Bom.cpp`

Core supports:

- UTF-8 and UTF-16/UTF-32 conversion paths
- system-charset conversion for OS boundaries
- language/locale-aware formatting and collation helpers in `Lang.*`
- BOM handling and code-point classification utilities

The package does not assume the platform native encoding is the framework encoding. Conversion helpers exist because file paths, environment variables, and OS APIs still differ by platform.

## Text semantics and tradeoffs
- `String` is byte-oriented and commonly used for UTF-8 text
- `WString` is for code-point-oriented text, locale helpers, and places where 32-bit internal representation is useful
- the split is explicit; Core does not try to hide every encoding choice behind one string class

This is central code, not a legacy corner.

## See also
- [05-Paths-and-Config.md](05-Paths-and-Config.md)
- [06-Streams.md](06-Streams.md)
- [12-Time.md](12-Time.md)
- [README.md](README.md)
