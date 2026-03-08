# Task: Structured Logging for Logic Engine
# Status: DONE

## Objective
Implement a more structured and configurable logging system for the logic engine and UGUI constraint checks. This should replace the current ad-hoc `FileAppend` and `Print` calls.

## Steps
1.  **Log Categories**: Define categories for different types of logical events (Proof search, Unification, Constraint check, Fact collection).
2.  **Verbosity Levels**: Implement configurable verbosity (ERROR, WARN, INFO, DEBUG, TRACE).
3.  **Output Diversion**: Allow redirecting logic logs to dedicated files, standard `LOG`, or custom GUI consoles without interfering with each other.
4.  **Integration**: Refactor `AI/Logic/Context.cpp` and `AI/LogicGui/Integration.cpp` to use the new logging system.

## Artifacts
- `uppsrc/AI/Logic/Context.cpp`
- `uppsrc/AI/LogicGui/Integration.cpp`
