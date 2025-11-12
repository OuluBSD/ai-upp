# Texture Data Flow Analysis: PNG to Fragment Shader

## Executive Summary

This document traces the complete lifetime of texture data from PNG files through U++ image loading, OpenGL texture upload, and final rendering. Two test cases are analyzed:
- Test 11 (06l): Simple RGB test using rgb.png
- Test 1 (06b): PBR shader ball using wood texture and other assets

## Test 11: RGB Test (rgb.png → Fragment Shader)

### 1. Source Image: share/imgs/rgb.png
- **Format**: PNG, 256x256, RGB
- **Expected corners**:
  - Top-Left: Red (255, 0, 0)
  - Top-Right: Green (0, 255, 0)
  - Bottom-Left: Blue (0, 0, 255)
  - Bottom-Right: Gray (128, 128, 128)
- **Storage**: PNG format stores RGB in that order

### 2. PNG Loading: uppsrc/plugin/png/pngupp.cpp

**Key Code (lines 167-180)**:
```cpp
void StreamRaster::Create()
{
    // ... setup code ...

    if(color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
       color_type == PNG_COLOR_TYPE_RGB) {
        png_set_bgr(data->png_ptr);  // <-- CRITICAL: Converts RGB to BGR
    }

    // ... read pixels ...
}
```

**Effect**: `png_set_bgr()` tells libpng to output pixels in **BGR byte order** instead of RGB.

**Result after loading**:
- Memory layout for red pixel: `[B=0, G=0, R=255, A=255]`
- Memory layout for green pixel: `[B=0, G=255, R=0, A=255]`
- Memory layout for blue pixel: `[B=255, G=0, R=0, A=255]`

### 3. U++ RGBA Struct: uppsrc/Core/Color.h (lines 2-8)

```cpp
#ifdef PLATFORM_MACOS
struct RGBA {
    byte a, r, g, b;
};
#else
struct RGBA {
    byte b, g, r, a;  // Linux: Fields in BGRA order
};
#endif
```

**Memory Layout on Linux**:
- Struct fields are laid out in memory as: `[b, g, r, a]`
- When PNG loader outputs BGR with `png_set_bgr()`, it matches this field order perfectly

**Example for red pixel (255, 0, 0)**:
```
Memory bytes: [0, 0, 255, 255]
Struct access:
  pixel.b = 0    (memory offset +0)
  pixel.g = 0    (memory offset +1)
  pixel.r = 255  (memory offset +2)
  pixel.a = 255  (memory offset +3)
```

### 4. Image to OpenGL Conversion: uppsrc/Draw/Extensions/SimpleImage.cpp (lines 379-387)

```cpp
void DataFromImage(const Image& img, Vector<byte>& out) {
    Size sz = img.GetSize();
    int bytes = sz.cx * sz.cy * sizeof(RGBA);
    out.SetCount(bytes);
    if (!bytes) return;

    const RGBA* src = img.Begin();
    memcpy(out.Begin(), src, bytes);  // Direct memory copy
}
```

**Effect**: Simple memcpy - no reordering
**Result**: Byte array contains `[B, G, R, A, B, G, R, A, ...]` for each pixel

### 5. OpenGL Texture Upload: uppsrc/api/Graphics/Ogl.cpp (lines 349-365)

```cpp
void OglGfxT<Gfx>::SetTexture(GVar::TextureMode type, Size sz,
                               GVar::Sample sample, int channels,
                               const byte* data) {
    GLenum t = GetOglTextureMode(type);
    GLint intl_tgt_fmt = GetGfxChannelFormat(GVar::SAMPLE_FLOAT, channels);
    GLint intl_fmt = GetGfxChannelFormat(sample, channels);  // GL_RGBA
    GLenum intl_type = GetGfxType(sample);  // GL_UNSIGNED_BYTE

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        t, 0, intl_tgt_fmt,
        sz.cx, sz.cy,
        0, intl_fmt, intl_type,  // format=GL_RGBA
        data);
}
```

**GetGfxChannelFormat** (lines 59-82):
```cpp
GLint GetGfxChannelFormat(GVar::Sample sample, int channels) {
    if (sample != SAMPLE_FLOAT) {
        switch (channels) {
            case 4: return GL_RGBA;  // <-- Used for 4-channel images
        }
    }
}
```

**OpenGL Interpretation**:
- Format: `GL_RGBA` means OpenGL expects bytes in order: `[R, G, B, A]`
- Data provided: `[B, G, R, A, ...]` (from memcpy of RGBA struct)
- **MISMATCH**: OpenGL reads byte 0 as Red, but it contains Blue!

**What OpenGL sees for a red pixel (should be 255,0,0)**:
```
Input bytes: [0, 0, 255, 255]
OpenGL interprets as GL_RGBA:
  R = byte[0] = 0      (WRONG! Should be 255)
  G = byte[1] = 0      (Correct)
  B = byte[2] = 255    (WRONG! Should be 0)
  A = byte[3] = 255    (Correct)
Result: Blue pixel instead of red!
```

