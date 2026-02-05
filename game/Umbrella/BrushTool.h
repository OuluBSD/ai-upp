#ifndef _Umbrella_BrushTool_h_
#define _Umbrella_BrushTool_h_

#include "LayerManager.h"
#include "Tile.h"

using namespace Upp;

enum BrushSize {
	BRUSH_1X1 = 1,
	BRUSH_2X2 = 2,
	BRUSH_3X3 = 3,
	BRUSH_5X5 = 5
};

class BrushTool {
private:
	BrushSize brushSize;
	TileType paintTile;
	bool isPainting;
	Point lastPaintPos;

public:
	BrushTool();

	// Settings
	void SetBrushSize(BrushSize size) { brushSize = size; }
	BrushSize GetBrushSize() const { return brushSize; }

	void SetPaintTile(TileType tile) { paintTile = tile; }
	TileType GetPaintTile() const { return paintTile; }

	// Painting
	void StartPainting(int col, int row, LayerManager& layerMgr);
	void ContinuePainting(int col, int row, LayerManager& layerMgr);
	void StopPainting();

	// Preview
	void GetBrushTiles(int centerCol, int centerRow, Vector<Point>& outTiles) const;

	bool IsPainting() const { return isPainting; }

private:
	void PaintAt(int col, int row, LayerManager& layerMgr);
};

#endif
