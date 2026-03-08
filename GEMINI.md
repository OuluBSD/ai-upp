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

## Memory Management & Pooling

### RecyclerPool and BiVectorRecycler

## Logging and GUI Automation

- **Alerts and Popups:** In GUI applications, avoid using modal dialogs (like `Exclamation`, `PromptOK`) during automated tests as they block the execution. Instead, use a wrapper that redirects messages to `LOG` when running in automation mode.
- **Log Inspection:** Prefer using `LOG()` macros for debugging and monitoring. Users should be encouraged to use `tail -f ~/.local/state/u++/log/<PackageName>.log` (or the equivalent path) to monitor output in real-time, rather than relying on `stdout`.
- **Automation Mode Detection:** Check for command-line flags like `--test` to determine if the application is running in an automated environment.

## Rich Text Format (QTF)

- **Standard**: In Ultimate++, the standard internal rich text format is **QTF** (Quick Text Format).
- **No HTML/XML**: ALWAYS use QTF for formatted text. NEVER use HTML or XML for reports or rich UI elements.
- **API**: Always prefer `SetQTF()` over other methods for displaying formatted text in `RichTextView` or other rich text-capable controls.

#### RecyclerPool<T, keep_as_constructed>
- **Purpose**: Manages a pool of allocated objects of type `T`.
- **`keep_as_constructed=true`**: If true, the destructor of `T` is NOT called when returning to the pool, and the constructor is NOT called when allocating new items (after the initial allocation). This is ideal for reuse of complex objects like `Vector` buffers where you want to retain capacity.
- **Thread Safety**: Internally synchronized (can be used from multiple threads).

#### BiVectorRecycler<T, keep_as_constructed>
- **Purpose**: A double-ended queue (deque) that automatically manages object reuse via an internal `RecyclerPool`.
- **Usage**:
  ```cpp
  BiVectorRecycler<RawDataBlock, true> queue;
  RawDataBlock* block = queue.AddTail(); // Allocates or reuses
  // ... use block ...
  queue.DropHead(); // Returns to pool
  ```
- **Ownership**: The container owns the *pointers* and manages their lifecycle relative to the pool. Use `pick()` to transfer ownership of the active queue items to another `BiVectorRecycler` (e.g., passing data between threads).
- **Move Semantics**: Supports moving (`pick`, `std::move`). When moved, the source container becomes empty, and the destination takes over the active items. The underlying pools remain separate, but items can safely cross between compatible pools.

**Use Case Example (SoftHMD Camera)**:
- Replaced `std::vector` and manual shifting with `BiVectorRecycler<RawDataBlock, true>`.
- `RawDataBlock` contains a `Vector<byte>`.
- `keep_as_constructed=true` ensures the internal capacity of `Vector<byte>` is preserved when blocks are recycled, minimizing heap allocations.

Familiarity with these guides is essential for effective and compliant operation within this codebase.
