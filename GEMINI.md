This document serves as an internal guide for the Gemini AI agent.

**Before starting any task, the Gemini AI agent should always read the following files:**

- **[AGENTS.md](AGENTS.md)**: This is the primary guide for all AI agents, providing comprehensive rules, conventions, and architectural details.
- **[QWEN.md](QWEN.md)**: This document offers a simplified and repetitive version of the core rules from AGENTS.md, which can be useful for quick refreshers and reinforcing critical guidelines.

**These documents contain crucial information regarding:**
- Build system (`umk`) usage and its intricacies (e.g., resource bundling, caching).
- Code style and naming conventions.
- Header inclusion policies (U++ BLITZ, `PackageName.h`, avoiding nested namespaces).
- `flagV1` convention for distinguishing original U++ code from custom additions.
- Current task status and priority in `task/` directory.
- Debugging strategies and environment-specific considerations (e.g., sandbox detection).
- **Windows Environment**: Note that `busybox` might be present on Windows systems. It should be used when necessary to perform Unix-like operations or when standard Windows shell commands are insufficient or problematic (e.g., creating files with specific content without escape sequence issues).

## Naming Conflicts and Include Policies (MSVC/Win32)

- **System Header Placement:** NEVER include system or external headers (like `windows.h`, `dshow.h`, `portaudio.h`, etc.) inside a `namespace Upp { ... }` or between `NAMESPACE_UPP` and `END_UPP_NAMESPACE`. Doing so causes identifiers like `CY` and `FAR` to be prefixed with `Upp::`, leading to severe syntax errors in Windows SDK headers.
- **Local Include Placement:** Local package includes (e.g., `#include "LocalFile.h"`) are intentionally placed inside the `Upp` namespace if they are part of the package implementation.
- **Main Header Rule:** ALWAYS include the main package header (e.g., `<PackageName/PackageName.h>`). NEVER include sub-headers (e.g., `<PackageName/SubHeader.h>`) directly, as they are not guaranteed to be standalone or correctly wrapped.
- **CY and FAR Protection:** When including system headers that use `CY` or `FAR`, wrap them with the established U++ protection pattern:
  ```cpp
  #ifdef flagWIN32
  #define CY win32_CY_
  #define FAR win32_FAR_
  #endif
  #include <system_header.h>
  #ifdef flagWIN32
  #undef CY
  #undef FAR
  #endif
  ```

## Uppy Python Interpreter (ByteVM) Insights

- **Module Imports**: Support for `from module import *` and dotted paths (e.g., `os.path`) requires recursive traversal of module dictionaries. If a module is not in `globals`, check `sys.modules`.
- **Statement Termination**: To prevent infinite loops in `PyCompiler`, ensure that every statement path (including `if`, `while`, `for`, `def`, `return`, `import`) explicitly consumes its terminator (usually `TK_NEWLINE` or `TK_END_STMT`) if `IsStmtEnd()` is true.
- **Python-like Methods**: Methods like `str.endswith()` and `str.join()` are implemented using `PyValue::BoundMethod`, which binds a builtin function to a specific object instance on the stack.
- **Built-in Modules**: Modules like `os` and `sys` are implemented as `PyValue::Dict()` populated with `PyValue::Function` wrappers around C++ functions.
- **Exit Handling**: `sys.exit(code)` should throw an `Exc` with a specific prefix (e.g., `"EXIT:code"`). The main `PythonCLI` loop catches this to extract the exit status and terminate gracefully.
- **String Formatting**: Basic `%` formatting support is implemented in `PY_BINARY_MODULO` by checking if the left operand is `PY_STR`.
- **Recursive CLI Calls**: Be cautious with `subprocess.run` implementations that call `bin/PythonCLI` recursively; ensure the underlying process management (e.g., `system()`) doesn't lead to deadlocks or output buffering issues.

Familiarity with these guides is essential for effective and compliant operation within this codebase.
