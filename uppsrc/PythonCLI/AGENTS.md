# PythonCLI Package AGENTS.md

## Purpose
PythonCLI is a command-line interface for the Uppy Python-like interpreter based on U++ ByteVM technology.

## Naming Convention
- The interpreter is named "Uppy" (not Python) to distinguish it from the official Python interpreter
- Version is 0.1v to indicate it's an early version

## Banner Format
The banner should match Python's format but with Uppy branding:
```
Uppy 0.1v (based on U++ Python implementation)
Type "help", "copyright", "credits" or "license" for more information.
>>>
```

## Key Features
- Interactive REPL mode with >>> prompt
- Support for basic Python-like syntax
- Integration with U++ ByteVM for execution
- Ability to run script files
- **New (0.1v+)**:
  - Subscription assignment (`list[idx] = val`, `dict[key] = val`)
  - String indexing (`str[idx]`)
  - Complex numbers support in arithmetic
  - Builtins: `abs()`, `bool()`, `complex()`, `iter()`, `next()`, `len()`, `min()`, `max()`, `sum()`, `range()`, `print()`
  - `pass` statement support

## Tests
Regular Python tests are located in `share/py/tests/`:
- `01_basic.py`: Arithmetic, variables, booleans
- `02_types.py`: Lists and Dicts (including assignment)
- `03_control_flow.py`: if, while, for loops
- `04_functions.py`: Function definitions and recursion (Fibonacci)
- `05_complex.py`: Complex numbers arithmetic
- `06_strings.py`: String concatenation and indexing
- `07_iterators.py`: iter() and next() builtins
- `08_builtins.py`: abs, min, max, sum builtins
- `09_misc.py`: pass statement

## Architecture
- Uses Tokenizer from Core/TextParsing
- PyCompiler and PyVM from ByteVM package
- Standard U++ console application pattern

## Special Commands
- quit() or exit() to terminate the interpreter
- help, copyright, credits, license for information

## Dependencies
- Core package for basic functionality
- ByteVM package for Python-like execution engine