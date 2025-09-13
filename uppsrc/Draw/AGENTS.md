AGENTS

Scope
- Applies to `uppsrc/Draw`.

Purpose
- Fundamental 2D graphics (fonts, images, raster operations) and Draw API used by GUI and PDF/SVG layers.

Key Areas
- Fonts and text: platform font backends (Win32/Freetype/Cocoa), shaping helpers.
- Images: `Image`, raster codec helpers (`Raster*`, palette, scaling, ops).
- Draw API: primitives, text, any-drawers, chameleon theme, SIMD helpers.

Conventions
- Keep platform font code in separate files; expose abstracted behavior via `Draw.h`.

.upp Notes
- List `AGENTS.md` first in `Draw.upp`.

