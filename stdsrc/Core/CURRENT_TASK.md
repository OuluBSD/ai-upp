STL-Backed Core API — Status & Roadmap

Objective
- Provide a U++ Core-compatible developer interface in `stdsrc/Core`, implemented with the C++ standard library (and small shims) so agents and tests can run portably while keeping familiar U++ names and patterns.

Current Status (2025-09)
- Phase 1 + 2 complete; Core is broadly usable for agent work and tests.
- Aggregator, strings, containers, path/streams/time-date, formatting, conversion, JSON/XML, Base64 are implemented with an emphasis on practical source compatibility.

Implemented (files)
- Aggregator & Infrastructure
  - `Core.h`: centralizes system includes; wraps headers in `namespace Upp`; defines platform macros: `PLATFORM_WIN32` (Windows) or `PLATFORM_POSIX` (others) and `flagPOSIX`; path macros `DIR_SEP` and `DIR_SEPS`.
  - `Core.upp`: lists headers (BLITZ-friendly).
  - `Defs.h`: base typedefs (`byte`, `word`, `dword`, `qword`, `int16`, `int64`, `wchar`, `char16`), `Nuller`, markers, `pick`, `ASSERT`.

- Strings & Containers
  - `String.h`, `WString.h`: wrappers over `std::string`/`std::wstring`. Conversions:
    - Windows: `MultiByteToWideChar/WideCharToMultiByte (UTF-8)`.
    - POSIX: `mbsrtowcs/wcsrtombs` with fallback manual UTF‑8 codec.
  - `Vcont.h` (`Vector<T>`), `Index.h` (unique set; `std::map` index), `Map.h` (`VectorMap<K,T>` preserving order).
  - `One.h` (unique ownership), `Ptr.h` (non-owning ptr).
  - `Hash.h`: `hash_t`, `CombineHash`, `GetHashValue` specializations.
  - `Uuid.h`: v4 generator, `ToString()`, `TryParse()`.
  - `Value.h`: minimal `std::any` variant with basic conversions and `ToString()`.

- Utilities
  - `Util.h`: `AsString`, `FormatPtr`.
  - `Convert.h`: `Atoi/Atof`, `ScanInt/ScanInt64/ScanDouble` (comma support), `FormatInt/FormatDouble`; `ConvertInt/Int64/Double/Float/Date/Time` shims (min/max/default/notnull, `Filter`).
  - `Format.h`: U++-style `Format` covering provided examples:
    - Positional `%2:s`, next-arg `%` default.
    - Flags: `0 +  ,  # ! ^ & _ ?` and space.
    - Width `N` and dynamic `*`; alignment `< > =`.
    - Integer `%d %i %o %x %X`, char `%c`.
    - Float `%e %E %f %g %G`.
    - `%m` (and `%M`) with `%.p` precision, `f/e/E` suffixes, exponent tweaks (`^`, `&`), comma, `# ! _ ?` behavior.
    - Keywords: `month/mon`, `day/dy`, `tw` (12 wrap with `%0tw`).
    - Roman `%r/%R`, letters `%a/%A`.
    - Mapping `%[k:text;...;default]s` and null mapping `%[text]~d`.
    - Suffix via backtick: `%d\`pt` → `123pt` (backtick is not printed).
  - `Base64.h`: buffer + Stream Base64 encode/decode.

- Time, Files & Paths
  - `TimeDate.h`: `Date`, `Time`, arithmetic, `Format(Date/Time)`, `ScanDate/ScanTime` (ISO-centric), timezone helpers (`GetTimeZone*`).
  - `Stream.h`: `Stream`, `MemoryStream`.
  - `FileStream.h`: file-backed `Stream` (read/write/seek/tell/size) via `std::fstream`.
  - `TextIO.h`: `TextReader`/`TextWriter`, `LoadFileAsString`, `SaveFileAsString`.
  - `Path.h`: `std::filesystem`-based path toolkit: join/normalize, pattern match, directory create/delete, temp paths/files, symlinks, attributes (read/write/exec/hidden), `GetFileOnPath`, file times (with conversions), timezone text parsing.

