# Task: PackageInfo Struct
# Status: DONE

## Objective
Define the `PackageInfo` struct in `uppsrc/Maestro/PackageInfo.h`.

## Requirements
- Fields: `name`, `dir`, `upp_path`, `build_system`, `dependencies`, `groups`, `ungrouped_files`, `is_virtual`, `virtual_type`.
- `Vector<FileGroup> groups`.
- `Vector<String> dependencies`, `ungrouped_files`, `files`.
- Add `Jsonize` support.
