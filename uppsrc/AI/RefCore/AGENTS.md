# AI/RefCore Package

## Purpose
A reference-counted memory management and path resolution framework. It provides the foundation for tree-like data structures used in logic and AI planning.

## Core Classes
- **Ref<T>**: Base class for reference-counted objects.
- **Var<T>**: Smart pointer for `Ref` objects.
- **PathResolver**: Resolves hierarchical paths (e.g., `/Root/Child?arg=val`) to `MetaNode` objects.
- **MetaNode**: Base class for resolvable objects.
- **MetaTime**: Utilities for handling time-series or period-based data in metadata.

## Path Scripting
The system supports a custom URL-like path syntax for accessing and creating objects dynamically.
Example: `/Scene/Object[0]/Properties?visible=true`