- Data
  - `JSON.h`: minimal JSON value (NUL/BOOL/NUMBER/STRING/ARRAY/OBJECT), parse/serialize, pretty-print, path access (`Ptr`), JSON Pointer (`Pointer`, `PointerSet`, `PointerRemove`).
  - `XML.h`: simple DOM: attributes, namespaces (prefix + `xmlns`), CDATA-aware parsing, pretty-print.

Platform behavior
- Macros: `PLATFORM_WIN32` vs `PLATFORM_POSIX`, `flagPOSIX` (non-Windows). Always define `DIR_SEP` and `DIR_SEPS`.
- UTF‑8/wide conversions:
  - Win32: `CP_UTF8` APIs — safe and locale-independent.
  - POSIX: try CRT multibyte with current locale; fallback to locale-independent UTF‑8 codec to avoid mojibake.

Deviations vs U++ Core (intentional)
- Containers: `Index<T>` omits “unlinked” behavior; `VectorMap` uses a map index plus ordered vector (no hash-table specific APIs).
- `Value`: `std::any`-based; only common types/formats supported.
- Streams: small subset (memory/file); no filter chain beyond Base64 helpers.
- Path: uses `std::filesystem`; platform attribute semantics (e.g., executable on Windows inferred from extension) differ from U++.
- JSON/XML: pragmatic subset; strict edge cases are not guaranteed.
- Format `%m`: matches provided examples and general intent; not byte-for-byte identical to all legacy edge cases.

Tests (stdtst/*)
- `Format`: verifies positional, width/align, `%`, `%m` (flags/suffix), mapping, keywords, roman/letters, `tw`, backtick suffix.
- `Path`: temp path, file create/copy/move/delete, RealizeDirectory, `GetFileOnPath`, normalize/equality.
- `Stream`: MemoryStream and FileStream R/W.
- `Convert`: integer/double scan & format, date/time scan, converters.
- `JSONXML`: JSON parse/pointer ops; XML round-trip.
- `Base64`: encode/decode roundtrip.

How to run tests
- In TheIDE: open each `stdtst/*/*.upp` package and run.
- CLI: compile each `*.cpp` with `-I.` and run the produced binary; tests print `OK` or failing details and exit non-zero.

Backlog / Next steps
- Format
  - Add integer thousands-grouping (locale or explicit flag) if needed.
  - Extend `%m` to true engineering steps (exponent multiple of 3) with a flag.
  - Add full sign/space/zero-pad interactions parity for floats.
- Path
  - Harden attribute checks on Windows (ACL nuances); add `RealizePath` variants matching U++ flags explicitly.
- Streams
  - Add gzip/deflate adapters if required (zlib available in toolchain) — keep optional.
- Value
  - Add basic visitor/compare helpers for simple cases.
- JSON/XML
  - Optional: JSON Patch support building on JSON Pointer; XML namespace resolution improvements.
- Docs
  - Keep `stdsrc/AGENTS.md` and this file synchronized; document any new deviations.

Acceptance criteria (for “Core usable in agent flows”)
- Buildable with C++17 on Windows/MSVC and Linux/g++.
- `stdtst/*` packages pass on both platforms.
- Header usage follows BLITZ policy: leafs are system-include free; `Core.h` aggregates.
- Formatting, path ops, streams, JSON/XML cover the common agent needs (based on current tests).

Contributor notes
- Add new shims under `stdsrc/Core/`; keep leaf headers free of system includes; include them via `Core.h`.
- Prefer STL (and Win32 CRT/APIs where needed); avoid third-party dependencies by default.
- Add a focused `stdtst/*` package for new functionality with at least a couple of scenario tests.
- Document deviations here and in `stdsrc/AGENTS.md`.

Changelog (recent highlights)
- Added platform macros + `DIR_SEP(S)`.
- Replaced `<codecvt>` with Win32/CRT-based UTF‑8/wide conversions and UTF‑8 fallback.
- Implemented U++-style `Format` with backtick suffix and `%m` semantics.
- Implemented Path/Streams/TimeDate, JSON/XML, Base64.
- Added `stdtst` tests for Format, Path, Stream, Convert, JSON/XML, Base64.
