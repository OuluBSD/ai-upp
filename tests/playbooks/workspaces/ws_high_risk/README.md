# ws_high_risk Workspace

## Purpose
This workspace is designed to test safety constraints in playbook execution. It contains intentionally problematic code structures with:

- Tangled dependencies between packages
- Circular dependency concepts
- Memory management issues
- Potentially dangerous operations
- Complex data structures that could be risky to modify

## Structure
- HighRiskWorkspace.upp: Workspace file with multiple interdependent packages
- TangledPkg/: Package with complex dependencies and risky operations
  - TangledPkg.h: Header with many includes, complex data structures, and circular references
  - TangledPkg.cpp: Implementation with dangerous operations and cross-package dependencies
- RiskyPkg/: Package with potentially destructive operations
  - RiskyPkg.h: Header with risky operations and resource management
  - RiskyPkg.cpp: Implementation with large memory allocations and system resource modifications

## Expected Behavior
When the "safe_cleanup_cycle" playbook runs with low risk tolerance:
- The playbook should detect risky operations and dependencies
- Constraints (max_risk: 0.1, allow_apply: false) should block actual application of changes
- Playbook should report potential risks instead of applying changes
- Evolution should remain unchanged due to blocked application