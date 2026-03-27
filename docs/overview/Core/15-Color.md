# Color

## What this covers
This file documents the color primitives in Core: `RGBA`, `Color`, special colors, dark-theme-aware colors, and color conversion helpers.

## Main types
[`uppsrc/Core/Color.h`](../../../uppsrc/Core/Color.h) defines:

- `RGBA`: raw 8-bit channel struct
- `Color`: Value-compatible color abstraction
- `SColor`: static/global special color wrapper
- `AColor`: auto-dark-mode-aware color wrapper

`Color` is a real `ValueType<Color, COLOR_V, ...>`, so it participates in the `Value` system directly.

## Semantics
`Color` stores a `dword` with flag bits:

- `SPECIAL` for non-ordinary color values
- `SCOLOR` for function-defined special colors
- `ACOLOR` for colors that are auto-adjusted in dark mode

That means `Color` is more than an RGB triple. Some values are symbolic or environment-sensitive.

## Conversions and helpers
Core includes:

- RGB/HSV and CMYK conversion helpers
- luminance and contrast helpers
- blending, lerp, grayscale, dark/light classification
- HTML/text conversion through `ColorToHtml` and `ColorFromText`

These are utility-level operations, not just UI sugar.

## Platform behavior
`RGBA` field order changes on macOS:

- macOS stores `a, r, g, b`
- other platforms store `r, g, b, a`

The public helper functions hide most of that, but the struct layout difference is real and worth remembering for raw-memory code.

## Current vs legacy
`Color` is current and central to the wider framework, even though this package is nominally non-GUI. The special-color and dark-theme hooks make it broader than a simple numeric type.

## See also
- [13-Value.md](13-Value.md)
- [14-Formatting-and-Conversion.md](14-Formatting-and-Conversion.md)
- [16-Geometry-Primitives.md](16-Geometry-Primitives.md)
- [17-Localization.md](17-Localization.md)
