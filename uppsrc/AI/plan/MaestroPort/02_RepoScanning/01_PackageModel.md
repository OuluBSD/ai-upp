# Task: Package Model
# Status: TODO

## Objective
Define the C++ data structures to represent a repository package, mirroring `maestro/repo/package.py`.

## Requirements
- Struct `PackageInfo` with fields: name, dir, build_system (upp, cmake, etc), dependencies, file groups.
- Struct `FileGroup` for organizing source files.
- Serialization support (Jsonize) for saving/loading scan results.

## Context
See `~/Dev/Maestro/maestro/repo/package.py`.
