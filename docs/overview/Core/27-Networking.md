# Networking

## What this page is for
This page is about Core's current transport mindset and the directions it leaves open.

Networking is one of the clearest places where the difference between "what exists now" and "what Core might grow into" matters.

## A practical, not universal, network story
Core's network identity appears grounded in ordinary stream-oriented connectivity. That gives the package a useful baseline without pretending it already owns every transport problem.

This is good architectural discipline. A runtime should know the difference between having a serious network foothold and claiming the entire networking landscape.

## Transport scope should stay visible
The overview should preserve the fact that transport choices are not interchangeable.

TCP, UDP, WebSocket, ENet-style reliability models, and local-service transport each carry different assumptions about latency, ordering, connection state, and failure. Core is healthier when it states what it currently privileges and what remains open rather than flattening everything into a generic "networking" label.

## Why this matters strategically
Networking is one of the places where future expansion could materially change Core's character.

Possible directions include:

- broader datagram-oriented support
- ENet or game-oriented transport work
- daemon and local-service integration
- MCP-style service patterns
- more portable communication models for constrained runtimes

These are not just features. They shape what kind of platform Core could become.

## Present strength, future tension
Core does not need to solve every transport problem today to justify keeping this page separate. The page matters because it captures a distinct tension between the current stream-oriented runtime and the broader communication roles the package may eventually assume.
