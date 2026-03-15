# Multi-Language Transpiler Support

This track focuses on expanding the ScriptIDE transpiler capabilities.

## Directory Conventions

- **Python Source**: `./pysrc/`
- **Transpiled Output**: `./pysrc/transpiled/`

## Objectives

1.  **Add support for typed Python**: Using Python's `typing` module as a foundation for static analysis.
2.  **Add language support to typed-python transpiler**: Target languages: Java, Swift, C#, TypeScript.
3.  **Add non-typed transpiler support**: Support for VB6, VB.NET, PHP, PowerShell (.ps1).
4.  **Full VB6 Support**:
    *   Clone VB6 reference codebase for analysis.
    *   Implement support for `.frm` files, keeping them close to existing `.form` file formats.
    *   Upgrade `.form` format to accommodate VB6 features.
    *   Support all VB6 file formats: `.cls`, `.bas`, `.csi`, `.exp`, `.lvw`, `.vbp`, `.vbw`, `.frm`, `.frx`, `.res`.

## Milestones & Features

- **Java Ecosystem**:
    - Transpile `.py` + `.form` to **Desktop Java** (targeting SWT for native look-and-feel).
    - Transpile `.py` + `.form` to **Android Java** project.
- **VB6 Ecosystem**:
    - Transpile `.py` + `.form` to a complete, runnable **VB6 Project** (`.vbp`).
- **Web (PHP)**:
    - Transpile `.py` + `.form` to a **PHP Web Page**.
    - Static layout with required dynamics implemented via JavaScript.
- **PowerShell**:
    - Transpile `.py` + `.form` to a **PowerShell script** (`.ps1`).
    - Windows GUI integration using **XAML/WPF** (Windows Presentation Foundation).

## Phases

- **Phase 1: Discovery & Setup**: Documentation and reference repository setup.
- **Phase 2: Typed Python Transpiler**: Target Java, Swift, C#, TypeScript.
- **Phase 3: Non-Typed Transpiler**: Target VB6, VB.NET, PHP, PowerShell.
- **Phase 4: VB6 Forms Integration**: `.frm` to `.form` mapping and upgrades.
- **Phase 5: VB6 File Formats**: Supporting the full suite of VB6 project files.
