# UUID

## What this page is for
This page is about identity as a small but foundational concern.

UUID support is not one of the grand themes of Core, but its presence matters because broad runtimes eventually need stable, portable ways to name things beyond local pointer or index identity.

## Identity across boundaries
UUID-style identity belongs in Core because it helps bridge boundaries:

- between processes
- between serialized and live forms
- between tools and runtime
- between local objects and broader systems

That is exactly the kind of low-level but pervasive utility that fits a foundation package.

## Small topic, real significance
This is a good example of why the overview should remain fine-grained. UUIDs are not philosophically central in the same way as containers or threading, but they preserve a distinct runtime concern that would be lost if the set were collapsed too aggressively.

## Future direction
If Core continues moving toward richer tooling, service coordination, and portable interchange, stable identity becomes more useful. The topic is modest, but it points toward a more connected runtime ecosystem.
