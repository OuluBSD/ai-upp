#pragma once
#ifndef _CtrlCore_MetaFile_h_
#define _CtrlCore_MetaFile_h_

#include "Ctrl.h"
#include <vector>
#include <memory>

namespace Upp {

// MetaFile - represents a collection of drawing operations that can be replayed
class MetaFile {
	std::vector<byte> data;  // Serialized drawing operations
	Size              size;  // Size of the metafile
	String            description;  // Optional description
	
public:
	void Clear() { data.clear(); size = Size(0, 0); description.Clear(); }
	
	Size GetSize() const { return size; }
	void SetSize(Size sz) { size = sz; }
	
	String GetDescription() const { return description; }
	void SetDescription(const String& desc) { description = desc; }
	
	// Serialize/Deserialize operations
	void Serialize(Stream& s);
	
	// Add drawing operations to the metafile
	void AddRect(int x, int y, int cx, int cy, Color color);
	void AddLine(int x1, int y1, int x2, int y2, int width, Color color);
	void AddImage(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
	void AddText(int x, int y, int angle, const WString& text, Font font, Color ink);
	
	// Draw the metafile to a target drawing context
	void Draw(Draw& target, const Rect& rect);
	void Draw(Draw& target, int x, int y, int cx, int cy);
	void Draw(Draw& target, int x, int y);
	
	// Constructor and assignment
	MetaFile() : size(Size(0, 0)) {}
	MetaFile(Size sz) : size(sz) {}
	
	// Static methods for common operations
	static MetaFile CreateFromImage(const Image& img);
	static MetaFile CreateFromDrawing(const Drawing& drawing);
};

// Enhanced metafile with more features
class ExtendedMetaFile : public MetaFile {
	std::vector<Rect> clip_regions;  // Clip regions for complex drawing
	ValueArray        values;        // Additional values associated with the metafile
	
public:
	void AddClipRegion(const Rect& r);
	void ClearClipRegions();
	
	const std::vector<Rect>& GetClipRegions() const { return clip_regions; }
	const ValueArray& GetValues() const { return values; }
	ValueArray& GetValues() { return values; }
	
	// Enhanced drawing operations
	void Draw(Draw& target, const Rect& rect, const Point& offset = Point(0, 0));
	
	ExtendedMetaFile() = default;
};

}

#endif