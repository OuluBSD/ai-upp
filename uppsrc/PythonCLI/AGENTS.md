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