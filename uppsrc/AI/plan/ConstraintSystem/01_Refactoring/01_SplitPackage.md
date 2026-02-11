# Task: Split TheoremProver Package
# Status: DONE

## Objective
Refactor the imported `TheoremProver` codebase. The core logic (parsing, evaluation, proving) must be moved to a new reusable package `uppsrc/AI/Logic`. The application shell (CLI/GUI) should remain as an example in `examples/TheoremProver`.

## Steps
1.  Create `uppsrc/AI/Logic` directory and `.upp` file.
2.  Move `Language.h/cpp`, `Parser.cpp`, `Prover.cpp`, `Typecheck.cpp`, `Evaluation.cpp` to `AI/Logic`.
3.  Ensure `AI/Logic` is part of the `AI` nest.
4.  Move `TheoremProver.cpp` (main app logic) to `examples/TheoremProver`.
5.  Update `examples/TheoremProver.upp` to use `AI/Logic`.
6.  Verify compilation of both the library and the example.

## Artifacts
- `uppsrc/AI/Logic/*`
- `examples/TheoremProver/*`
