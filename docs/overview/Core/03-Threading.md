# Threading

## What this page is for
This page is about Core's concurrency stance.

Threading in Core is best understood as a practical discipline rather than a grand theory. The package wants concurrency to be available, ordinary, and useful, but not romanticized.

## Concurrency as realism
Core treats multithreading as part of normal runtime life, not as an optional modern add-on. At the same time, it does not act as though all environments deserve the same concurrency story.

That split matters. The package tries to keep one public vocabulary while admitting that some targets are richer than others, and some build modes are compatibility compromises.

## Single-thread mode matters conceptually
The existence of a single-thread mode is not just legacy trivia. It says something important about Core's self-image.

It says the framework still wants to imagine runtimes where threads are absent, limited, undesirable, or strategically excluded. That matters for portability. It matters for unusual targets. It matters for any future attempt to support a stronger single-thread GUI or event-loop identity.

The risk is that this mode becomes ceremonial rather than strategic. If it stays, it should remain an intentional constrained-runtime story, not just old scaffolding.

## Core prefers understandable concurrency
The package generally leans toward simple synchronization primitives and practical scheduling helpers rather than an abstract, endlessly customizable concurrency universe.

That fits Core's wider temperament. It usually prefers tools that communicate operational shape clearly over tools that maximize theoretical flexibility.

## Coherence over maximal sophistication
There is no sign that Core wants to compete with highly elaborate executor frameworks. Its threading mindset is closer to this:

- basic primitives should be stable
- common patterns should be easy
- the runtime should not hide that work ultimately lands on real threads with real limits

That is not glamorous, but it is architecturally consistent.

## Future pressure
The most interesting future questions are not about adding more primitives for their own sake. They are about what kind of runtime identities Core wants to support:

- stronger single-thread deployments
- more explicit worker-pool policies
- different models for background work in GUI-heavy applications
- portability to environments where normal desktop threading assumptions are weak

Those are worldview questions, not merely API gaps.
