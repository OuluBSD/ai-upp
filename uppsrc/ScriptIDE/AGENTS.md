# ScriptIDE - Python IDE using ByteVM

## Overview
ScriptIDE is a Spyder-like Python IDE built on Ultimate++ framework using the internal ByteVM Python interpreter.

## Features
- Python code editor with syntax highlighting
- IPython-like console
- Variable explorer
- Integrated debugger (breakpoints, step over/in/out)
- File navigation
- Help browser
- Work area management

## Architecture
- **PythonIDE**: Main window managing layout and coordination
- **PythonConsole**: IPython-style console using ByteVM
- **VariableExplorer**: ArrayCtrl showing stack variables
- **FileTree**: TreeCtrl for file navigation
- **Settings**: Configuration management

## Dependencies
- ByteVM: Python interpreter with debugging support
- CodeEditor: Syntax highlighting for Python
- TabBar/FileTabs: File tab management
- RichEdit: Help system

## ByteVM Integration
Uses internal uppsrc/ByteVM Python interpreter ONLY. No external Python runtime required.
