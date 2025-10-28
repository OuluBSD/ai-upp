#pragma once
#ifndef _Draw_Palette_h_
#define _Draw_Palette_h_

#include "Draw.h"

struct PaletteCv {
    byte data[RASTER_MAP_R][RASTER_MAP_G][RASTER_MAP_B];
    
    byte& At(int r, int g, int b) {
        return data[r][g][b];
    }
    
    const byte& At(int r, int g, int b) const {
        return data[r][g][b];
    }
    
    PaletteCv() {}
};

void CreatePaletteCv(const RGBA *palette, int ncolors, PaletteCv& cv_pal);
void CreatePalette(Raster& raster, RGBA *palette, int ncolors);
void CreatePalette(Raster& raster, RGBA *palette, int ncolors, PaletteCv& cv);

#endif