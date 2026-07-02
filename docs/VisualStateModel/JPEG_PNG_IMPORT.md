# JPEG / PNG Sequence Import

## Overview

`JpegSequenceImporter` imports a directory of `.jpg`, `.jpeg`, or `.png` images
into a new VisualStateModel session store. It is part of the
`reference/VisualStateWorkbench` package and **requires the Draw package** — it
cannot be used in the headless `uppsrc/VisualStateModel` package.

## Usage

```cpp
#include "JpegSequenceImporter.h"  // workbench package only

VsmJpegImportOptions opts;
opts.source_dir   = "C:/frames/capture001";
opts.output_dir   = AppendFileName(GetTempPath(), "vsm_import_001");
opts.fps          = 30;          // used only for timestamp assignment
opts.sort_numeric = true;        // sort by leading numeric prefix
opts.grayscale    = true;        // convert RGBA → 8-bit grayscale

JpegSequenceImporter importer;
importer.SetLog(&log);
VsmJpegImportResult res = importer.Import(opts);

if(res.success)
    OpenSessionPath(res.output_dir);
```

## File Naming Conventions

Files are collected from `source_dir` matching `*.jpg`, `*.jpeg`, `*.png`
(case-insensitive on Windows).

When `sort_numeric = true` (the default), files are sorted by the **first
contiguous run of digits** in the filename, regardless of prefix or suffix:

| Filename         | Numeric key |
|------------------|-------------|
| `0001_frame.jpg` | 1           |
| `frame_0010.png` | 10          |
| `00000023.jpg`   | 23          |
| `img-100.jpeg`   | 100         |

Files without any digit sort to key 0 and appear first; ties are broken
alphabetically by filename.

When `sort_numeric = false`, files are sorted alphabetically by filename.

## Grayscale Conversion

When `opts.grayscale = true`, each RGBA pixel is converted to an 8-bit luma
value using the BT.601 integer approximation:

```
Y = (R × 77 + G × 150 + B × 29) >> 8
```

The result is stored in a `VsmImageBuffer` with `channels = 1`.
This matches the format expected by headless preprocessing (grayscale normalize,
fingerprint extraction, etc.).

When `opts.grayscale = false`, the buffer is stored as `channels = 3` (RGB).

## Frame Timestamps

Timestamps are assigned as:

```
ts_ms = frame_index × (1000 / fps)
```

For `fps = 30`, frame 0 → 0 ms, frame 1 → 33 ms, frame 2 → 66 ms, etc.
Only successfully imported frames consume a `frame_index`; skipped files leave
no gap.

## Decode Path

- `.jpg` / `.jpeg` — decoded via `JPGRaster` (Draw package, `StreamRaster`
  subclass). The raster is opened from a `FileIn` stream, then scan lines are
  read sequentially via `GetLine(y)` to fill an `ImageBuffer`.
- `.png` — decoded via `PNGRaster` (same mechanism).

Files that fail to open or decode are skipped and recorded as warnings in
`VsmJpegImportResult::warnings`. Import continues with remaining files.

## Workbench Integration

`MainWindow::OnImportImageSequence()` auto-detects the contents of the selected
directory and dispatches:

| Condition                    | Action                                      |
|------------------------------|---------------------------------------------|
| Has `.jpg`/`.png`, no `.vsm` | `RunJpegImport()` — calls `JpegSequenceImporter` |
| Has `.vsm`                   | `RunVsmImport()` — calls `VsmImageSequenceImporter` |
| Neither                      | `PromptOK("No .vsm or .jpg/.png files found.")` |

When both `.vsm` and JPEG/PNG are present, the VSM importer takes precedence
(VSM files are the native format and likely an already-imported session).

## Adding to a Different GUI-Dependent Package

1. Copy `JpegSequenceImporter.h/.cpp` into your package.
2. Add both files to your `.upp` package descriptor under `file`.
3. Ensure your package's `uses` section includes `CtrlLib` (or at minimum `Draw`)
   so that `JPGRaster`, `PNGRaster`, `ImageBuffer`, and `JPGEncoder` are available.
4. The headless `VisualStateModel` package must still be in `uses` for
   `VsmSessionStore`, `VsmImageBuffer`, `AppLog`, `CoreLog`.

## Synthetic Smoke Test

At startup, `MainWindow::DockInit()` calls `RunJpegSmokeTest()`, which:

1. Creates 3 synthetic 64×64 JPEG files in a temp directory (solid gray at
   brightness 64, 128, and 192) using `JPGEncoder(80)`.
2. Imports them with `sort_numeric = true`, `grayscale = true`, `fps = 25`.
3. Asserts `result.success == true && result.frames_imported == 3`.
4. Logs either `JPEG smoke test OK: 3/3 frames imported` or a failure message
   to the workbench debug log.
