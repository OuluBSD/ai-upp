# Task: Design Python to JavaScript Transpiler

## Goal

Create a transpiler package that converts the supported Python subset into JavaScript suitable for the browser host runtime.

## Initial Direction

- package: `uppsrc/ScriptTranspiler`
- input: Python module source
- output: JavaScript module text plus warnings/errors
- implementation starts as a constrained source-to-source transpiler, not a full Python compiler replacement
