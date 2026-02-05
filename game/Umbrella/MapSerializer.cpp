#include "Umbrella.h"
#include "MapSerializer.h"

using namespace Upp;

bool MapSerializer::LoadFromFile(const String& filePath, LayerManager& layerMgr) {
	// Load JSON file
	String jsonText = LoadFile(filePath);
	if(jsonText.IsEmpty()) {
		Exclamation("Failed to load file: " + filePath);
		return false;
	}

	// Parse JSON
	Value json = ParseJSON(jsonText);
	if(json.IsError()) {
		Exclamation("Failed to parse JSON: " + filePath);
		return false;
	}

	// Check required fields
	if(!json.Exists("columns") || !json.Exists("rows")) {
		Exclamation("Missing required fields (columns, rows) in: " + filePath);
		return false;
	}

	// Extract grid dimensions
	int columns = json["columns"];
	int rows = json["rows"];
	int gridSize = json.Get("gridSize", 14);  // Default 14 if missing
	int mapCols = json.Get("mapCols", 32);    // Default 32 if missing
	int mapRows = json.Get("mapRows", 24);    // Default 24 if missing

	// Clear and reinitialize layer manager
	layerMgr.ClearAllLayers();
	layerMgr.InitializeDefaultLayers(columns, rows);

	// Get layers
	Layer* terrainLayer = layerMgr.FindLayerByType(LAYER_TERRAIN);
	Layer* backgroundLayer = layerMgr.FindLayerByType(LAYER_BACKGROUND);

	if(!terrainLayer || !backgroundLayer) {
		Exclamation("Failed to find default layers");
		return false;
	}

	// Set map area
	terrainLayer->GetGrid().SetMapArea(mapCols, mapRows);
	backgroundLayer->GetGrid().SetMapArea(mapCols, mapRows);

	// Load walls
	if(json.Exists("walls")) {
		const ValueArray& wallArray = json["walls"];
		Vector<int> wallIndices = LoadTileIndices(wallArray);
		for(int index : wallIndices) {
			Point pt = IndexToColRow(index, columns);
			terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_WALL);
		}
	}

	// Load fullBlocks
	if(json.Exists("fullBlocks")) {
		const ValueArray& blockArray = json["fullBlocks"];
		Vector<int> blockIndices = LoadTileIndices(blockArray);
		for(int index : blockIndices) {
			Point pt = IndexToColRow(index, columns);
			terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_FULLBLOCK);
		}
	}

	// Load background
	if(json.Exists("background")) {
		const ValueArray& bgArray = json["background"];
		Vector<int> bgIndices = LoadTileIndices(bgArray);
		for(int index : bgIndices) {
			Point pt = IndexToColRow(index, columns);
			backgroundLayer->GetGrid().SetTile(pt.x, pt.y, TILE_BACKGROUND);
		}
	}

	// TODO: Load spawns (entity layer - future task)

	return true;
}

Vector<int> MapSerializer::LoadTileIndices(const ValueArray& jsonArray) {
	Vector<int> indices;

	for(int i = 0; i < jsonArray.GetCount(); i++) {
		indices.Add(jsonArray[i]);
	}

	return indices;
}

Point MapSerializer::IndexToColRow(int index, int columns) {
	return Point(index % columns, index / columns);
}

bool MapSerializer::SaveToFile(const String& filePath, const LayerManager& layerMgr) {
	// TODO: Implement in future task
	Exclamation("Save functionality not yet implemented");
	return false;
}
