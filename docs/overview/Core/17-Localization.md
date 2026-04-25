# Localization

## What this page is for
This page is about language support as a first-class framework responsibility.

Localization living in Core says something important: the project does not treat human language as an afterthought best postponed to application code.

## Language belongs in the foundation
A framework that wants to support real applications eventually has to decide whether localization is ornamental or structural.

Core's placement suggests the structural reading. Translation, language identity, and locale-sensitive behavior affect text, help, UI, storage, and diagnostics. Keeping them near the foundation helps the rest of the stack behave like one system rather than many local patches.

## More cultural than technical
Localization is not just a parsing or lookup problem. It is one of the places where a runtime admits that software is used by people in different linguistic contexts.

That is why its presence in Core feels philosophically right. It keeps the framework from adopting an overly narrow, monocultural idea of runtime concerns.

## Future direction
This area becomes more interesting if Core grows toward:

- stronger reuse of translation assets
- closer coupling between embedded help and language systems
- more portable tooling flows

Localization is one of the quiet places where the framework's maturity shows.