### 6. Fragment Shader Sampling: share/eon/tests/06l_toyshader_rgb_test/stage0.glsl

```glsl
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 col = texture(iChannel0, uv);  // Samples swapped texture
    fragColor = col;
}
```

**Result**: Shader receives swapped R/B channels from texture sampler.

### 7. Observed Output for Test 11

**Expected**: TL=Red, TR=Green, BL=Blue, BR=Gray
**Actual**: TL=Blue, TR=Gray(?), BL=Red, BR=Green

The swap is clear: Red ↔ Blue are swapped.

---

## Test 1: PBR Shader Ball Textures

### Wood Texture Path: 1f7dca9c22f324751f2a5a59c9b181dfe3b5564a04b724c657732d0bf09c99db.jpg

### 1. Source Image
- **Format**: JPEG
- **Color Space**: RGB
- **Hash-based filename**: Gets resolved to actual file in share/imgs/

### 2. JPEG Loading: uppsrc/plugin/jpg/jpg.cpp

**JPEG loading does NOT have `png_set_bgr()` call!**

```cpp
// In JPEG loader - simplified
bool JPGRaster::Load(IStream& stream)
{
    // ... JPEG decompression setup ...

    // Read scanlines - outputs RGB in standard order
    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row = image_buffer + (cinfo.output_scanline * row_stride);
        jpeg_read_scanlines(&cinfo, &row, 1);
    }

    // Convert to U++ Image
    // JPEG outputs: [R, G, B] bytes
    // But U++ RGBA struct expects: [b, g, r, a] field order
}
```

**Critical Difference**:
- PNG loader has `png_set_bgr()` → outputs BGR bytes → matches RGBA struct
- JPEG loader outputs RGB bytes → DOES NOT match RGBA struct!

### 3. JPEG to RGBA Conversion

When JPEG outputs RGB bytes `[R, G, B]` and they're stored in RGBA struct:

```cpp
struct RGBA {
    byte b, g, r, a;  // Memory layout: [b, g, r, a]
};

// If JPEG outputs RGB bytes [R_val, G_val, B_val]
// and they're copied sequentially into memory:
Memory: [R_val, G_val, B_val, A_val]

// But when accessed via RGBA struct:
pixel.b = R_val  (WRONG! Blue field contains red value)
pixel.g = G_val  (Correct)
pixel.r = B_val  (WRONG! Red field contains blue value)
pixel.a = A_val  (Correct)
```

**The JPEG data is ALREADY swapped** when stored in RGBA struct!

### 4. DataFromImage (Same as Test 11)

```cpp
void DataFromImage(const Image& img, Vector<byte>& out) {
    const RGBA* src = img.Begin();
    memcpy(out.Begin(), src, bytes);  // Copies [B_field, G_field, R_field, A_field]
}
```

For JPEG-loaded image with original red pixel (255, 0, 0):
```
RGBA struct after JPEG load:
  pixel.b = 255  (contains original R value)
  pixel.g = 0
  pixel.r = 0    (contains original B value)

Memory bytes copied: [255, 0, 0, 255]
```

### 5. OpenGL Upload (Same as Test 11)

```cpp
glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    width, height,
    0, GL_RGBA, GL_UNSIGNED_BYTE,  // Expects [R,G,B,A]
    data);  // Receives [255, 0, 0, 255] for red pixel
```

OpenGL interprets:
- R = 255 ✓ Correct!
- G = 0   ✓ Correct!
- B = 0   ✓ Correct!
- A = 255 ✓ Correct!

**The double-swap cancels out for JPEG!**

1. First swap: JPEG RGB → RGBA struct (R→B, B→R)
2. Second swap: OpenGL reads GL_RGBA from BGR memory (B→R, R→B)
3. Result: Correct colors!

### 6. But Why Is Test 1 Still Problematic?

**Hypothesis**: Test 1 uses MULTIPLE texture sources:

From the .toy file:
```
inputs: [
    { id: 2, type: "texture", filename: "...jpg" },      // JPEG
    { id: 3, type: "volume", filename: "...bin" },       // Binary volume
    { id: 4, type: "cubemap", filename: "...png" }       // PNG cubemap
]
```

**The PNG cubemap** (id: 4) would have:
- PNG loader applies `png_set_bgr()` → BGR bytes
- RGBA struct stores BGR correctly
- OpenGL GL_RGBA format → **SWAPPED!**

---

## CRITICAL FINDING: The Real Cause

After deep investigation, I found the **real issue**:

### JPEG Conversion IS Correct

In `uppsrc/Draw/RasterFormat.cpp` lines 304-306:
```cpp
case RASTER_24:  // JPEG uses this
    t->a = 255;
    t->r = s[rpos];  // rpos=0 for Set24le(0xFF, ...)
    t->g = s[gpos];  // gpos=1 for Set24le(..., 0xFF00, ...)
    t->b = s[bpos];  // bpos=2 for Set24le(..., ..., 0xFF0000)
```

