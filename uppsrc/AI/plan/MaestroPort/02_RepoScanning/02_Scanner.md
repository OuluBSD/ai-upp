# Task: Directory Scanner
# Status: TODO

## Objective
Implement a recursive directory scanner that identifies packages based on file markers (e.g., `.upp`, `CMakeLists.txt`).

## Requirements
- Function `ScanRepo(String root)` returning `Array<PackageInfo>`.
- Parse `.upp` files to extract file lists and dependencies.
- Basic detection of CMake projects.

## Context
See `~/Dev/Maestro/maestro/repo/scanner.py` and `upp_parser.py`.
