# Task: Audit Python Subset For Browser Transpilation

## Goal

Define the Python subset that can be transpiled into browser-oriented JavaScript without destabilizing the existing Python/ByteVM path.

## Scope

- inventory current `.gamestate` Python patterns
- define supported syntax and runtime assumptions
- identify unsupported Python features that must fail clearly

## Key Principle

Python remains the source language. JavaScript is generated output for the browser host, not a new authoring frontend for `ByteVM`.
