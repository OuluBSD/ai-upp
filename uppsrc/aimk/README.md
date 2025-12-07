# aimk - AI Make

AI-friendly commandline U++ package/assembly builder.

## Overview

`aimk` is a fork of `umk` (Ultimate++ Make) designed to provide a more AI-friendly interface for building U++ applications. The goal is to make it easier for AI agents to understand build output, errors, and to provide clearer command-line options.

## Current Status

The package structure has been created and adapted from `umk`:

- Package files renamed from umk → aimk
- Header guards updated
- Help text updated to reflect aimk branding
- Config file search paths now check for aimk config directories first

## Building

The build script is ready:

```bash
script/build-console.sh aimk
```

Or using umk directly:

```bash
umk upptst,examples,tutorial,reference,uppsrc aimk CLANG.bm -bsH1 +NOGUI,USEMALLOC,DEBUG_FULL,MAIN bin/aimk
```

### Known Build Issues

**Currently the build fails with linker errors** related to Android and Java modules. This is a pre-existing issue that also affects `umk` itself in this codebase. The errors are:

- Undefined references to Android SDK/NDK classes
- Undefined references to Java/JDK classes
- Undefined references to libclang functions

These appear to be related to how ide/Builders package handles its dependencies on ide/Android and ide/Java. The modules compile but don't link properly.

## Future Enhancements

Once the build issues are resolved, planned AI-friendly features include:

- Structured JSON output for errors and warnings
- Machine-readable build progress
- Simplified command-line options
- Better error messages with context
- Integration with AI coding assistants

## Differences from umk

Current changes:
- Config directory search includes `~/.config/u++/aimk/` before umk paths
- UppHub search includes aimk directories
- Updated branding and help text

## Files

- `aimake.h` - Main header (was umake.h)
- `aimake.cpp` - Main implementation with CONSOLE_APP_MAIN entry point
- `Console.cpp` - Console/process management
- `IdeContext.cpp` - IDE context implementation
- `MakeBuild.cpp` - Build implementation
- `Export.cpp` - Export functionality
- `UppHub.cpp` - UppHub integration
- `aimk.cpp` - Windows launcher stub (delegates to theide.exe)
- `aimk.upp` - Package definition
