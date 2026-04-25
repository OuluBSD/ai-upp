# Profiling

## What this page is for
This page is about how Core thinks about observing cost.

Profiling in Core is less about performance theater and more about preserving a culture where runtime expense can be inspected instead of guessed at.

## Measurement as responsibility
Core's profiling attitude fits its broader hardware-near mindset. If cost matters, then measurement should live close to the runtime rather than only in external tooling.

That does not mean the package needs a vast performance laboratory. It means a serious framework should have native ways to ask where time went and how often work occurred.

## Modest tools, serious meaning
Even lightweight profiling hooks matter because they normalize a certain kind of engineering behavior.

They say:

- cost is not mysterious
- timing should be discussable
- performance claims should be testable

This is exactly the kind of modest but important cultural infrastructure that belongs in Core.

## Future direction
Profiling is one of the clearest areas where Core could grow without changing its temperament.

Plausible directions include:

- tighter integration with developer tools
- clearer aggregation and visualization
- better cross-platform timing trust
- more useful support for service and background workloads

If that happens, Core should still resist becoming ornamental. The best future profiling layer would remain pragmatic, low-friction, and close to actual runtime behavior.
