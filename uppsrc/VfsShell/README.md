# VfsShell

VfsShell is a command-line interface for VFS (Virtual File System) operations in the U++ framework. It provides a shell-like environment for navigating and manipulating virtual file systems, similar to traditional Unix shells but operating within the VFS context.

## Features

- pwd: Print working directory
- cd [path]: Change directory
- ls [path]: List directory contents
- tree [path]: Show directory tree structure
- mkdir <path>: Create directory
- touch <path>: Create or update file timestamp
- rm <path>: Remove file or directory
- mv <src> <dst>: Move/rename file or directory
- link <src> <dst>: Create a link between files/directories
- export <vfs> <host>: Export VFS content to host file system
- cat [paths...]: Concatenate and display file content (also reads from stdin if no paths)
- grep [-i] <pattern> [path]: Search for patterns in files
- rg [-i] <pattern> [path]: Ripgrep-like search (placeholder)
- head [-n N] [path]: Display first N lines of a file
- tail [-n N] [path]: Display last N lines of a file
- uniq [path]: Remove duplicate lines
- count [path]: Count characters in file
- history [-a | -n N]: Show command history
- random [min [max]]: Generate random number
- true / false: Return success/failure status
- echo <data...>: Display message

## Architecture

The implementation follows U++ conventions and uses:
- uppsrc/ide package handling for integration
- uppsrc/Vfs for virtual file system operations
- Standard U++ GUI components (CodeEditor) for the interface

## Usage

The VfsShell can be used as a standalone application or integrated into larger U++ applications that require VFS manipulation capabilities.

## Implementation Notes

This implementation replaces the original ~/Dev/VfsBoot/src/VfsShell/ functionality with a more integrated approach using U++'s native systems.