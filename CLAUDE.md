# CLAUDE - Advanced AI Agent Guide

**For**: Claude (Anthropic) - Advanced reasoning and code generation
**See also**: [AGENTS.md](AGENTS.md) for general conventions, [QWEN.md](QWEN.md) for simpler AI

---

## Quick Start

You're working with Ultimate++ (U++) codebase. Read these in order:
1. **[AGENTS.md](AGENTS.md)** - Core conventions (flags, build policy, headers)
2. **[CODESTYLE.md](CODESTYLE.md)** - Coding style and design tenets
3. **[THREAD_DEPENDENCIES.md](THREAD_DEPENDENCIES.md)** - Current work priorities
4. **[task/](task/)** - Active development threads

---

## Claude-Specific Strengths

You excel at:
- **Complex reasoning**: Understanding intricate dependencies and architectural decisions
- **Code analysis**: Deep diving into existing codebases to understand patterns
- **Refactoring**: Safely restructuring code while maintaining functionality
- **Documentation**: Creating comprehensive, well-organized documentation
- **Problem decomposition**: Breaking complex tasks into manageable steps

Use these strengths when working with this codebase.

---

## Current Focus Areas

### High Priority (from THREAD_DEPENDENCIES.md)
1. **Vfs thread** - VFS tree fixes, Eon test debugging (task/Vfs.md)
2. **VfsShell thread** - Shell overlay + ConsoleIde (task/VfsShell.md)
3. **stdsrc thread** - U++ to STL wrapper implementation (task/stdsrc.md)

### Active Investigation
- **Eon06 test 06a**: Volumetric clouds texture artifacts
  - Location: `upptst/Eon06/`
  - See: task/Vfs.md for current status and investigation notes

---

## Advanced Reasoning Patterns

### When Analyzing Complex Issues

1. **Read the context**:
   - Check task/*.md for thread status
   - Review CURRENT_TASK.md if present
   - Look for related AGENTS.md files

2. **Understand dependencies**:
   - Check THREAD_DEPENDENCIES.md
   - Trace package dependencies via .upp files
   - Identify what blocks what

3. **Test hypothesis**:
   - Run relevant tests (bin/Eon06 0 0, etc.)
   - Use build scripts to verify changes
   - Check for regressions

4. **Document findings**:
   - Update CURRENT_TASK.md with discoveries
   - Add notes to relevant task/*.md files
   - Consider Book chronicle if significant

### When Implementing Features

1. **Plan first**:
   - Break down into subtasks
   - Identify affected packages
   - Check for similar patterns in codebase

2. **Follow conventions**:
   - Use flagV1 for upstream vs custom distinction
   - Follow BLITZ header pattern
   - Match existing code style (see CODESTYLE.md)

3. **Test thoroughly**:
   - Build affected packages
   - Run relevant tests
   - Check for build warnings

4. **Document changes**:
   - Update AGENTS.md if adding new concepts
   - Add to CURRENT_TASK.md
   - Update thread status in task/*.md

---

## Cross-References

### Project Organization
- **[AGENTS.md](AGENTS.md)** - General conventions and policies (READ THIS)
- **[CODESTYLE.md](CODESTYLE.md)** - Code style rules
- **[HIERARCHY.md](HIERARCHY.md)** - Folder structure overview
- **[THREAD_DEPENDENCIES.md](THREAD_DEPENDENCIES.md)** - Thread dependencies

### Active Work
- **[task/Vfs.md](task/Vfs.md)** - VFS fixes and Eon tests
- **[task/VfsShell.md](task/VfsShell.md)** - Shell and ConsoleIde
- **[task/ShaderEditor.md](task/ShaderEditor.md)** - GraphLib + ShaderToy
- **[task/stdsrc.md](task/stdsrc.md)** - U++ to STL wrapper
- **[task/uppstd.md](task/uppstd.md)** - Mapping documentation
- **[task/TODO.md](task/TODO.md)** - Future enhancements

### Package-Specific
- `uppsrc/*/AGENTS.md` - Package-specific guides
- `stdsrc/AGENTS.md` - STL-backed implementations

---

## Tips for Effective Work

1. **Start with context**: Always read relevant task/*.md and AGENTS.md files first
2. **Think architecturally**: Consider impact across packages and threads
3. **Preserve patterns**: Match existing code structure and style
4. **Test incrementally**: Build and test frequently during development
5. **Document thoroughly**: Update docs as you work, not after
6. **Use your strengths**: Deep analysis, careful refactoring, clear documentation

---

## Important: Read AGENTS.md

**All the core conventions are in [AGENTS.md](AGENTS.md)**. This file assumes you've read:
- flagV1 convention (upstream vs custom code)
- Header include policy (U++ BLITZ)
- Build & sandbox policy
- Subpackage independence rules
- Documentation standards (AGENTS.md, CURRENT_TASK.md, Book)
- Known graphics rendering issues

**Don't skip AGENTS.md** - it contains critical information that applies to all AI agents.
