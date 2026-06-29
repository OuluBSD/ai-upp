# VisualStateModel — Video Source Adapter Investigation

Date: 2026-06-29

## Summary

This document covers how VisualStateModel should ingest live and recorded video
sources without locking the project into one capture implementation.
No capture code is added in this task. The goal is an architecture that can
support multiple source types through a common adapter interface.

---

## Required Source Abstraction

VisualStateModel needs to consume frames from at least five source types:

| Source | Description |
|---|---|
| **Recorded frame directory** | Pre-captured `.vsm` or JPEG/PNG files; index from `manifest.json` |
| **Image sequence** | Numbered image files in a directory with no manifest |
| **Video file** | MP4/AVI/MKV — requires FFmpeg or similar |
| **Live webcam / capture card** | DirectShow (Windows), V4L2 (Linux), AVFoundation (macOS) |
| **Network video server** | MJPEG-over-HTTP, RTSP, or custom binary protocol |

For Phase 3, the priority is: **recorded frame directory** (already working via
`VsmSessionStore`) and **image sequence** (trivial extension).

---

## Proposed Headless Adapter Interface

```cpp
// Pure virtual adapter — one implementation per source type.
class VsmFrameSource {
public:
    virtual ~VsmFrameSource() {}

    // Attempt to open/connect to the source.
    virtual bool Open(const String& uri) = 0;
    // Release resources.
    virtual void Close() = 0;
    // True if source is ready to deliver frames.
    virtual bool IsReady() const = 0;

    // Frame metadata
    virtual int  GetWidth()  const = 0;
    virtual int  GetHeight() const = 0;
    virtual int  GetFPS()    const = 0; // 0 if unknown

    // Read one frame. Returns false on end-of-stream or error.
    // Timestamp is milliseconds since start; -1 if unknown.
    virtual bool ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) = 0;

    // Source diagnostics
    virtual String GetLastError() const { return String(); }
    virtual String GetSourceInfo() const { return String(); }
};
```

Implementations:
- `VsmSessionStoreSource` — reads `.vsm` files from a session; already partially
  supported via `VsmSessionStore::LoadFrameImage`.
- `VsmImageDirSource` — scans a directory for numbered image files.
- `VsmVideoFileSource` — wraps FFmpeg `AVFormatContext` (future; adds external dep).
- `VsmWebcamSource` — wraps platform capture API (future).
- `VsmNetworkSource` — MJPEG or RTSP client (future).

---

## Timestamp / Frame Index Model

Each frame should carry both:
- **Frame index** — monotonically increasing integer, zero-based.
- **Timestamp (ms)** — wall-clock or relative; `int64`, -1 if unknown.

`VsmImageBuffer` does not store its own timestamp. The caller is responsible for
associating the buffer with a `VsmChangeEvent.ts` field.

For recorded sessions, timestamps come from `manifest.json`.
For live sources, timestamps come from the capture API (best effort).

---

## Backpressure, Drop, and Missing Frame Behavior

| Scenario | Recommended behavior |
|---|---|
| Slow pipeline | Drop oldest unprocessed frame; record miss count in diagnostics |
| Missing file asset | Log warning; emit `VsmImageBuffer` of zeros (black frame) |
| Network loss | Mark source as failed; stop pipeline; log error |
| End of recording | Return `false` from `ReadFrame`; pipeline finishes normally |

The current synchronous pipeline runner (`VsmObservationPipeline`) has no
concept of backpressure — it processes one frame at a time. For live sources
a thread-safe queue between the frame source and the pipeline is needed (not
in scope for Phase 3).

---

## How Captures Become Session Directories

```
Live source
    ↓ ReadFrame() at N fps
  VsmCaptureSink (not yet implemented)
    - calls VsmSessionStore::SaveFrameImage() for each frame
    - writes events from change detection
    - saves manifest.json
    ↓
  Session directory  (same layout as replayed recordings)
    ↓
  VsmObservationPipeline (already implemented)
```

This split means the capture path and the analysis path are independent.
A recorded session can be analysed offline without the capture component.

---

## How Live Frames Feed the Observation Pipeline

For live operation (not in scope for Phase 3):

1. Frame source delivers frames to a `VsmFrameQueue` (ring buffer).
2. Observation pipeline consumer runs on a timer or background thread.
3. Pipeline processes queued frames, emitting observations to model runtime.
4. Capture sink simultaneously writes frames to the session store.

Because `VsmObservationPipeline` is synchronous and not thread-safe, a thin
adapter layer (not yet written) would be needed to drive it from a timer.

---

## Replay Determinism

Recorded sessions are deterministic: the same frame sequence always produces
the same observations, given the same rules and pipeline configuration.

Determinism is protected by:
- Session directory stores frames and manifest separately from run outputs.
- `VsmPipelineCache` uses content-based keys; the cache does not mutate
  authoritative session data.
- Live capture always writes a new session directory; replay is replay.

---

## Cross-Platform Concerns

| Platform | Webcam | Video file | Network |
|---|---|---|---|
| Windows | DirectShow / Media Foundation | FFmpeg or WMF | WinSock2 |
| Linux | V4L2 | FFmpeg | POSIX sockets |
| FreeBSD | V4L2 (limited) | FFmpeg | POSIX sockets |
| macOS | AVFoundation | FFmpeg | POSIX sockets |

U++ provides platform-agnostic socket wrappers in Core. For webcam and video
file decoding, FFmpeg is the safest cross-platform option but adds a large
external dependency. An optional package (e.g. `reference/VsmFFmpegAdapter/`)
should wrap FFmpeg and keep it out of the headless Core package.

---

## Dependency Candidates and Risks

| Candidate | Use | Risk |
|---|---|---|
| **FFmpeg** | Video decode, webcam on Linux | Large (shared libs OK); licensing (LGPL) |
| **OpenCV** | All-in-one capture + image ops | Very large; pulls in its own image types |
| **DirectShow (Windows only)** | Webcam, capture card | Platform-locked; COM boilerplate |
| **V4L2 (Linux only)** | Webcam | Platform-locked; kernel ABI |
| **MJPEG-over-HTTP** | Network camera | Requires only socket; smallest scope |

**Recommendation:** start with MJPEG-over-HTTP for network sources (Core sockets
already available) and `VsmSessionStoreSource` for recorded sessions. Defer
FFmpeg and webcam to a later package.

---

## Recommended First Implementation Target

**`VsmSessionStoreSource`** — reads `.vsm` frame images from an existing session
directory. Implementation is straightforward:

```cpp
class VsmSessionStoreSource : public VsmFrameSource {
    VsmSessionStore* store_;
    int              next_frame_ = 0;
public:
    bool Open(const String& uri) override;    // uri = session root path
    void Close() override;
    bool IsReady() const override;
    int  GetWidth()  const override;
    int  GetHeight() const override;
    int  GetFPS()    const override { return 0; } // unknown for recorded sessions
    bool ReadFrame(VsmImageBuffer& out, int64& ts_ms) override;
};
```

This gives the pipeline a frame source that works with the session store
already implemented in Phase 2, enabling replay of real frame sequences
without any external dependencies.
