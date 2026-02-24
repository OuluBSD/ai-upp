# GLDraw and the SDL2.0 Rainbow
**Date Span:** 2013-09-01 to 2013-09-30

### High-Level GPU Drawing: GLDraw
Launched the **GLDraw** package, a cross-platform hardware acceleration layer for rendering complex UI and data. This was supported by the arrival of geometry shaders in `CoreGl` and the development of the **SDL20** and **SDL20GL** Rainbow backends.

### Advanced Process and Network Control
`LocalProcess` gained the ability to read **stderr separately** from stdout, providing professional control for system utilities. The **Skylark** web framework added `Http::GetPeerAddr()` for reliable client IP identification.

### SQL Expression Expansion
`SqlExp` was significantly upgraded with support for **Insert...From...GroupBy...Having** and a more intuitive `Select(...).From()` syntax. `SqlMassInsert` was also refined to handle `ValueMap` parameters and the `SelectAll` directive.

### Tooling and Typography
TheIDE added the **Alt+I** shortcut to jump to type definitions and implemented SQL syntax highlighting for `.ddl` files. The typographical foundation was expanded with the **DroidFonts** and **FT_fontsys** plugins for high-quality mobile and Linux rendering.
