# Recycling

## What this page is for
This page is about reuse as a runtime attitude.

Recycling and pooling are easy to describe narrowly as performance tricks. In Core, they also reveal something broader: the framework is willing to care about object lifetime patterns over time, not just individual allocations in isolation.

## Reuse as systems thinking
A runtime begins to feel mature when it stops asking only how to create objects and starts asking how long-lived systems should reuse them.

That is the real meaning of recycling in Core. It reflects a bias toward systems that remain active, repetitive, and cost-sensitive over long periods rather than only short transactional scripts.

## Not everything should be generalized
Core's better instinct here is restraint. Recycling is useful where it matches actual workload patterns. It becomes harmful when it turns into a universal doctrine.

The package is healthiest when it treats reuse as an explicit strategy for the right places, not as an excuse to make every lifetime story more obscure.

## Future direction
If the framework grows further toward services, persistent tools, long-running background processes, or more ambitious scheduling layers, recycling will become more architecturally visible.

That does not require turning Core into a giant pooling framework. It requires keeping the idea available where the runtime genuinely benefits from remembering yesterday's work.
