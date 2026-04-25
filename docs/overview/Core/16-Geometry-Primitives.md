# Geometry Primitives

## What this page is for
This page is about geometric assumptions near the runtime floor.

Simple geometric types in Core show that the framework expects space, placement, and rectangular reasoning to be common enough that they should not depend on a later UI package.

## Foundational geometry is really about the UI worldview
These primitives suggest that the framework's center of gravity includes application layout and screen-oriented reasoning, even in the non-GUI foundation.

That is not a contradiction. It reflects a practical belief that ordinary software spends a lot of time dealing with positions, extents, clipping, and alignment, and that these ideas deserve stable shared language early in the stack.

## Deliberate scope
The interesting part is what Core geometry likely does not try to be.

It does not need to become a universal computational-geometry system. Its role is narrower and more useful: give the rest of the framework durable primitives for common spatial reasoning without pretending to solve every geometric domain.

## Future direction
If the framework keeps reaching into more rendering, visualization, or unusual-device territory, this layer may need to stay simple while also becoming more portable across different coordinate assumptions.

That would be a healthy tension to keep visible.
