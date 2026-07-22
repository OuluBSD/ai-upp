# AmpTemplateProbe

Headless probe for the native Windows C++ AMP backend. It is intentionally
separate from the video recognition application and must fail explicitly when
native AMP is not compiled in; it must not silently use `AMPCompat`.
