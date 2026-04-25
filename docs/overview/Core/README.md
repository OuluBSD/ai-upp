# Core Overview

This directory is a thinking layer, not a reference manual.

The pages here treat `uppsrc/Core/` as the place where the framework decides what kind of runtime it wants to be: explicit or hidden, portable or merely abstract, cautious or ambitious, conservative or experimental. They are meant to describe intention, pressure, tradeoffs, and possible directions, not to freeze every current implementation detail into prose.

That is why the set is fine-grained. The number of files is intentional. Core carries many separate ideas, and it is better to preserve those ideas as separate viewpoints than to flatten them into one broad essay.

Use these pages for:

- understanding the framework's runtime worldview
- seeing where the package is historically layered or internally tense
- identifying where future expansion still feels plausible
- discussing what should stay, what should harden, and what should change

Do not use these pages as the final authority on concrete behavior. For exact semantics, headers, source files, and package manifests still win.

Core overview documents:

- [00-Core-Philosophy.md](00-Core-Philosophy.md): why Core behaves like a foundation instead of a utility shelf.
- [01-Architecture.md](01-Architecture.md): how the package acts as a runtime center of gravity.
- [02-Memory-and-Performance.md](02-Memory-and-Performance.md): the hardware-near temperament behind Core decisions.
- [03-Threading.md](03-Threading.md): concurrency as a practical tool, not a purity exercise.
- [04-Strings-and-Text.md](04-Strings-and-Text.md): text handling as an explicit policy choice.
- [05-Paths-and-Config.md](05-Paths-and-Config.md): deployment realism, file placement, and platform honesty.
- [06-Streams.md](06-Streams.md): streams as the preferred dataflow mental model.
- [07-Logging.md](07-Logging.md): runtime memory, debugging culture, and operational visibility.
- [08-Profiling.md](08-Profiling.md): observing cost without turning the runtime into a laboratory.
- [09-Containers.md](09-Containers.md): ownership-visible containers as a worldview.
- [10-Recycling.md](10-Recycling.md): reuse, pooling, and avoiding waste in long-lived systems.
- [11-Callbacks-and-Events.md](11-Callbacks-and-Events.md): callable layers as both bridge and burden.
- [12-Time.md](12-Time.md): time as a practical systems problem rather than a pure library feature.
- [13-Value.md](13-Value.md): dynamic values as a runtime negotiation layer.
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md): how Core crosses human-readable and machine-readable boundaries.
- [15-Color.md](15-Color.md): why color lives in Core at all.
- [16-Geometry-Primitives.md](16-Geometry-Primitives.md): the geometric assumptions the rest of the UI stack inherits.
- [17-Localization.md](17-Localization.md): language support as framework responsibility, not decoration.
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md): parsing and interchange as structural infrastructure.
- [19-Visitor.md](19-Visitor.md): reflective traversal as an unfinished but persistent idea.
- [20-Pointer-Safety.md](20-Pointer-Safety.md): lifetime visibility without surrendering to heavy ownership models.
- [21-Compression.md](21-Compression.md): self-contained runtime packaging and transport pragmatism.
- [22-Topics-Help.md](22-Topics-Help.md): documentation embedded in the platform's own runtime culture.
- [23-CoWork.md](23-CoWork.md): simple parallelism, useful constraints, and the ceiling of the current model.
- [24-UUID.md](24-UUID.md): identity as a small but necessary runtime concern.
- [25-Caching.md](25-Caching.md): memory, reuse, and where Core might want stronger cache policy.
- [26-Hashing.md](26-Hashing.md): structural identity, digests, and trust boundaries.
- [27-Networking.md](27-Networking.md): the current transport mindset and the directions still left open.
- [28-Daemon.md](28-Daemon.md): Core's service-host instincts and how far they might reasonably grow.
- [29-Runtime-Linking.md](29-Runtime-Linking.md): optionality, modularity, and late binding.
- [30-Windows-Specific.md](30-Windows-Specific.md): platform-specific honesty on Windows, including legacy and compatibility debt.
- [Core_docs_overview_general.md](Core_docs_overview_general.md): broader framing for the entire package as a design space.
