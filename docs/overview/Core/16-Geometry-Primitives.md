# Geometry Primitives

## What this covers
This file documents the fundamental geometric value types in `uppsrc/Core/Gtypes.h`: `Point_`, `Size_`, `Rect_`, their standard typedefs, and the helper math that lives alongside them.

## Main types
Core defines templated 2D primitives:

- `Point_<T>`
- `Size_<T>`
- `Rect_<T>`

and then standard aliases:

- integer: `Point`, `Size`, `Rect`
- floating-point: `Pointf`, `Sizef`, `Rectf`
- 16-bit and 64-bit integer variants

These types are value-semantic, serializable, JSON/XML-capable, and Value-compatible.

## Semantics
### `Size_<T>`
Represents extent, not position. It supports arithmetic, scalar/vector products, `Squared`, and `Length`.

### `Point_<T>`
Represents position/vector in 2D. It supports arithmetic with both points and sizes, plus conversion to/from `Size_`.

### `Rect_<T>`
Represents a 2D box by edges: `left`, `top`, `right`, `bottom`. The semantics are edge-based, not center-based.

Observed behaviors from code:

- `IsEmpty()` is true when `right <= left` or `bottom <= top`
- helper constructors such as `RectC` interpret `(x, y, cx, cy)` as origin plus size
- there are many convenience accessors for corners, center points, size, and translated variants

## Math helpers
`Gtypes.cpp` adds floating-point geometry helpers such as:

- `GetFitSize`
- `Mid`
- `Orthogonal`
- `Normalize`
- `Polar`
- `Direction`
- `Distance`

So this layer is not only passive structs. It is a small, general-purpose 2D geometry toolkit.

## Value and serialization integration
`ValueUtil.cpp` registers:

- `Point`, `Point64`, `Pointf`
- `Size`, `Size64`, `Sizef`
- `Rect`, `Rect64`, `Rectf`

Small types use `SvoRegister` where possible; `Rect` variants are registered as rich values.

## Current vs fork-specific scope
These 2D primitives are central Core types.

By contrast, a type named `Volume` is not part of base `uppsrc/Core/Gtypes.h` in this repository snapshot. If you encounter `Volume` elsewhere in the tree, treat it as package-specific or fork-specific to that subsystem, not as part of the foundational Core geometry surface documented here.

## See also
- [13-Value.md](13-Value.md)
- [15-Color.md](15-Color.md)
- [18-Parsers-and-Serialization.md](18-Parsers-and-Serialization.md)
- [README.md](README.md)
