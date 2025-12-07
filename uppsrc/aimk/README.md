# aimk - AI Make

AI-friendly commandline U++ package/assembly builder with modern, intuitive CLI.

## Overview

`aimk` is a redesigned version of `umk` (Ultimate++ Make) specifically built for AI agents and modern workflows. It features:

- **Clear, readable options**: `--package=MyApp` instead of positional arguments
- **Comprehensive help**: `--help` shows detailed usage with examples
- **Named parameters**: No more confusing positional arguments
- **Intuitive flags**: `--clean`, `--verbose`, `--threads=8` instead of `-a`, `-v`, `-H8`

## Quick Start

Build a package:
```bash
aimk --package=Bombs
```

Build with custom settings:
```bash
aimk -p MyApp -a ~/myapp/src,uppsrc -f GUI,SHARED -o ~/bin/myapp
```

## Usage

```
USAGE:
  aimk --package=<name> [OPTIONS]
  aimk -p <name> [OPTIONS]

REQUIRED:
  -p, --package=NAME         Main package to build (e.g., MyApp)

COMMON OPTIONS:
  -a, --assembly=PATHS       Assembly directories (comma or colon separated)
                             Default: upptst,examples,tutorial,reference,uppsrc
  -m, --method=NAME          Build method (GCC, CLANG, MSC, etc.)
                             Default: CLANG
  -o, --output=PATH          Output executable path
  -f, --flags=FLAGS          Build flags (comma separated: GUI,SHARED,etc.)
  -c, --config=NAME          Build configuration (debug, release)

BUILD OPTIONS:
  --clean                    Clean before building
  --blitz                    Enable blitz build
  --threads=N                Number of parallel build threads
  --verbose                  Verbose build output

EXPORT OPTIONS:
  --makefile[=PATH]          Generate Makefile
  --compile-commands         Generate compile_commands.json

RUN OPTIONS:
  --run [ARGS...]            Run executable after successful build

OTHER OPTIONS:
  -h, --help                 Show help message
  -v, --version              Show version information
```

## Examples

```bash
# Simple build
aimk --package=Bombs

# Build with custom assembly and flags
aimk -p MyApp -a ~/myapp/src,uppsrc -f GUI,SHARED -o ~/bin/myapp

# Clean build with CLANG, 8 threads
aimk -p MyApp --clean --method=CLANG --threads=8

# Build and run with arguments
aimk -p MyApp --run arg1 arg2

# Generate compile_commands.json for IDE
aimk -p MyApp --compile-commands
```

## Building aimk

The build script:
```bash
script/build-console.sh aimk
```

Or using umk directly:
```bash
umk upptst,examples,tutorial,reference,uppsrc aimk CLANG.bm -bsH1 +NOGUI,USEMALLOC,DEBUG_FULL,MAIN bin/aimk
```

### Known Build Issues

**Currently the build fails with linker errors** related to Android and Java modules. This is a pre-existing issue that also affects `umk` itself. Once resolved, aimk will be fully functional.

## Key Improvements over umk

### Command-Line Interface
- ✅ Long option names (`--package` vs positional args)
- ✅ Short options with values (`-p MyApp`)
- ✅ `--help` with comprehensive documentation
- ✅ `--version` flag
- ✅ Clear error messages
- ✅ Intuitive option naming

### AI-Friendly Design
- Named parameters make intent clear
- No cryptic single-letter flags
- Self-documenting commands
- Consistent option syntax
- Helpful examples in help text

## Files

- `aimake.h` - Main header with Ide and Console classes
- `aimake.cpp` - Main implementation with modern CLI parser
- `Console.cpp` - Console/process management
- `IdeContext.cpp` - IDE context implementation
- `MakeBuild.cpp` - Build implementation
- `Export.cpp` - Export functionality
- `UppHub.cpp` - UppHub integration
- `aimk.cpp` - Windows launcher stub
- `aimk.upp` - Package definition
