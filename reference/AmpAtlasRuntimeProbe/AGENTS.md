# AmpAtlasRuntimeProbe

Headless smoke test for loading an external AMP template atlas and uploading
its compact grayscale pixels to a native C++ AMP accelerator.

The probe requires a previously saved device inventory when selecting a
device. It does not enumerate devices in debugger mode and does not activate
the compatibility backend.

For direct decoded-frame regression, use
`--amp-real-video-regression <video> --amp-regression-times <times.txt>`.
Each non-comment line is `timestamp_ms [L|R]`; optionally add
`--amp-video-focus-rect x,y,w,h` to bound the pre-match region. The video path
uses `VsmVideoFileFrameSource`/libavcodec and does not create JPEG frames.
