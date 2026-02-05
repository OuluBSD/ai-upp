#ifndef _Umbrella_MapSerializer_h_
#define _Umbrella_MapSerializer_h_

#include "LayerManager.h"

using namespace Upp;

class MapSerializer {
public:
	// Load map from JSON file
	static bool LoadFromFile(const String& filePath, LayerManager& layerMgr);

	// Save map to JSON file (future)
	static bool SaveToFile(const String& filePath, const LayerManager& layerMgr);

private:
	// Helper: Load tile indices from JSON array
	static Vector<int> LoadTileIndices(const ValueArray& jsonArray);

	// Helper: Convert linear index to (col, row)
	static Point IndexToColRow(int index, int columns);
};

#endif
