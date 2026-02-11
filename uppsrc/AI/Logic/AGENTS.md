# AI/Logic Package

## Purpose
This package provides a First-Order Logic (FOL) Theorem Prover engine based on Sequent Calculus and Unification. It is intended to be used for formal verification of system constraints and AI reasoning.

## Core Classes
- **Node**: Base class for all terms and formulas. Uses `RefCore` for memory management.
- **NodeVar**: Smart pointer/handle for `Node` objects.
- **Variable / Function / UnificationTerm**: Term types.
- **Predicate / Not / And / Or / Implies / ForAll / ThereExists / Equal**: Formula types.

## Prover Logic
The engine uses a frontier-based search in Sequent Calculus.
- **Unification**: Used to solve for existential variables.
- **Identity**: Basic support for reflexivity (`t = t`).

## Integration
- Exposes `ProveLogic(String)` and `AddAxiom(String)` for simple CLI/GUI integration.
- Custom output can be captured via `WhenPrint` callback.

## Future Scientific Features
- Full Equality reasoning (Paramodulation).
- Resolution-based proving.
- Model checking and counter-example generation.
