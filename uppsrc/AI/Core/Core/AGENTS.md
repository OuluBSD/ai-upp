Purpose: Core utilities, types, enums used by all subpackages.

- Start here when you need:
  - Base types (`Types.*`), common helpers (`Common.*`, `Fn.*`).
  - Enums and content tags (`Enums.*`, `EnumsContent.cpp`).
  - Natural language and phoneme primitives.

- Tasks examples:
  - Add new enum or tag → update `Enums.*` (and `EnumsContent.cpp`).
  - Add utility shared across packages → `Common.*` or `Fn.*`.
  - New core type → `Types.*`.

- Public surface:
  - Use `#include <AICore2/Core/Core.h>` (re-exports `Public.h`).
