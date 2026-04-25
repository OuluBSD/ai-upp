# Color

## What this page is for
This page is about why color appears in Core at all.

At first glance, color looks like a higher-level UI topic. Its presence in the foundation suggests that the framework considers certain visual primitives basic enough to deserve runtime-level stability.

## Visual semantics begin early
Color in Core implies that the framework's UI stack is not meant to be bolted on as a completely separate worldview. Some visual semantics are considered foundational.

That is reasonable. Applications do not experience geometry and color as exotic features. They experience them as ordinary language. Putting basic visual primitives in Core gives the rest of the stack a stable baseline.

## Practical, not artistic
The important reading here is not that Core wants to become a graphics theory package. It is that the runtime acknowledges some visual concepts as everyday engineering material.

This matches the framework's general habit of keeping frequently reused semantics near the center rather than scattering them into many thin dependencies.

## Future direction
Color will matter more if the framework keeps stretching across desktop UI, rendering experiments, alternative display environments, and constrained targets. The foundational question is whether Core can keep these primitives simple without making them too provincial.
