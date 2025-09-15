Current Task: STL-backed Core API (Phase 1)

Goal
- Provide a compatible developer interface for key U++ Core classes under `stdsrc/Core`, backed by the C++ standard library (and Boost when needed).

Scope (Phase 1)
- Implement `String` and `WString` with common methods: construction, `GetLength`, `IsEmpty`, `Clear`, `Cat`, `Left/Right/Mid`, `Find/ReverseFind`, `StartsWith/EndsWith`, `Trim`.
- Add core containers: `Vector<T>`, `Index<T>`, `VectorMap<K,T>`.
- Provide `One<T>` unique-ownership wrapper, minimal `Value` via `std::any`, and hashing helpers.
- Provide `Uuid` utilities and small `Util` helpers (`AsString`, `FormatPtr`).
- Provide `Core.h` aggregator and package manifest `Core.upp` following U++ header/BLITZ policy.

Design Notes
- `String` inherits `std::string`; `WString` inherits `std::wstring`.
- Non-aggregator headers do not include system headers; `Core.h` includes `<string>`, `<cwchar>`, `<locale>`, `<codecvt>`.
- Namespace is applied only by `Core.h` using `NAMESPACE_UPP`/`END_UPP_NAMESPACE` macros.

Done
- Scaffolding: `AGENTS.md`, `CURRENT_TASK.md`, `Core.upp`.
- Aggregator: `Core.h` including all public headers and `std::hash` specializations for `Upp::String/WString`.
- Strings: `String.h`, `WString.h` (inherit `std::string`/`std::wstring`).
- Containers: `Vcont.h` (`Vector<T>`), `Index.h`, `Map.h` (`VectorMap<K,T>`).
- Ownership: `One.h` wrapper over `std::unique_ptr`.
- Hashing: `Hash.h` with `CombineHash`, `GetHashValue`.
- Utils: `Util.h` (`AsString`, `FormatPtr`).
- UUID: `Uuid.h` with `Create`, `ToString`, `TryParse`.

Next
- Expand containers: add ordered removal/insert variants and more U++ parity helpers as needed.
- Add `Ptr` and maybe a simple `Callback` shim if required by dependents.
- Consider basic `TimeDate` shim (wrapping `<chrono>`) and minimal `Stream` memory streams for interop.
- Add unit-style sanity snippets to validate behavior (kept local to package).

Out of Scope (later phases)
- Streams, Value, XML/JSON, threading, process APIs.
- Advanced Unicode/UTF helpers (will add adapters where needed).
