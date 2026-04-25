# Value

## What this page is for
This page is about dynamic values as a runtime negotiation layer.

Core is not only about static, explicit types. It also keeps a place for structured dynamism, and that is strategically important.

## Dynamism inside an explicit runtime
`Value`-style facilities only make sense in Core if they serve a clear purpose. The likely purpose is not to replace static design. It is to let dissimilar parts of the framework exchange data without forcing every interaction into bespoke compile-time coupling.

That makes the value layer a negotiation surface:

- between tools and runtime
- between serialization and live objects
- between generic infrastructure and domain-specific data

## Why this does not contradict explicitness
At first glance, a dynamic value system seems to oppose Core's explicit temperament. In practice it can support it, if used carefully.

The key difference is whether dynamism hides structure or makes cross-boundary structure manageable. In Core, the healthier reading is the second one. The package keeps a dynamic layer because a large framework needs one, not because it wants to become vague everywhere.

## Future direction
If Core continues to evolve toward more tooling, service, reflection, or schema-like behavior, the value layer will likely matter even more.

The real challenge is keeping it disciplined enough that it remains an interoperability tool rather than a universal escape hatch.
