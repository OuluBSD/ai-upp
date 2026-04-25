# Core Philosophy

## What this page is for
This page is about the attitude behind `uppsrc/Core/`, not its API inventory.

Core matters because it is where the framework decides what kind of program it wants to help people write. It is the layer that teaches the rest of the tree how explicit to be, how suspicious to be of hidden cost, and how comfortable to be with platform-specific reality.

## Core is the real foundation
Core should not be read as a neutral dependency that happens to sit low in the hierarchy. It is more assertive than that.

It acts as the foundation in the architectural sense and in the cultural sense:

- higher layers assume its runtime vocabulary
- its tradeoffs leak upward into container choice, serialization style, diagnostics, and scheduling
- its worldview tends to become the worldview of the application built on top of it

That is why arguments about Core are rarely small. They are usually arguments about the character of the whole framework.

## More than a utility library
A common instinct is to compare Core to STL plus some helpers. That misses the point.

Core behaves more like a replacement worldview. It does not merely fill gaps. It proposes different defaults:

- ownership should be visible
- text encodings should not be hand-waved away
- debugging should be stricter than release
- platform differences should be acknowledged instead of cosmetically hidden
- convenience should be earned by preserving meaning, not by erasing it

That makes Core opinionated. It is not trying to disappear behind standard C++.

## Explicitness is treated as a moral good
Core repeatedly chooses named distinctions over generalized ambiguity.

That choice is visible in the framework's habits:

- different container families say different things about lifetime and storage
- different runtime modes exist instead of one fake universal mode
- debug and release are allowed to express different priorities
- historical compatibility is often kept visible rather than silently dissolved

This makes the package larger, but it also keeps important decisions legible.

## Debug is allowed to be judgmental
Core's debug philosophy is one of its clearest statements of intent.

The package does not behave as though every build should offer the same emotional experience. Debug is where misuse should surface quickly. Release is where the program should remain viable. That split is not a flaw to hide; it is one of the package's more mature instincts.

The deeper message is that software quality comes partly from designing different enforcement styles for different phases of development.

## Hardware-near thinking
Core often reads like a runtime written by people who want to remember the machine.

That does not mean everything must be low level. It means abstractions should not destroy the operator's sense of storage, copying, scheduling, synchronization, and platform cost. Core wants the programmer to stay close enough to the hardware to make intentional choices.

## Platform realism over fake purity
Core is strongest when it refuses to pretend that all targets are the same.

Desktop Windows, POSIX, UWP, single-thread mode, and constrained environments are not interchangeable. Core's better habit is to offer a common language without faking identical behavior. That realism is a strength, especially if the package is asked to stretch toward stranger targets later.

## Core keeps history in the open
Core is not a minimalist manifesto. It is an evolving foundation with old layers, new layers, experiments, and compatibility baggage.

That is not embarrassing unless the overview tries to deny it. The honest reading is that Core is a place where the framework keeps both its central runtime beliefs and the evidence of how those beliefs changed over time.

## Where this should lead
The philosophical challenge for Core is not to become smaller at any cost. It is to become clearer about which parts are:

- permanent foundation
- compatibility bridge
- strategic experiment
- future-facing extension point

That clarity matters more than tidiness.
