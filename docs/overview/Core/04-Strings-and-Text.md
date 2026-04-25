# Strings And Text

## What this page is for
This page is about why text handling belongs to Core's worldview.

String types are never just storage classes in a framework runtime. They reveal how seriously the project takes encoding, interchange, operating-system boundaries, and the cost of pretending those things do not exist.

## Text is policy
Core behaves as though text should be handled with explicit policy rather than with one vague universal string story.

That choice is significant. It rejects the fantasy that all textual data is morally the same. Human language, byte transport, platform APIs, and internal representation place different demands on the runtime, and Core prefers to keep those distinctions visible.

## Explicit representation is healthier
The package's separate textual paths suggest a simple idea: encoding choices should not disappear just because developers are tired of thinking about them.

That can feel less convenient than a single all-purpose abstraction, but it usually produces better engineering judgment. Core's text culture is therefore aligned with its broader preference for explicitness over sentimental simplicity.

## Platform boundaries make this unavoidable
Text is where portability lies most easily.

Once a framework spans Windows, POSIX, different filesystem behaviors, localization flows, network protocols, and tooling output, any naive text model breaks down. Core's better instinct is to accept that pain early and give the rest of the stack a more deliberate language for dealing with it.

## Future direction
The important future question is not whether text APIs should become prettier. It is whether Core can keep a serious, explicit text model while still supporting more environments:

- constrained targets
- more aggressive portability work
- browser-hosted runtimes
- platforms with unusual wide-character expectations

If Core remains honest here, the rest of the framework stays healthier.
