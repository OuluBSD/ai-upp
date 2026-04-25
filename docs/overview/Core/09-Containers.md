# Containers

## What this page is for
This page is about why container choice in Core is ideological as well as technical.

A container library always carries beliefs about ownership, movement, stability, allocation, and what kinds of convenience are worth the confusion they create.

## Ownership should be visible
Core's container story strongly suggests that ownership belongs in the type-level conversation.

That is one of the package's most important differences from more generic C++ habits. Instead of treating lifetime as something to infer from convention, Core tends to make storage and ownership part of the public semantic surface.

This can feel heavier at first, but it pays for itself by making architectural intent easier to read.

## Containers as worldview
The container layer is one of the clearest examples of Core acting as a replacement worldview rather than a helper collection.

The point is not merely to have different data structures. The point is to encourage a particular style of programming:

- values should behave like values
- owning aggregates should say so
- structural choices should communicate operational consequences

That is a framework ethic, not just a template API preference.

## Why this matters for the whole tree
Once higher packages adopt Core containers, they also adopt Core's assumptions about mutability, ownership, and interchange. That shapes code review, bug patterns, and future extension work.

This is why container discussions inside Core are rarely local. They quietly influence the character of the entire codebase.

## Future direction
The real question for the future is not whether Core should imitate other ecosystems more closely. It is whether it can preserve ownership-visible semantics while still improving interoperability and reducing accidental friction.

That is a delicate problem, but a worthwhile one.
