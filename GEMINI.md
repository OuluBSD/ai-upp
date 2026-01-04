This document serves as an internal guide for the Gemini AI agent.

**Before starting any task, the Gemini AI agent should always read the following files:**

- **[AGENTS.md](AGENTS.md)**: This is the primary guide for all AI agents, providing comprehensive rules, conventions, and architectural details.
- **[QWEN.md](QWEN.md)**: This document offers a simplified and repetitive version of the core rules from AGENTS.md, which can be useful for quick refreshers and reinforcing critical guidelines.

**These documents contain crucial information regarding:**
- Build system (`umk`) usage and its intricacies (e.g., resource bundling, caching).
- Code style and naming conventions.
- Header inclusion policies (U++ BLITZ, `PackageName.h`, avoiding nested namespaces).
- `flagV1` convention for distinguishing original U++ code from custom additions.
- Current task status and priority in `task/` directory.
- Debugging strategies and environment-specific considerations (e.g., sandbox detection).

Familiarity with these guides is essential for effective and compliant operation within this codebase.
