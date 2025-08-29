Purpose: Runtime engine, process framework, and task orchestration.

- Start here when you need:
  - To define or extend processes, tasks, agents.
  - To integrate remote execution or code processing.

- Tasks examples:
  - New process type → `ProcessBase.*` / `ProcessFramework.*`.
  - Task orchestration → `TaskManager.*`, `RemoteTask.*`.
  - Agent plumbing → `Agent.*`, `Omni.*`, prompts in runtime → `Prompt.*`.

- Public surface:
  - Use `#include <AICore2/Runtime/Runtime.h>` (re-exports `Public.h`).
