# Time

## What this page is for
This page is about time as a systems problem.

Time in a runtime is never just clocks and formatting. It sits at the boundary between machines, people, logs, files, protocols, and platform conventions. That makes it an inherently architectural concern.

## Time resists simplification
Core's treatment of time belongs in the foundation because time is one of the easiest places for a framework to lie to itself.

Civil time, elapsed time, scheduling time, and serialized time are not the same thing. A serious runtime does better when it admits those tensions instead of hiding them under one generic abstraction.

## Human time versus machine time
This is one of the places where Core's practical instincts matter.

Applications need both:

- machine-oriented timing for measurement and scheduling
- human-oriented time for display, interchange, and records

The framework is healthier when it keeps both stories available without pretending they collapse into each other.

## Future direction
The future pressure here is broader portability and operational trust:

- more consistent behavior across unusual targets
- better integration with diagnostics and service runtimes
- clearer handling in environments where local time assumptions are weak or unstable

Time is one of those topics that becomes more important the farther the framework reaches.
