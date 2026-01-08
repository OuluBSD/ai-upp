# ws_basic_cleanup Workspace

## Purpose
This workspace is designed to test the "safe_cleanup_cycle" playbook functionality. It contains intentionally "messy" code with:

- Unused includes (iostream, vector, map)
- Simple functions with trivial complexity
- Unused functions that should be candidates for removal
- Unnecessary code that can be cleaned up safely

## Structure
- TestUpp.upp: Workspace file including Core, CtrlCore and TestPkg
- TestPkg/: Package with intentionally messy code
  - TestPkg.h: Header with unused includes and declarations
  - TestPkg.cpp: Implementation with unused functions and complex code

## Expected Behavior
The "safe_cleanup_cycle" playbook should be able to perform small, safe cleanups in this workspace, such as:
- Removing unused includes
- Identifying and suggesting removal of unused functions
- Simplifying overly complex code sections