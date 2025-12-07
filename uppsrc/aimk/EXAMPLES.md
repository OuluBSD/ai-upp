# aimk Command Examples

## Basic Usage

### Simplest build
```bash
aimk --package=Bombs
# Builds Bombs package with default settings
# Default assembly: upptst,examples,tutorial,reference,uppsrc
# Default method: CLANG
# Default config: debug
```

### Short form
```bash
aimk -p Bombs
# Same as above, using short option
```

## Common Scenarios

### Build with custom assembly paths
```bash
aimk -p MyApp -a ~/myproject/src,~/upp/uppsrc
# Specify where to find source packages
```

### Build with specific compiler
```bash
aimk -p MyApp -m GCC
# Use GCC instead of default CLANG
```

### Build with flags
```bash
aimk -p MyApp -f GUI,SHARED
# Add GUI and SHARED flags to build
```

### Custom output location
```bash
aimk -p MyApp -o ~/bin/myapp
# Place executable at specific path
```

### Release build
```bash
aimk -p MyApp -c release
# Build in release configuration
```

## Advanced Usage

### Clean build with multiple options
```bash
aimk -p MyApp --clean --blitz --threads=8 --verbose
# Clean before building
# Enable blitz mode
# Use 8 parallel threads
# Show verbose output
```

### Build and run
```bash
aimk -p MyApp --run
# Build and execute the result
```

### Build and run with arguments
```bash
aimk -p MyApp --run --arg1 value1 --arg2 value2
# Build and run with command-line arguments
```

### Generate compile_commands.json
```bash
aimk -p MyApp --compile-commands
# Generate compile_commands.json for IDE integration
# Useful for clangd, VSCode, CLion, etc.
```

### Generate Makefile
```bash
aimk -p MyApp --makefile
# Generate Makefile in current directory
```

```bash
aimk -p MyApp --makefile=~/project/Makefile
# Generate Makefile at specific path
```

### Export project
```bash
aimk -p MyApp --export-project=~/exported
# Export project files to directory
```

## Comparison with umk

### Old way (umk)
```bash
# confusing positional arguments
umk examples Bombs CLANG -ab +GUI,SHARED ~/bombs

# What does this mean?
# - examples = assembly
# - Bombs = package
# - CLANG = method
# - -a = clean
# - -b = blitz
# - +GUI,SHARED = flags
# - ~/bombs = output

# Hard to remember, hard for AI to parse
```

### New way (aimk)
```bash
# clear, self-documenting options
aimk -p Bombs -a examples -m CLANG --clean --blitz -f GUI,SHARED -o ~/bombs

# Or more naturally:
aimk --package=Bombs \
     --assembly=examples \
     --method=CLANG \
     --clean \
     --blitz \
     --flags=GUI,SHARED \
     --output=~/bombs

# Clear what each parameter does
# Easy for AI to understand and generate
```

## Real-World Examples

### Typical C++ GUI application
```bash
aimk -p MyGuiApp -f GUI,SHARED -c release -o ~/bin/myguiapp
```

### Console utility with debugging
```bash
aimk -p MyTool -c debug -f DEBUG_FULL --verbose -o ~/bin/mytool-debug
```

### Cross-platform library
```bash
aimk -p MyLib -f SHARED,SO --static -o ~/lib/libmylib.so
```

### Complete workflow
```bash
# Clean build with tests
aimk -p MyApp --clean --method=CLANG --threads=16 --verbose

# Generate IDE files
aimk -p MyApp --compile-commands

# Build and run
aimk -p MyApp --run --test-mode

# Build release
aimk -p MyApp -c release -o ~/release/myapp
```

## Getting Help

### Show help
```bash
aimk --help
# or
aimk -h
```

### Show version
```bash
aimk --version
# or
aimk -v
```

### Error: missing package
```bash
aimk --clean
# Error: Package name is required
#
# [Shows full help text]
```

### Error: unknown option
```bash
aimk -p MyApp --unknown-flag
# Unknown option: --unknown-flag
# Use --help for usage information
```

## Tips for AI Agents

When generating aimk commands:

1. **Always specify package**: Use `-p` or `--package=`
2. **Use long forms for clarity**: `--method=CLANG` clearer than `-m CLANG`
3. **Combine related options**: `--clean --verbose` for debugging builds
4. **Default assembly works**: Don't specify `-a` unless custom paths needed
5. **Check help first**: Use `--help` to understand available options