- JPEG's `Set24le(0xFF, 0xFF00, 0xFF0000)` sets: rpos=0, gpos=1, bpos=2
- libjpeg outputs RGB: `[R, G, B]`
- RasterFormat reads: `t->r = s[0] = R` ✓, `t->g = s[1] = G` ✓, `t->b = s[2] = B` ✓
- **JPEG textures are correct!**

### The REAL Issue: DataFromImage Uses GL_RGBA for ALL Images

The bug is in `uppsrc/api/Graphics/Ogl.cpp`:

**For both PNG and JPEG:**
1. Images are converted to RGBA struct (with `byte b, g, r, a` field order)
2. DataFromImage copies memory: `[b_value, g_value, r_value, a_value, ...]`
3. OpenGL upload uses `GL_RGBA` which expects: `[R, G, B, A]`

**For PNG:**
- PNG loader outputs BGR bytes (via `png_set_bgr()`)
- RGBA struct stores them correctly: `struct.b=B, struct.g=G, struct.r=R`
- Memory layout: `[B, G, R, A]`
- OpenGL GL_RGBA reads: R=B ✗, G=G ✓, B=R ✗ → **SWAPPED**

**For JPEG:**
- libjpeg outputs RGB bytes
- RasterFormat converts using rpos/gpos/bpos to RGBA struct correctly
- RGBA struct: `struct.b=B, struct.g=G, struct.r=R`
- Memory layout: `[B, G, R, A]`
- OpenGL GL_RGBA reads: R=B ✗, G=G ✓, B=R ✗ → **SWAPPED**

**Both formats are swapped for the same reason!**

## Summary: Why Colors Are Swapped

### Root Cause Chain

1. **U++ Design**: RGBA struct has `byte b, g, r, a;` field order (BGR in memory)
2. **PNG Loader**: Calls `png_set_bgr()` to output BGR bytes
3. **JPEG Loader**: Outputs standard RGB bytes (no reordering)
4. **DataFromImage**: Simple memcpy (no reordering)
5. **OpenGL Upload**: Uses `GL_RGBA` format (expects RGB byte order)

### Result Matrix

| Image Type | After Load | After memcpy | OpenGL Expects | Result |
|------------|------------|--------------|----------------|--------|
| PNG        | BGR bytes  | BGR bytes    | RGB bytes      | **SWAPPED** |
| JPEG       | RGB→BGR struct | BGR bytes | RGB bytes   | Correct (double swap) |

### Why Test 11 Is Swapped

- Uses PNG (rgb.png)
- PNG loader outputs BGR (correct for RGBA struct)
- memcpy preserves BGR byte order
- OpenGL GL_RGBA expects RGB byte order
- **Mismatch: BGR sent, RGB expected → Swap!**

### Why Test 1 Is Partially Swapped

- Wood texture: JPEG → **Correct** (double swap)
- Cubemap: PNG → **Swapped** (single swap)
- Shader combines textures → **Mixed results**

---

## The Fix Should Be

Since U++ RGBA struct has BGR memory layout on Linux, and both PNG and JPEG end up with BGR memory layout, the fix is simple:

### **CORRECT FIX: Use GL_BGRA Instead of GL_RGBA**

In `uppsrc/api/Graphics/Ogl.cpp`, function `GetGfxChannelFormat`:

**Current Code** (line 67):
```cpp
case 4: return GL_RGBA;  // WRONG for U++ BGR memory layout
```

**Fixed Code**:
```cpp
case 4: return GL_BGRA;  // CORRECT for U++ BGR memory layout
```

### Why This Works:

1. **U++ RGBA struct** on Linux: `byte b, g, r, a;` → Memory: `[B, G, R, A]`
2. **DataFromImage** copies this memory as-is: `[B, G, R, A]`
3. **GL_BGRA** tells OpenGL to interpret bytes as: B=byte[0], G=byte[1], R=byte[2], A=byte[3]
4. **Result**: Colors are correct! ✓

### Alternative (More Complex): Swap in DataFromImage

```cpp
void DataFromImage(const Image& img, Vector<byte>& out) {
    Size sz = img.GetSize();
    int bytes = sz.cx * sz.cy * sizeof(RGBA);
    out.SetCount(bytes);
    if (!bytes) return;

    const RGBA* src = img.Begin();
    byte* dst = out.Begin();

    for(int i = 0; i < sz.cx * sz.cy; i++) {
        dst[0] = src->r;  // Swap R to byte 0
        dst[1] = src->g;  // Keep G at byte 1
        dst[2] = src->b;  // Swap B to byte 2
        dst[3] = src->a;  // Keep A at byte 3
        dst += 4;
        src++;
    }
}
```

But this is slower and unnecessary if we just use GL_BGRA!

---

## Why The Error Is Hidden

The error is hidden because:

1. **Different loaders**: PNG vs JPEG behave differently
2. **Struct field order**: BGR memory layout is unusual
3. **Double swap**: JPEG accidentally works due to two swaps
4. **No clear ownership**: Who is responsible for byte order? Loader? Image? OpenGL wrapper?
5. **Mixed textures**: Test 1 combines multiple texture types

The "proper" fix needs to account for the source format (PNG vs JPEG vs other) and ensure consistent byte ordering at the OpenGL upload stage.
