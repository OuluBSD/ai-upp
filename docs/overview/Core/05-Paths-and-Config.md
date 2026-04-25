# Paths And Config

## What this page is for
This page is about Core's attitude toward deployment reality.

Paths and config are where a framework stops being an abstract runtime and starts admitting how software is actually shipped, installed, copied, bundled, and misused on real systems.

## Platform realism lives here
Core's path and config story matters because it resists fake uniformity.

File placement, executable-relative resources, user-level configuration, and platform conventions do not line up neatly across systems. A serious runtime has to decide whether to hide that mess or describe it with enough honesty that applications can still behave intentionally.

Core tends to prefer the second approach.

## Portable application thinking
There is a recurring portable-application instinct in Core's worldview. The package often feels sympathetic to software that wants to carry more of its own environment with it instead of assuming a perfectly managed host system.

That instinct is important because it makes the runtime useful in unconventional deployment conditions, not just standard desktop installations.

## Configuration is architectural
Config placement is not a mere helper topic. It expresses what the framework believes about application identity.

Does the application belong beside its executable, inside user space, inside a managed platform directory, or inside some hybrid compromise? Core's answer is contextual rather than ideological, and that is a strength. It implies the runtime cares more about practical legitimacy than elegant universal rules.

## Future pressure
This area becomes more important if the framework reaches toward:

- WebAssembly-style packaged runtimes
- Android and mobile filesystem rules
- DOS-like or kiosk-style deployments
- service processes with stricter host integration

Those targets will stress not only technical path handling, but also the framework's assumptions about where an application is allowed to live.
