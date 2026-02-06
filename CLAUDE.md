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
- **Eon07 test black screen**: Component initialization working, investigating rendering pipeline
  - See: AGENTS.md "ECS Initialization and Component Lifecycle" for patterns learned

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
   - **For memory leaks**: Always use Valgrind with USEMALLOC config (see AGENTS.md "Memory Leak Detection")

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
   - Build affected packages: `script/build.py -mc 0 -j 12 <PackageName>`
   - Run relevant tests
   - Check for build warnings

4. **Document changes**:
   - Update AGENTS.md if adding new concepts
   - Add to CURRENT_TASK.md
   - Update thread status in task/*.md

### When Working with ECS Components

See **AGENTS.md "ECS Initialization and Component Lifecycle"** for critical patterns:

1. **Arg() Phase**: Store configuration, **never resolve cross-references**
   - Defer entity path resolution, component lookups to Initialize()

2. **Initialize() Phase**: Resolve cross-references when full tree exists
   - Use manual VfsValue path traversal for untyped intermediate nodes
   - Access systems via `GetEngine().TryGet<System>()`, not entity scope

3. **PostInitialize() Phase**: Handle dependencies on other Initialize() side effects

4. **Common Pitfalls**:
   - Starting boolean success tracking with `false` instead of `true`
   - Using `FindPath<Entity>()` with untyped intermediate VfsValues
   - Searching for systems in entity scope instead of engine scope

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

## Meta-Awareness: Improving Code Readability

When debugging, pay attention to **why** problems were hard to detect. Your reasoning capabilities make you excellent at identifying structural issues that hide bugs.

### Ask These Questions

After finding a difficult bug:
1. **What made this hard to find?** Was naming misleading? Were types ambiguous?
2. **Would renaming help?** Could better names make this class of bug obvious?
3. **Are error messages helpful?** Do they point to root cause or just symptoms?
4. **Is this pattern repeated?** Are there other places with similar ambiguity?

### Propose Improvements Proactively

You are **encouraged** to suggest:
- **Renaming operations**: Find-and-replace across all files is welcome (commit first!)
- **Better error messages**: Add context that explains what went wrong and why
- **Type disambiguation**: Add parameters to prevent incorrect matches
- **Macro improvements**: Make hidden information explicit in macro names

### Example: The `REGISTER_EON_*` Refactoring

**Original Problem**: Macros `REGISTER_COMPONENT` and `REGISTER_SYSTEM_ECS` both registered into the same global factory with `eon_name` keys, but the different macro names falsely suggested separate namespaces. This caused a name collision (`"physics"` registered as both system and component) that was hard to detect.

**Root Cause Analysis**:
- Macro names suggested separation that didn't exist
- The shared namespace (eon_name) was not emphasized
- No type checking prevented collisions

**Solution Applied**:
1. Renamed macros to `REGISTER_EON_COMPONENT` and `REGISTER_EON_SYSTEM` to emphasize shared namespace
2. Modified `FindFactoryEon()` to accept type parameter for disambiguation
3. Added better error messages showing type mismatches

**Key Insight**: The macro abstraction **hid** that both registrations shared the same key space. Making this explicit prevented the bug class entirely.

### Your Role in Code Quality

Use your analytical strengths to:
- Spot patterns that obscure problems
- Identify where explicitness would prevent bugs
- Propose structural improvements, not just bug fixes
- Think about future AI agents and developers reading this code

**Remember**: This project values **clarity over cleverness**. Suggestions that make problems obvious are always welcome.

---

## Important: Read AGENTS.md

**All the core conventions are in [AGENTS.md](AGENTS.md)**. This file assumes you've read:
- flagV1 convention (upstream vs custom code)
- Header include policy (U++ BLITZ)
- Build & sandbox policy
- Memory leak detection & Valgrind workflow
- Subpackage independence rules
- Documentation standards (AGENTS.md, CURRENT_TASK.md, Book)
- Known graphics rendering issues

**Don't skip AGENTS.md** - it contains critical information that applies to all AI agents.
