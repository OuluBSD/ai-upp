# ScriptTranspiler Supported Subset

This package currently targets the Python subset exercised by:

- `uppsrc/ScriptIDE/reference/Hearts/main.py`
- `uppsrc/ScriptIDE/reference/Solitaire/main.py`

It is intentionally not general Python.

## Supported Patterns

- top-level `import module`
- function definitions
- `if` / `elif` / `else`
- `while`
- `for ... in ...`
- `return`
- `global`
- `assert`
- simple assignments
- augmented assignments like `+=`
- list literals and nested list literals
- indexing like `arr[i]`
- string concatenation
- inline conditional expressions
- tuple unpacking in assignment / iteration
- common builtins:
  - `len`
  - `str`
  - `int`
  - `min`
  - `max`
- Python truthiness checks via runtime helpers
- list operations:
  - `.append`
  - `.clear`
  - `.remove`
  - list concatenation with `arr = arr + [item]`

## Current Assumptions

- modules are resolved by the browser host, not by recursive Python file transpilation
- transpiled helper-module imports should currently use flat names (`import mymodule`)
- dynamic Python features are out of scope
- mixed object models and runtime metaprogramming are out of scope
- classes only work where the host/runtime explicitly supports the resulting patterns

## Near-Term Target

Keep extending the subset only when a concrete reference project requires it.
The next extensions should be driven by Hearts and Solitaire behavior, not by theoretical Python coverage.
