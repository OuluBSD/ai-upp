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
	if(json["columns"].IsVoid() || json["rows"].IsVoid()) {
		Exclamation("Missing required fields (columns, rows) in: " + filePath);
		return false;
	}

	// Extract grid dimensions
	int columns = json["columns"];
	int rows = json["rows"];
	int gridSize = json["gridSize"].IsVoid() ? 14 : (int)json["gridSize"];
	int mapCols = json["mapCols"].IsVoid() ? 32 : (int)json["mapCols"];
	int mapRows = json["mapRows"].IsVoid() ? 24 : (int)json["mapRows"];

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
	if(!json["walls"].IsVoid()) {
		const ValueArray& wallArray = json["walls"];
		Vector<int> wallIndices = LoadTileIndices(wallArray);
		for(int index : wallIndices) {
			Point pt = IndexToColRow(index, columns);
			terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_WALL);
		}
	}

	// Load fullBlocks
	if(!json["fullBlocks"].IsVoid()) {
		const ValueArray& blockArray = json["fullBlocks"];
		Vector<int> blockIndices = LoadTileIndices(blockArray);
		for(int index : blockIndices) {
			Point pt = IndexToColRow(index, columns);
			terrainLayer->GetGrid().SetTile(pt.x, pt.y, TILE_FULLBLOCK);
		}
	}

	// Load background
	if(!json["background"].IsVoid()) {
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

bool MapSerializer::LoadDropletSpawns(const String& filePath, Array<DropletSpawnPoint>& spawns) {
	// Load JSON file
	String jsonText = LoadFile(filePath);
	if(jsonText.IsEmpty()) {
		return false;  // File doesn't exist or empty
	}

	// Parse JSON
	Value json = ParseJSON(jsonText);
	if(json.IsError()) {
		return false;
	}

	// Check if droplets array exists
	if(json["droplets"].IsVoid()) {
		return true;  // No droplets defined, but not an error
	}

	// Parse droplet spawn points
	const ValueArray& dropletArray = json["droplets"];
	for(int i = 0; i < dropletArray.GetCount(); i++) {
		const ValueMap& dropletData = dropletArray[i];

		DropletSpawnPoint spawn;

		// Required fields
		if(dropletData["col"].IsVoid() || dropletData["row"].IsVoid()) {
			LOG("Warning: Droplet spawn missing col/row, skipping");
			continue;
		}

		spawn.col = dropletData["col"];
		spawn.row = dropletData["row"];

		// Mode (default: RAINBOW)
		if(!dropletData["mode"].IsVoid()) {
			String modeStr = dropletData["mode"];
			if(modeStr == "ICE") spawn.mode = DROPLET_ICE;
			else if(modeStr == "FIRE") spawn.mode = DROPLET_FIRE;
			else spawn.mode = DROPLET_RAINBOW;
		}

		// Direction (default: 1)
		spawn.direction = dropletData["direction"].IsVoid() ? 1 : (int)dropletData["direction"];

		// Interval (default: 2000ms)
		spawn.intervalMs = dropletData["intervalMs"].IsVoid() ? 2000 : (int)dropletData["intervalMs"];

		// Enabled (default: true)
		spawn.enabled = dropletData["enabled"].IsVoid() ? true : (bool)dropletData["enabled"];

		spawns.Add(pick(spawn));
	}

	LOG("Loaded " << spawns.GetCount() << " droplet spawn points from " << filePath);
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
