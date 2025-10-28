#pragma once
#ifndef _Draw_RasterFormat_h_
#define _Draw_RasterFormat_h_

#include "Draw.h"
#include <cstring>

namespace Upp {

enum {
	RASTER_1           = 0,
	RASTER_2           = 1,
	RASTER_4           = 2,
	RASTER_8           = 3,
	RASTER_16          = 4,
	RASTER_24          = 5,
	RASTER_32          = 6,
	RASTER_8ALPHA      = 7,
	RASTER_32ALPHA     = 8,
	RASTER_32PREMULTIPLIED = 9,
	RASTER_MSBFIRST    = 32,
};

class RasterFormat {
public:
	int type;
	int rpos, gpos, bpos, apos;
	dword rmask, gmask, bmask;

	void Set16le(dword rmask, dword gmask, dword bmask);
	void Set16be(dword rmask, dword gmask, dword bmask);
	void Set24le(dword rmask, dword gmask, dword bmask);
	void Set32le(dword rmask, dword gmask, dword bmask, dword amask = 0);
	void Set24be(dword rmask, dword gmask, dword bmask);
	void Set32be(dword rmask, dword gmask, dword bmask, dword amask = 0);
	void Set32leStraight(dword rmask, dword gmask, dword bmask, dword amask = 0);
	void Set32beStraight(dword rmask, dword gmask, dword bmask, dword amask = 0);
	void SetRGBA();
	void SetRGBAStraight();

	int  IsRGBA() const;
	int  GetByteCount(int cx) const;
	int  GetBpp() const;
	bool HasAlpha() const;
	int  GetColorCount() const;
	int  GetPaletteCount() const;
	
	void Read(RGBA *t, const byte *s, int cx, const RGBA *palette) const;
	void TailBits(RGBA *t, const byte *src, int cx, byte andm, byte shift, const RGBA *palette);
	void TailBitsMSB1st(RGBA *t, const byte *src, int cx, byte shift1, byte andm, byte shift, const RGBA *palette);

	RasterFormat() : type(RASTER_8), rpos(0), gpos(0), bpos(0), apos(0), rmask(0), gmask(0), bmask(0) {}
};

}

#endif