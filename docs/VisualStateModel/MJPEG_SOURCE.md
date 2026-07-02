# VisualStateModel — MJPEG Source Prototype

## Overview

`VsmMjpegParser` is a headless multipart/x-mixed-replace boundary parser added
to `uppsrc/VisualStateModel`. It parses MJPEG streams into raw JPEG payload
bytes without any GUI or network dependency.

`reference/VisualStateMjpegSource/` is a buildable prototype that wraps the
parser as a `VsmFrameSource` adapter and integrates it with `VsmCaptureSink`.

---

## VsmMjpegParser (headless, Core only)

### API

```cpp
VsmMjpegParser parser;
parser.SetLog(&log);
parser.Reset("myboundary");  // set boundary string (without leading "--")
parser.Feed(raw_http_body);  // feed bytes from HTTP response

String payload; VsmMjpegPartHeader header;
while(parser.ExtractNextPayload(payload, header)) {
    // header.content_type   == "image/jpeg"
    // header.content_length == payload.GetCount() (if available)
    // payload               == raw JPEG bytes
}
```

### Features

- State machine: `FIND_BOUNDARY → READ_HEADERS → READ_PAYLOAD`
- Supports `Content-Length`-based extraction and boundary-search fallback
- Handles incremental `Feed()` calls (stream can be split arbitrarily)
- `GetPayloadCount()` / `GetSkipCount()` for diagnostics

### Synthetic Stream Generator

```cpp
String stream = VsmMakeSyntheticMjpeg(boundary, fake_jpeg_bytes, frame_count);
```

Generates a valid in-memory MJPEG stream for deterministic testing.

---

## JPEG Decode Blocker

`VsmMjpegParser` extracts raw JPEG payload bytes only.

Converting JPEG bytes to `VsmImageBuffer` requires `Draw::JPGRaster`, which
lives in the `Draw` package — a GUI dependency excluded from the headless
`uppsrc/VisualStateModel` package.

**To enable JPEG decode:**

1. Add `Draw` to the consuming package's `.upp` deps.
2. In `ReadFrame()`:
   ```cpp
   Image img = JPGRaster().LoadString(payload);
   if(img.IsEmpty()) return false;
   // Convert RGBA → VsmImageBuffer
   VsmImageBuffer buf;
   buf.Create(img.GetWidth(), img.GetHeight(), 1);
   for(int y = 0; y < img.GetHeight(); y++)
       for(int x = 0; x < img.GetWidth(); x++) {
           RGBA c = img[y][x];
           buf.Set(x, y, 0, (c.r + c.g + c.b) / 3);
       }
   out_frame = pick(buf);
   ```

Until decode is wired up, `VsmMjpegSource::ReadFrame()` returns a gray
placeholder `VsmImageBuffer` of the declared source dimensions.

---

## Reference Package (`reference/VisualStateMjpegSource/`)

Tests:

1. **Boundary parser**: 5-frame synthetic MJPEG → extract all 5 payloads OK
2. **VsmMjpegSource** in test-stream mode: 5 frames, placeholder pixels
3. **Capture sink integration**: records 5 frames into a new session, replays all

```sh
bin/build.exe -m 7 -j12 VisualStateMjpegSource
bin\VisualStateMjpegSource.exe
```

Expected:
```
All MJPEG prototype checks passed.
```

---

## Live HTTP Source

`Open(url)` in test-stream mode works deterministically.

For live HTTP:
1. Open TCP connection to the MJPEG URL using `TcpSocket` (available in Core).
2. Issue HTTP GET request.
3. Parse `Content-Type: multipart/x-mixed-replace; boundary=XXXX` response header
   to extract the boundary string.
4. Feed response body bytes to `VsmMjpegParser` in a read loop.
5. Call `ExtractNextPayload()` after each feed.

This is documented but not implemented in the prototype to avoid live network
dependency in tests.

---

## Headless Constraint

`uppsrc/VisualStateModel` must remain GUI-free. `VsmMjpegParser` has no Draw,
CtrlLib, Docking, or network library dependency. Verify:

```sh
rg "CtrlLib|Docking|TopWindow|Draw\b" uppsrc/VisualStateModel
```

Must return nothing (Draw::Point/Rect used in Types.h is acceptable since
`Rect` is defined in Core, not Draw proper; the `Draw::` qualified symbols
must not appear).
