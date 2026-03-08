# Media Foundations and UI Refinement
**Date Span:** 2010-12-01 to 2010-12-31

### Native Media Support
Launched the **Media** package, providing built-in audio and video playback capabilities. This was supported by the `MediaPlayer` control and the arrival of `plugin/portaudio` and `Sound` in the Bazaar, creating a complete A/V stack.

### UI Performance and Polish
Overhauled `FileSel` with lazy icon loading, making large directory navigation significantly faster. `TabBar` received deep fixes for its stacking and dragging logic, while `LabelBox` gained visual support for the `Disabled` state.

### Massive Bazaar Expansion
Introduced a flurry of specialized developer tools: `PlotCtrl`, `AutoScroller`, `CtrlProp`, and dedicated editors for `Point`, `Rect`, and `LogPos`. `SliderCtrlX` offered an enhanced alternative for rotary and linear input.

### Tooling and Core Maturation
TheIDE added structured flags to the package editor and a new "selection to ASCII" conversion tool. `SqlExp` was hardened with `SQLT_TIMESTAMP` for Oracle and debug-mode column validation. Core added `DayOfYear`, `StrToTime`, and more `Callback` combinations.
