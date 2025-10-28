#pragma once
#ifndef _Draw_DrawRasterData_h_
#define _Draw_DrawRasterData_h_

#include "Draw.h"

namespace Upp {

// DrawRasterData - draws raster image data to a drawing context
void DrawRasterData(Draw& w, int x, int y, int cx, int cy, const String& data);

// DataDrawer interface for rendering various data formats
class DataDrawer {
public:
	virtual ~DataDrawer() {}
	virtual void Open(const String& data, int cx, int cy) = 0;
	virtual void Render(ImageBuffer& ib) = 0;
	
	static void Register(DataDrawer* drawer, const char* id);
	
	template<typename T>
	static void Register(const char* id) {
		Register(new T(), id);
	}
};

// StreamRaster class for raster operations
class StreamRaster {
public:
	virtual ~StreamRaster() {}
	virtual Size GetSize() const = 0;
	virtual bool Get(RGBA *scanline, int y) = 0;
	
	static StreamRaster* OpenAny(Stream& stream);
};

// RescaleImage for scaling operations
class RescaleImage {
public:
	virtual ~RescaleImage() {}
	virtual void Create(Size sz, StreamRaster& src, Size srcsz) = 0;
	virtual void Get(RGBA *scanline) = 0;
};

} // namespace Upp

#endif