# CoWork

## What this page is for
This page is about Core's lightweight parallel-work philosophy.

`CoWork` is interesting not because it is the final answer to task scheduling, but because it reveals what kind of concurrency convenience Core considers acceptable.

## Convenience with visible limits
The likely design attitude is simple: parallel work should be easy to reach for, but the runtime should not pretend that convenience has infinite elasticity.

That is a good Core instinct. It values practicality while leaving operational limits visible. The result is less glamorous than a huge executor ecosystem, but often more honest.

## A deliberately bounded model
`CoWork` suggests a style of framework parallelism that is:

- local
- direct
- useful for fork-join patterns
- intentionally narrower than a complete scheduling ideology

That boundedness should not be treated as failure. It is part of the package's preference for understandable machinery over grand abstraction.

## Future direction
This is also one of the clearest places where Core may eventually want more:

- multiple pools or policies
- stronger integration with service runtimes
- clearer saturation behavior for larger systems
- better stories for constrained or single-thread targets

The overview should preserve both truths: the current model is useful, and it is probably not the endpoint.
