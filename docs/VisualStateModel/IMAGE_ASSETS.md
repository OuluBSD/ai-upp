# VisualStateModel — Image Assets

## Overview

`uppsrc/VisualStateModel` is a headless package — it must not depend on
`Draw`, `CtrlLib`, or any GUI component. U++ `Image` and `ImageBuffer` live
in the `Draw` package, which is a GUI-level dependency. Therefore
VisualStateModel uses its own internal pixel buffer type: `VsmImageBuffer`.

---

## VsmImageBuffer

Defined in `uppsrc/VisualStateModel/ImageBuffer.h`.

```
struct VsmImageBuffer {
    int          width;
    int          height;
    int          channels;  // 1=gray, 3=RGB, 4=RGBA
    Vector<byte> pixels;    // row-major, channels per pixel
};
```

### Key methods

| Method | Description |
|---|---|
| `Create(w, h, ch)` | Allocate zeroed pixel buffer |
| `Get(x, y, ch)` | Read one channel value |
| `Set(x, y, v, ch)` | Write one channel value |
| `Save(path)` | Write to .vsm binary file |
| `Load(path)` | Read from .vsm binary file |
| `Info()` | Returns `"WxH ch=N"` string |

### Synthetic constructors (for testing)

| Method | Description |
|---|---|
| `MakeSolid(w, h, value, ch)` | Solid-color buffer (all pixels same value) |
| `MakeGradient(w, h)` | Horizontal grayscale gradient 0→255 |
| `MakeCheckerboard(w, h, cell_size)` | Black/white checkerboard pattern |

---

## VSM1 Binary Format

Files use the extension `.vsm`. Format:

```
Offset  Size  Field
0       4     Magic: "VSM1"
4       4     Width  (uint32 LE)
8       4     Height (uint32 LE)
12      4     Channels (uint32 LE)
16      W*H*Ch  Pixel data, row-major, channels per pixel
```

Total file size: `16 + W × H × channels` bytes.

---

## Session Store Integration

`VsmSessionStore` (in `SessionStorage.h`) has four image I/O methods:

| Method | Description |
|---|---|
| `SaveFrameImage(index, img)` | Write frame image; updates manifest with `.vsm` format |
| `LoadFrameImage(index, out)` | Load frame image from session directory |
| `SaveCropImage(region_id, img)` | Write crop image for a region |
| `LoadCropImage(region_id, out)` | Load crop image for a region |

The legacy `AllocateFrame` / `AllocateCrop` still create `.placeholder` text
files for compatibility with sessions that have no real image data.

`VsmSessionManifest::image_format` is updated to `"vsm"` when the first real
image is saved. Placeholder sessions remain `"placeholder"`.

---

## Workbench Display

The GUI workbench (`reference/VisualStateWorkbench`) converts `VsmImageBuffer`
to U++ `Image` (Draw package) at the boundary layer only — inside
`FrameCanvas` — to avoid polluting the headless package.

The conversion path (not yet implemented — see Task 0017):

```
VsmImageBuffer  →  Upp::ImageBuffer (Draw)  →  Upp::Image  →  display
```

---

## Why Not U++ Image in the Headless Package?

| Package | Allowed in headless? | Reason |
|---|---|---|
| Core | Yes | No GUI, no platform dependency |
| Draw | No | Pulls in GDI/X11/Quartz depending on platform |
| CtrlLib | No | Requires window system |
| Docking | No | Requires CtrlLib |

`VsmImageBuffer` stays in Core-only territory and can be used in console
tools, test runners, and pipeline logic without a window.

---

## Test Coverage

Task 0015 adds `TestImageAssets()` to `reference/VisualStateModelTest`:

- `MakeSolid` dimensions and pixel value
- `MakeGradient` edge values (0 and 255)
- `MakeCheckerboard` cell boundary
- `Save` / `Load` round-trip (verify file binary format)
- `VsmSessionStore::SaveFrameImage` / `LoadFrameImage`
- `VsmSessionStore::SaveCropImage` / `LoadCropImage`
- Manifest `image_format` update after save

All 12 test suites pass as of task 0015.
