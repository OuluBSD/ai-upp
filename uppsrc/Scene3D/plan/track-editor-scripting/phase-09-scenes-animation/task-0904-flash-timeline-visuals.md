# Task 0904 - Flash-Style Timeline Visuals

## Goal
Fix timeline visuals to behave like Flash: keyframes, empty frames, and tick marks.

## Scope
- Draw keyframes only where they exist; empty frames are blank.
- Optional faint placeholder at frame 0 for empty tracks.
- Major tick marks at seconds; minor ticks at frames.
- Vertical separators only at major frames (not every column).
- Distinguish keyframe vs. tween range visually.

## Implementation Notes
- Replace current always-on dots with conditional draw.
- Add tween span rendering between keyframes.
- Keep row background subtle to avoid visual noise.

## Success Criteria
- Empty tracks look empty, not filled with dots.
- Keyframes appear only where they exist.
- Major separators match kps (1s).

## Status
- Pending
